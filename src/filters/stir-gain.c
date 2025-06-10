#include <obs-module.h>
#include <plugin-support.h>
#include <stdio.h>

#include "stir-context.h"
#include "chain.h"

typedef struct {
	float gain;
	uint8_t mask;
	size_t channels;
} gain_state_t;

const char *stir_gain_get_name(void *data)
{
	UNUSED_PARAMETER(data);
	return obs_module_text("STIR Gain");
}

void stir_gain_destroy(void *data)
{
	gain_state_t *state = data;
	bfree(state);
}

void stir_gain_update(void* data, obs_data_t* settings) {
	gain_state_t *state = data;
	state->gain = (float)obs_data_get_double(settings, "gain");

	for (int ch = 0; ch < state->channels; ++ch) {
		char key[12];
		snprintf(key, sizeof(key), "gain_ch_%d", ch);
		if (obs_data_get_bool(settings, key)) {
			state->mask |= (1 << ch);
		} else {
			state->mask &= ~(1 << ch);
		}
	}
}

void *stir_gain_create(obs_data_t *settings, obs_source_t *source)
{
	gain_state_t *state = bzalloc(sizeof(gain_state_t));
	state->channels = audio_output_get_channels(obs_get_audio());
	stir_gain_update(state, settings);
	return state;
}

static void process_audio(stir_context_t *ctx, void *userdata)
{
	gain_state_t *state = (gain_state_t *)userdata;
	float *buf = stir_get_buf(ctx);
	size_t f = stir_buf_get_frames(ctx);
	for (size_t i = 0; i < state->channels; ++i) {
		if (state->mask & (1 << i)) {
			buf[i * f] *= state->gain;
		}
	}
}

void stir_gain_add(void *data, obs_source_t *source)
{
	gain_state_t *state = data;

	stir_register_filter("gain", state, process_audio, state);
}

void stir_gain_remove(void *data, obs_source_t *source)
{
	gain_state_t *state = data;
	stir_unregister_filter(state);
}

obs_properties_t* stir_gain_properties(void* data) {
	UNUSED_PARAMETER(data);
	obs_properties_t *props = obs_properties_create();
	obs_properties_t *gain_channels = obs_properties_create();
	obs_properties_add_bool(gain_channels, "gain_ch_0", "Channel 1");
	obs_properties_add_bool(gain_channels, "gain_ch_1", "Channel 2");
	obs_properties_add_bool(gain_channels, "gain_ch_2", "Channel 3");
	obs_properties_add_bool(gain_channels, "gain_ch_3", "Channel 4");
	obs_properties_add_bool(gain_channels, "gain_ch_4", "Channel 5");
	obs_properties_add_bool(gain_channels, "gain_ch_5", "Channel 6");
	obs_properties_add_group(props, "gain_channels", "Channels", OBS_GROUP_NORMAL, gain_channels);
	obs_properties_add_float_slider(props, "gain", "Gain Amount", 0.0, 30.0, 0.1);
	return props;
}

void stir_gain_defaults(obs_data_t *settings)
{
	obs_data_set_default_bool(settings, "gain_ch_1", false);
	obs_data_set_default_bool(settings, "gain_ch_2", false);
	obs_data_set_default_bool(settings, "gain_ch_3", false);
	obs_data_set_default_bool(settings, "gain_ch_4", false);
	obs_data_set_default_bool(settings, "gain_ch_5", false);
	obs_data_set_default_bool(settings, "gain_ch_6", false);
	obs_data_set_default_double(settings, "gain", 0.0);
}

struct obs_source_info stir_gain_info = {
	.id = "stir_gain",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_AUDIO,
	.get_name = stir_gain_get_name,
	.create = stir_gain_create,
	.destroy = stir_gain_destroy,
	.filter_add = stir_gain_add,
	.filter_remove = stir_gain_remove,
	.get_properties = stir_gain_properties,
	.get_defaults = stir_gain_defaults,
	.update = stir_gain_update
};