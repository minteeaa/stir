#include <obs-module.h>

#include "stir-context.h"
#include "chain.h"

typedef struct {
	float gain;
} gain_state_t;

const char *stir_gain_get_name(void *data)
{
	UNUSED_PARAMETER(data);
	return obs_module_text("STIR Gain");
}

void *stir_gain_create(obs_data_t *settings, obs_source_t *source)
{
	gain_state_t *state = bzalloc(sizeof(gain_state_t));
	state->gain = 2.0f;
	return state;
}

void stir_gain_destroy(void *data)
{
	gain_state_t *state = data;
	bfree(state);
}

static void process_audio(stir_context_t *ctx, void *userdata)
{
	gain_state_t *state = (gain_state_t *)userdata;
	float *buf = stir_get_buf(ctx);
	size_t sz = stir_buf_get_channels(ctx);
	size_t f = stir_buf_get_frames(ctx);
	for (size_t i = 0; i < sz; ++i) {
		buf[i * f] *= state->gain;
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

struct obs_source_info stir_gain_info = {
	.id = "stir_gain",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_AUDIO,
	.get_name = stir_gain_get_name,
	.create = stir_gain_create,
	.destroy = stir_gain_destroy,
	.filter_add = stir_gain_add,
	.filter_remove = stir_gain_remove
};