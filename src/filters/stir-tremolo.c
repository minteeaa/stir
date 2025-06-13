#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <.deps/include/graphics/math-defs.h>
#include <obs-module.h>
#include <obs-frontend-api.h>

#include "plugin-support.h"

#include "filters/stir-tremolo.h"
#include "filters/stir-router.h"
#include "stir-context.h"
#include "chain.h"

struct channel_variables {
	float phase;
};

struct tremolo_state {
	obs_source_t *context;
	
	struct channel_variables channel_state[MAX_AUDIO_CHANNELS];
	float rate;
	float depth;

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

void stir_tremolo_update(void* data, obs_data_t* settings) {
	struct tremolo_state *state = data;
	state->rate = (float)obs_data_get_double(settings, "tremolo_rate");
	state->depth = (float)obs_data_get_double(settings, "tremolo_depth") * 0.01f;
	state->sample_rate = (float)audio_output_get_sample_rate(obs_get_audio());
	for (int ch = 0; ch < MAX_AUDIO_CHANNELS; ++ch) {
		char key[12];
		snprintf(key, sizeof(key), "lfo_ch_%d", ch);
		if (obs_data_get_bool(settings, key)) {
			state->mask |= (1 << ch);
		} else {
			state->mask &= ~(1 << ch);
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

float tremolo(struct tremolo_state *state, struct channel_variables *vars, float in) {
	float out = 0.0f;
	float tremolo_lfo = 1.0f + state->depth * sinf(2.0f * M_PI * vars->phase);
	out = in * tremolo_lfo;

	vars->phase += state->rate / state->sample_rate;
	if (vars->phase >= 1.0f)
		vars->phase -= 1.0f;
	return out;
}

static void process_audio(stir_context_t *ctx, void *userdata, uint32_t samplect)
{
	struct tremolo_state *state = (struct tremolo_state *)userdata;
	float *buf = stir_get_buf(ctx);
	size_t f = stir_buf_get_frames(ctx);
	for (size_t i = 0; i < state->channels; ++i) {
		if (state->mask & (1 << i)) {
			struct channel_variables *channel_vars = &state->channel_state[i];
			for (size_t fr = 0; fr < samplect; ++fr) {
				buf[i * f + fr] = tremolo(state, channel_vars, buf[i * f + fr]);
			}
		}
	}
}

void stir_tremolo_add(void *data, obs_source_t *source) {
	struct tremolo_state *state = data;
	stir_register_filter(source, "tremolo", state->context, process_audio, state);
}

void stir_tremolo_remove(void* data, obs_source_t *source) {
	struct tremolo_state *state = data;
	stir_unregister_filter(source, state->context);
}

obs_properties_t *stir_tremolo_properties(void *data)
{
	UNUSED_PARAMETER(data);
	obs_properties_t *props = obs_properties_create();
	obs_properties_t *tremolo_channels = obs_properties_create();
	for (int k = 0; k < audio_output_get_channels(obs_get_audio()); ++k) {
		char id[12];
		snprintf(id, sizeof(id), "lfo_ch_%d", k);
		char desc[12];
		snprintf(desc, sizeof(desc), "Channel %d", k + 1);
		obs_properties_add_bool(tremolo_channels, id, desc);
	}
	obs_properties_add_group(props, "tremolo_channels", "Channels", OBS_GROUP_NORMAL, tremolo_channels);

	obs_property_t *r = obs_properties_add_float_slider(props, "tremolo_rate", "Rate", 0.0, 20.0, 0.1);
	obs_property_t *d = obs_properties_add_float_slider(props, "tremolo_depth", "Depth", 0.0, 100.0, 0.5);
	obs_property_float_set_suffix(r, " Hz");
	obs_property_float_set_suffix(d, "%");
	return props;
}

void stir_tremolo_defaults(obs_data_t *settings)
{
	for (int k = 0; k < audio_output_get_channels(obs_get_audio()); ++k) {
		char id[12];
		snprintf(id, sizeof(id), "lfo_ch_%d", k);
		obs_data_set_default_bool(settings, id, false);
	}
	obs_data_set_default_double(settings, "tremolo_rate", 4.0);
	obs_data_set_default_double(settings, "tremolo_depth", 50.0);
}

struct obs_source_info stir_tremolo_info = {
	.id = "stir_tremolo",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_AUDIO,
	.get_name = stir_tremolo_get_name,
	.create = stir_tremolo_create,
	.destroy = stir_tremolo_destroy,
	.filter_add = stir_tremolo_add,
	.filter_remove = stir_tremolo_remove,
	.get_properties = stir_tremolo_properties,
	.get_defaults = stir_tremolo_defaults,
	.update = stir_tremolo_update
};