#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <obs-module.h>
#include <obs-frontend-api.h>

#include "filters/stir-highpass.h"
#include "stir-context.h"
#include "chain.h"
#include "filters/common.h"

struct channel_variables {
	float b0, b1, b2;
	float a0, a1, a2;
	float x1, x2, y1, y2;
	float z1, z2;
};

struct highpass_state {
	struct filter_base base;

	float cutoff, q;
	float wetmix, drymix;

	struct channel_variables *ch_state[MAX_CONTEXTS * MAX_AUDIO_CHANNELS];

	float sample_rate;
	uint32_t mask;
	size_t channels;
};

void butterworth_calculate_highpass(struct highpass_state *state, struct channel_variables *vars)
{
	float omega_c = 2.0f * M_PI * (state->cutoff / state->sample_rate);
	float alpha = sinf(omega_c) / (2.0f * state->q);

	float cos_omega_c = cosf(omega_c);

	vars->b0 = (1.0f + cos_omega_c) / 2.0f;
	vars->b1 = -(1.0f + cos_omega_c);
	vars->b2 = (1.0f + cos_omega_c) / 2.0f;

	vars->a0 = 1 + alpha;
	vars->a1 = -2.0f * cos_omega_c;
	vars->a2 = 1.0f - alpha;

	vars->b0 /= vars->a0;
	vars->b1 /= vars->a0;
	vars->b2 /= vars->a0;

	vars->a1 /= vars->a0;
	vars->a2 /= vars->a0;
}

float butterworth_highpass(struct highpass_state *state, struct channel_variables *vars, float in)
{
	float wet = 0.0f;
	float out = 0.0f;

	wet = vars->b0 * in + vars->b1 * vars->x1 + vars->b2 * vars->x2 - vars->a1 * vars->y1 - vars->a2 * vars->y2;
	out = (in * state->drymix) + (wet * state->wetmix);

	vars->x2 = vars->x1;
	vars->x1 = in;
	vars->y2 = vars->y1;
	vars->y1 = out;

	return out;
}

const char *stir_highpass_get_name(void *data)
{
	UNUSED_PARAMETER(data);
	return obs_module_text("STIR Highpass");
}

void stir_highpass_destroy(void *data)
{
	struct highpass_state *state = data;
	for (size_t ch = 0; ch < MAX_CONTEXTS * MAX_AUDIO_CHANNELS; ++ch) {
		if (state->ch_state[ch])
			bfree(state->ch_state[ch]);
	}
	bfree(state);
}

void stir_highpass_update(void *data, obs_data_t *settings)
{
	struct highpass_state *state = data;
	state->wetmix = (float)obs_data_get_double(settings, "hp_wet_mix");
	state->drymix = (float)obs_data_get_double(settings, "hp_dry_mix");
	state->q = (float)obs_data_get_double(settings, "hp_q");
	state->cutoff = (float)obs_data_get_double(settings, "hp_cutoff_freq");
	state->sample_rate = (float)audio_output_get_sample_rate(obs_get_audio());
	context_collection_t *ctx_c = stir_ctx_c_find(state->base.parent);
	if (ctx_c) {
		for (size_t c = 0; c < ctx_c->length; ++c) {
			for (size_t ch = 0; ch < state->channels; ++ch) {
				uint8_t id = stir_ctx_get_num_id(ctx_c->ctx[c]);
				const char *cid = stir_ctx_get_id(ctx_c->ctx[c]);
				size_t index = id * state->channels + ch;
				char key[24];
				snprintf(key, sizeof(key), "%s_hp_ch_%zu", cid, ch % 8u);
				if (obs_data_get_bool(settings, key)) {
					state->mask |= (1 << index);
					if (!state->ch_state[index]) {
						state->ch_state[index] = bzalloc(sizeof(struct channel_variables));
					}
				} else {
					state->mask &= ~(1 << index);
					if (state->ch_state[index]) {
						bfree(state->ch_state[index]);
						state->ch_state[index] = NULL;
					}
				}
				if (state->ch_state[index]) {
					butterworth_calculate_highpass(state, state->ch_state[index]);
				}
			}
		}
	}
}

