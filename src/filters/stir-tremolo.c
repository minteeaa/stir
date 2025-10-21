#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <obs-module.h>
#include <obs-frontend-api.h>

#include "filters/stir-tremolo.h"
#include "stir-context.h"
#include "chain.h"
#include "util.h"

struct channel_variables {
	float phase;
};

struct tremolo_state {
	obs_source_t *context;
	obs_source_t *parent;

	struct channel_variables channel_state[MAX_AUDIO_CHANNELS];
	float rate;
	float depth;
	float wetmix, drymix;

	float sample_rate;
	uint8_t mask;
	size_t channels;
};

const char *stir_tremolo_get_name(void *data)
{
	UNUSED_PARAMETER(data);
	return obs_module_text("STIR Tremolo");
}

void stir_tremolo_destroy(void *data)
{
	struct tremolo_state *state = data;
	bfree(state);
}

void stir_tremolo_update(void *data, obs_data_t *settings)
{
	struct tremolo_state *state = data;
	state->rate = (float)obs_data_get_double(settings, "tremolo_rate");
	state->depth = (float)obs_data_get_double(settings, "tremolo_depth") * 0.01f;
	state->wetmix = (float)obs_data_get_double(settings, "tremolo_wet_mix");
	state->drymix = (float)obs_data_get_double(settings, "tremolo_dry_mix");
	state->sample_rate = (float)audio_output_get_sample_rate(obs_get_audio());
	context_collection_t *ctx_c = stir_ctx_c_find(state->parent);

	if (ctx_c) {
		for (size_t c = 0; c < ctx_c->length; ++c) {
			for (size_t ch = 0; ch < MAX_AUDIO_CHANNELS; ++ch) {
				uint8_t id = stir_ctx_get_num_id(ctx_c->ctx[c]);
				char key[24];
				snprintf(key, sizeof(key), "%u_lfo_ch_%zu", id, ch % 8u);
				if (obs_data_get_bool(settings, key)) {
					state->mask |= (1 << (id * state->channels + ch));
				} else {
					state->mask &= ~(1 << (id * state->channels + ch));
				}
			}
		}
	}
}

void *stir_tremolo_create(obs_data_t *settings, obs_source_t *source)
{
	struct tremolo_state *state = bzalloc(sizeof(struct tremolo_state));
	state->channels = audio_output_get_channels(obs_get_audio());
	state->context = source;
	stir_tremolo_update(state, settings);
	return state;
}

float tremolo(struct tremolo_state *state, struct channel_variables *vars, float in)
{
	float out = 0.0f;
	float tremolo_lfo = 1.0f + state->depth * sinf(2.0f * M_PI * vars->phase);
	float wet = in * tremolo_lfo;
	out = (in * state->drymix) + (wet * state->wetmix);

	vars->phase += state->rate / state->sample_rate;
	if (vars->phase >= 1.0f)
		vars->phase -= 1.0f;
	return out;
}

static void process_audio(stir_context_t *ctx, void *userdata, uint32_t samplect)
{
	struct tremolo_state *state = (struct tremolo_state *)userdata;
	float *buf = stir_ctx_get_buf(ctx);
	uint8_t id = stir_ctx_get_num_id(ctx);
	for (size_t i = 0; i < state->channels; ++i) {
		if (state->mask & (1 << (id * state->channels + i))) {
			struct channel_variables *channel_vars = &state->channel_state[i];
			for (size_t fr = 0; fr < samplect; ++fr) {
				buf[i * samplect + fr] = tremolo(state, channel_vars, buf[i * samplect + fr]);
			}
		}
	}
}

void stir_tremolo_add(void *data, obs_source_t *source)
{
	struct tremolo_state *state = data;
	state->parent = source;
	stir_register_filter(source, "tremolo", state->context, process_audio, state);
}

void stir_tremolo_remove(void *data, obs_source_t *source)
{
	struct tremolo_state *state = data;
	stir_unregister_filter(source, state->context);
}

obs_properties_t *stir_tremolo_properties(void *data)
{
	struct tremolo_state *state = data;
	obs_properties_t *props = obs_properties_create();

	filter_make_ch_list(props, state->parent, "lfo");

	obs_property_t *r = obs_properties_add_float_slider(props, "tremolo_rate", "Rate", 0.0, 20.0, 0.1);
	obs_property_t *d = obs_properties_add_float_slider(props, "tremolo_depth", "Depth", 0.0, 100.0, 0.5);
	obs_property_float_set_suffix(r, " Hz");
	obs_property_float_set_suffix(d, "%");
	obs_property_t *wm = obs_properties_add_float_slider(props, "tremolo_wet_mix", "Wet Mix", 0.0, 1.0, 0.01);
	obs_property_float_set_suffix(wm, "x");
	obs_property_t *dm = obs_properties_add_float_slider(props, "tremolo_dry_mix", "Dry Mix", 0.0, 1.0, 0.01);
	obs_property_float_set_suffix(dm, "x");
	return props;
}

void stir_tremolo_defaults(obs_data_t *settings)
{
	for (size_t c = 0; c < MAX_CONTEXTS; ++c) {
		for (size_t k = 0; k < audio_output_get_channels(obs_get_audio()); ++k) {
			char id[11];
			snprintf(id, sizeof(id), "%zu_lfo_ch_%zu", c, k % 8u);
			obs_data_set_default_bool(settings, id, false);
		}
	}
	obs_data_set_default_double(settings, "tremolo_rate", 4.0);
	obs_data_set_default_double(settings, "tremolo_depth", 50.0);
	obs_data_set_default_double(settings, "tremolo_wet_mix", 1.0);
	obs_data_set_default_double(settings, "tremolo_dry_mix", 0.0);
}

struct obs_source_info stir_tremolo_info = {.id = "stir_tremolo",
					    .type = OBS_SOURCE_TYPE_FILTER,
					    .output_flags = OBS_SOURCE_AUDIO,
					    .get_name = stir_tremolo_get_name,
					    .create = stir_tremolo_create,
					    .destroy = stir_tremolo_destroy,
					    .filter_add = stir_tremolo_add,
					    .filter_remove = stir_tremolo_remove,
					    .get_properties = stir_tremolo_properties,
					    .get_defaults = stir_tremolo_defaults,
					    .update = stir_tremolo_update};
