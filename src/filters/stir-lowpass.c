#include <math.h>
#include <stdint.h>
#include <.deps/include/graphics/math-defs.h>
#include <obs-module.h>
#include <obs-frontend-api.h>

#include "filters/stir-lowpass.h"
#include "filters/stir-router.h"
#include "stir-context.h"
#include "chain.h"

typedef struct {
	float gain;
} lowpass_state_t;

const char *stir_lowpass_get_name(void *data)
{
	UNUSED_PARAMETER(data);
	return obs_module_text("STIR Lowpass");
}

void *stir_lowpass_create(obs_data_t *settings, obs_source_t *source)
{
	lowpass_state_t *state = bzalloc(sizeof(lowpass_state_t));
	state->gain = 2.0f;
	return state;
}

void stir_lowpass_destroy(void *data)
{
	lowpass_state_t *state = data;
	bfree(state);
}


static void process_audio(stir_context_t* ctx, void* userdata) {
	lowpass_state_t *state = (lowpass_state_t *)userdata;
	float *buf = stir_get_buf(ctx);
	size_t sz = stir_buf_get_channels(ctx);
	size_t f = stir_buf_get_frames(ctx);
	for (size_t i = 0; i < sz; ++i) {
		buf[i * f] *= state->gain;
	}
}

void stir_lowpass_add(void *data, obs_source_t *source) {
	lowpass_state_t *state = data;

	stir_register_filter("lowpass", state, process_audio, state);
}

void stir_lowpass_remove(void* data, obs_source_t *source) {
	lowpass_state_t *state = data;
	stir_unregister_filter(state);
}

struct obs_source_info stir_lowpass_info = {
	.id = "stir_lowpass",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_AUDIO,
	.get_name = stir_lowpass_get_name,
	.create = stir_lowpass_create,
	.destroy = stir_lowpass_destroy,
	.filter_add = stir_lowpass_add,
	.filter_remove = stir_lowpass_remove
};