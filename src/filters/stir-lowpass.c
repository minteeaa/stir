#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <obs-module.h>
#include <obs-frontend-api.h>

#include "filters/stir-lowpass.h"
#include "stir-context.h"
#include "chain.h"

struct channel_variables {
	float b0, b1, b2;
	float a0, a1, a2;
	float x1, x2, y1, y2;
	float z1, z2;
};

struct lowpass_state {
	obs_source_t *context;

	float cutoff;
	float intensity;

	struct channel_variables channel_state[MAX_AUDIO_CHANNELS];

	float sample_rate;
	uint8_t mask;
	size_t channels;
};

void butterworth_calculate_lowpass(struct lowpass_state *state, struct channel_variables *vars) {
	float omega_c = 2.0f * M_PI * (state->cutoff / state->sample_rate);
	float alpha = sinf(omega_c) / (2 * sqrtf(2.0f));

	float cos_omega_c = cosf(omega_c);

	vars->b0 = (1.0f - cos_omega_c) / 2.0f;
	vars->b1 = 1.0f - cos_omega_c;
	vars->b2 = (1.0f - cos_omega_c) / 2.0f;

	vars->a0 = 1 + alpha;
	vars->a1 = -2.0f * cos_omega_c;
	vars->a2 = 1.0f - alpha;

	vars->b0 /= vars->a0;
	vars->b1 /= vars->a0;
	vars->b2 /= vars->a0;

	vars->a1 /= vars->a0;
	vars->a2 /= vars->a0;
}

float butterworth_lowpass(struct lowpass_state *state, struct channel_variables *vars, float in) {
	float out = 0.0f;

	out = (vars->b0 * in + vars->b1 * vars->x1 + vars->b2 * vars->x2 - vars->a1 * vars->y1 - vars->a2 * vars->y2) * state->intensity;

	vars->x2 = vars->x1;
	vars->x1 = in;
	vars->y2 = vars->y1;
	vars->y1 = out;

	return out;
}

const char *stir_lowpass_get_name(void *data)
{
	UNUSED_PARAMETER(data);
	return obs_module_text("STIR Lowpass");
}

void stir_lowpass_destroy(void *data)
{
	struct lowpass_state *state = data;
	bfree(state);
}

void stir_lowpass_update(void* data, obs_data_t* settings) {
	struct lowpass_state *state = data;
	state->intensity = (float)obs_data_get_double(settings, "lp_intensity") * 0.01f;
	state->cutoff = (float)obs_data_get_double(settings, "lp_cutoff_freq");
	state->sample_rate = (float)audio_output_get_sample_rate(obs_get_audio());
	for (int ch = 0; ch < MAX_AUDIO_CHANNELS; ++ch) {
		char key[12];
		snprintf(key, sizeof(key), "lp_ch_%d", ch);
		if (obs_data_get_bool(settings, key)) {
			state->mask |= (1 << ch);
		} else {
			state->mask &= ~(1 << ch);
		}
	}
	for (size_t i = 0; i < state->channels; ++i) {
		struct channel_variables *channel_vars = &state->channel_state[i];
		butterworth_calculate_lowpass(state, channel_vars);
	}
}

void *stir_lowpass_create(obs_data_t *settings, obs_source_t *source)
{
	struct lowpass_state *state = bzalloc(sizeof(struct lowpass_state));
	state->channels = audio_output_get_channels(obs_get_audio());
	state->context = source;
	stir_lowpass_update(state, settings);
	return state;
}

static void process_audio(stir_context_t *ctx, void *userdata, uint32_t samplect)
{
	struct lowpass_state *state = (struct lowpass_state *)userdata;
	float *buf = stir_get_buf(ctx);
	size_t f = stir_buf_get_frames(ctx);
	for (size_t i = 0; i < state->channels; ++i) {
		if (state->mask & (1 << i)) {
			struct channel_variables *channel_vars = &state->channel_state[i];
			for (size_t fr = 0; fr < samplect; ++fr) {
				buf[i * f + fr] = butterworth_lowpass(state, channel_vars, buf[i * f + fr]);
			}
		}
	}
}

void stir_lowpass_add(void *data, obs_source_t *source) {
	struct lowpass_state *state = data;
	stir_register_filter(source, "lowpass", state->context, process_audio, state);
}

void stir_lowpass_remove(void* data, obs_source_t *source) {
	struct lowpass_state *state = data;
	stir_unregister_filter(source, state->context);
}

obs_properties_t *stir_lowpass_properties(void *data)
{
	UNUSED_PARAMETER(data);
	obs_properties_t *props = obs_properties_create();
	obs_properties_t *lowpass_channels = obs_properties_create();
	for (int k = 0; (size_t)k < audio_output_get_channels(obs_get_audio()); ++k) {
		char id[17];
		snprintf(id, sizeof(id), "lp_ch_%d", k);
		char desc[19];
		snprintf(desc, sizeof(desc), "Channel %d", k + 1);
		obs_properties_add_bool(lowpass_channels, id, desc);
	}
	obs_properties_add_group(props, "lowpass_channels", "Channels", OBS_GROUP_NORMAL, lowpass_channels);

	obs_property_t *lf = obs_properties_add_float_slider(props, "lp_cutoff_freq", "Cutoff", 10.0, 2000.0, 1.0);
	obs_property_t *i = obs_properties_add_float_slider(props, "lp_intensity", "Intensity", 1.0, 100.0, 0.5);
	obs_property_float_set_suffix(lf, " Hz");
	obs_property_float_set_suffix(i, "%");
	return props;
}

void stir_lowpass_defaults(obs_data_t *settings)
{
	for (int k = 0; (size_t)k < audio_output_get_channels(obs_get_audio()); ++k) {
		char id[17];
		snprintf(id, sizeof(id), "lp_ch_%d", k);
		obs_data_set_default_bool(settings, id, false);
	}
	obs_data_set_default_double(settings, "lp_cutoff_freq", 100.0);
	obs_data_set_default_double(settings, "lp_intensity", 100.0);
}

struct obs_source_info stir_lowpass_info = {
	.id = "stir_lowpass",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_AUDIO,
	.get_name = stir_lowpass_get_name,
	.create = stir_lowpass_create,
	.destroy = stir_lowpass_destroy,
	.filter_add = stir_lowpass_add,
	.filter_remove = stir_lowpass_remove,
	.get_properties = stir_lowpass_properties,
	.get_defaults = stir_lowpass_defaults,
	.update = stir_lowpass_update
};