void *stir_highpass_create(obs_data_t *settings, obs_source_t *source)
{
	UNUSED_PARAMETER(settings);
	struct highpass_state *state = bzalloc(sizeof(struct highpass_state));
	state->base.ui_id = "lp";
	state->channels = audio_output_get_channels(obs_get_audio());
	state->base.context = source;
	migrate_pre_13_config(settings, state->base.ui_id, state->base.ui_id);
	return state;
}

static void process_audio(stir_context_t *ctx, void *userdata, uint32_t samplect)
{
	struct highpass_state *state = (struct highpass_state *)userdata;
	float *buf = stir_ctx_get_buf(ctx);
	uint8_t id = stir_ctx_get_num_id(ctx);
	for (size_t i = 0; i < state->channels; ++i) {
		size_t index = id * state->channels + i;
		if (state->mask & (1 << index)) {
			struct channel_variables *channel_vars = state->ch_state[index];
			for (size_t fr = 0; fr < samplect; ++fr) {
				buf[i * samplect + fr] =
					butterworth_highpass(state, channel_vars, buf[i * samplect + fr]);
			}
		}
	}
}

void stir_highpass_add(void *data, obs_source_t *source)
{
	struct highpass_state *state = data;
	state->base.parent = source;
	obs_data_t *settings = obs_source_get_settings(state->base.context);
	obs_data_t *defaults = obs_data_get_defaults(settings);
	obs_data_t *settings_safe = obs_data_create_from_json(obs_data_get_json(settings));
	obs_data_apply(defaults, settings_safe);
	stir_highpass_update(state, settings_safe);
	obs_data_release(settings_safe);
	obs_data_release(settings);
	stir_register_filter(source, "highpass", state->base.context, process_audio, state);
}

void stir_highpass_remove(void *data, obs_source_t *source)
{
	struct highpass_state *state = data;
	stir_unregister_filter(source, state->base.context);
}

obs_properties_t *stir_highpass_properties(void *data)
{
	struct highpass_state *state = data;
	obs_properties_t *props = obs_properties_create();

	filter_make_ctx_dropdown(props, &state->base);
	filter_make_ch_list(props, &state->base);

	obs_property_t *cf = obs_properties_add_float_slider(props, "hp_cutoff_freq", "Cutoff", 100.0, 2500.0, 1.0);
	obs_property_float_set_suffix(cf, " Hz");
	obs_property_t *q = obs_properties_add_float_slider(props, "hp_q", "Q", 0.5, 2.0, 0.01);
	obs_property_float_set_suffix(q, "x");
	obs_property_t *wm = obs_properties_add_float_slider(props, "hp_wet_mix", "Wet Mix", 0.0, 1.0, 0.01);
	obs_property_float_set_suffix(wm, "x");
	obs_property_t *dm = obs_properties_add_float_slider(props, "hp_dry_mix", "Dry Mix", 0.0, 1.0, 0.01);
	obs_property_float_set_suffix(dm, "x");
	return props;
}

void stir_highpass_defaults(obs_data_t *settings)
{
	obs_data_set_default_double(settings, "hp_cutoff_freq", 2000.0);
	obs_data_set_default_double(settings, "hp_q", 0.70);
	obs_data_set_default_double(settings, "hp_wet_mix", 1.0);
	obs_data_set_default_double(settings, "hp_dry_mix", 0.0);
}

struct obs_source_info stir_highpass_info = {.id = "stir_highpass",
					     .type = OBS_SOURCE_TYPE_FILTER,
					     .output_flags = OBS_SOURCE_AUDIO,
					     .get_name = stir_highpass_get_name,
					     .create = stir_highpass_create,
					     .destroy = stir_highpass_destroy,
					     .filter_add = stir_highpass_add,
					     .filter_remove = stir_highpass_remove,
					     .get_properties = stir_highpass_properties,
					     .get_defaults = stir_highpass_defaults,
					     .update = stir_highpass_update};
