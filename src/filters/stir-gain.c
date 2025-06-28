#include <obs-module.h>
#include <plugin-support.h>
#include <stdio.h>
#include <media-io/audio-math.h>

#include "stir-context.h"
#include "chain.h"

struct gain_state {
	obs_source_t *context;

	float gain;
	uint8_t mask;
	size_t channels;
};

const char *stir_gain_get_name(void *data)
{
	UNUSED_PARAMETER(data);
	return obs_module_text("STIR Gain");
}

void stir_gain_destroy(void *data)
{
	struct gain_state *state = data;
	bfree(state);
}

void stir_gain_update(void* data, obs_data_t* settings) {
	struct gain_state *state = data;
	state->gain = db_to_mul((float)obs_data_get_double(settings, "gain"));

	for (size_t ch = 0; ch < state->channels; ++ch) {
		char key[12];
		snprintf(key, sizeof(key), "gain_ch_%zu", ch % 6u);
		if (obs_data_get_bool(settings, key)) {
			state->mask |= (1 << ch);
		} else {
			state->mask &= ~(1 << ch);
		}
	}
}

void *stir_gain_create(obs_data_t *settings, obs_source_t *source)
{
	struct gain_state *state = bzalloc(sizeof(struct gain_state));
	state->channels = audio_output_get_channels(obs_get_audio());
	state->context = source;
	stir_gain_update(state, settings);
	return state;
}

static void process_audio(stir_context_t *ctx, void *userdata, uint32_t samplect)
{
	struct gain_state *state = (struct gain_state *)userdata;
	float *buf = stir_get_buf(ctx);
	for (size_t i = 0; i < state->channels; ++i) {
		if (state->mask & (1 << i)) {
			for (size_t fr = 0; fr < samplect; ++fr) {
				buf[i * samplect + fr] *= state->gain;
			}
		}
	}
}

void stir_gain_add(void *data, obs_source_t *source)
{
	struct gain_state *state = data;
	stir_register_filter(source, "gain", state->context, process_audio, state);
}

void stir_gain_remove(void *data, obs_source_t *source)
{
	struct gain_state *state = data;
	stir_unregister_filter(source, state->context);
}

obs_properties_t* stir_gain_properties(void* data) {
	UNUSED_PARAMETER(data);
	obs_properties_t *props = obs_properties_create();
	obs_properties_t *gain_channels = obs_properties_create();
	for (size_t k = 0; k < audio_output_get_channels(obs_get_audio()); ++k) {
		char id[12];
		snprintf(id, sizeof(id), "gain_ch_%zu", k % 6u);
		char desc[12];
		snprintf(desc, sizeof(desc), "Channel %zu", (k + 1) % 7u);
		obs_properties_add_bool(gain_channels, id, desc);
	}
	obs_properties_add_group(props, "gain_channels", "Channels", OBS_GROUP_NORMAL, gain_channels);
	obs_property_t *p = obs_properties_add_float_slider(props, "gain", "Gain Amount", -30.0, 30.0, 0.1);
	obs_property_float_set_suffix(p, " dB");
	return props;
}

void stir_gain_defaults(obs_data_t *settings)
{
	for (size_t k = 0; k < audio_output_get_channels(obs_get_audio()); ++k) {
		char id[12];
		snprintf(id, sizeof(id), "gain_ch_%zu", k % 6u);
		obs_data_set_default_bool(settings, id, false);
	}
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
