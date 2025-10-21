#include <obs-module.h>
#include <plugin-support.h>
#include <stdio.h>
#include <media-io/audio-math.h>

#include "media-io/audio-io.h"
#include "obs-properties.h"
#include "obs.h"
#include "stir-context.h"
#include "chain.h"
#include "util.h"
#include "util/bmem.h"
#include "util/c99defs.h"

#define MAX_ECHO_DELAY_MS 5000.0f
#define SMOOTHING_COEFFICIENT 0.9998f

struct channel_variables {
	float *cbuf;
	int write;
};

struct echo_state {
	struct channel_variables channel_state[MAX_AUDIO_CHANNELS];
	obs_source_t *parent;

	float delay_current, delay_target, delay_smoothed;
	float decay, wet_mix, dry_mix;

	obs_source_t *context;
	uint32_t mask;
	size_t channels;
};

size_t max_cbuf_frames;
float smoothing_factor;

const char *stir_echo_get_name(void *data)
{
	UNUSED_PARAMETER(data);
	return obs_module_text("STIR Echo");
}

void stir_echo_destroy(void *data)
{
	struct echo_state *state = data;
	for (size_t ch = 0; ch < state->channels; ++ch) {
		struct channel_variables *chs = &state->channel_state[ch];
		if (chs->cbuf)
			bfree(chs->cbuf);
	}
	bfree(state);
}

void stir_echo_update(void *data, obs_data_t *settings)
{
	struct echo_state *state = data;
	float user_delay = (float)obs_data_get_double(settings, "echo-delay");
	float sample_rate = (float)audio_output_get_sample_rate(obs_get_audio());
	state->delay_target = (user_delay * sample_rate) / 1000.0f;
	state->decay = (float)obs_data_get_double(settings, "echo-decay");
	state->wet_mix = (float)obs_data_get_double(settings, "echo-wet-mix");
	state->dry_mix = (float)obs_data_get_double(settings, "echo-dry-mix");
	context_collection_t *ctx_c = stir_ctx_c_find(state->parent);

	if (ctx_c) {
		for (size_t c = 0; c < ctx_c->length; ++c) {
			for (size_t ch = 0; ch < state->channels; ++ch) {
				uint8_t id = stir_ctx_get_num_id(ctx_c->ctx[c]);
				char key[24];
				struct channel_variables *chs = &state->channel_state[ch];
				snprintf(key, sizeof(key), "%u_echo_ch_%zu", id, ch % 8u);
				if (obs_data_get_bool(settings, key)) {
					if (chs->cbuf == NULL) {
						chs->cbuf = bzalloc(max_cbuf_frames * sizeof(float));
						chs->write = 0;
					}
					state->mask |= (1 << (id * state->channels + ch));
				} else {
					if (chs->cbuf) {
						bfree(chs->cbuf);
						chs->cbuf = NULL;
					}
					state->mask &= ~(1 << (id * state->channels + ch));
				}
			}
		}
	}
}

void *stir_echo_create(obs_data_t *settings, obs_source_t *source)
{
	struct echo_state *state = bzalloc(sizeof(struct echo_state));
	state->channels = audio_output_get_channels(obs_get_audio());
	state->context = source;
	state->delay_current = 1.0f;
	state->delay_smoothed = 1.0f;
	max_cbuf_frames =
		(size_t)((MAX_ECHO_DELAY_MS * (float)audio_output_get_sample_rate(obs_get_audio())) / 1000.0f) + 1;
	stir_echo_update(state, settings);
	return state;
}

float echo(float in, struct channel_variables *ch, struct echo_state *state)
{
	float out = 0.0f;
	state->delay_smoothed = interpexp(state->delay_smoothed, state->delay_target, SMOOTHING_COEFFICIENT);

	if (fabsf(state->delay_smoothed - state->delay_target) < 20.0f) {
		state->delay_current = state->delay_target;
	}
	size_t delay_samples = (size_t)state->delay_current;
	size_t read = (ch->write + max_cbuf_frames - delay_samples) % max_cbuf_frames;

	out = ch->cbuf[read];
	float wet = (in * state->dry_mix) + ((out * state->decay) * state->wet_mix);

	ch->cbuf[ch->write] = wet;
	ch->write = (ch->write + 1) % max_cbuf_frames;

	return wet;
}

static void process_audio(stir_context_t *ctx, void *userdata, uint32_t samplect)
{
	struct echo_state *state = (struct echo_state *)userdata;
	float *buf = stir_ctx_get_buf(ctx);
	uint8_t id = stir_ctx_get_num_id(ctx);
	for (size_t i = 0; i < state->channels; ++i) {
		if (state->mask & (1 << (id * state->channels + i))) {
			struct channel_variables *channel_vars = &state->channel_state[i];
			for (size_t fr = 0; fr < samplect; ++fr) {
				buf[i * samplect + fr] = echo(buf[i * samplect + fr], channel_vars, state);
			}
		}
	}
}

void stir_echo_add(void *data, obs_source_t *source)
{
	struct echo_state *state = data;
	state->parent = source;
	stir_register_filter(source, "echo", state->context, process_audio, state);
}

void stir_echo_remove(void *data, obs_source_t *source)
{
	struct echo_state *state = data;
	stir_unregister_filter(source, state->context);
}

obs_properties_t *stir_echo_properties(void *data)
{
	struct echo_state *state = data;
	obs_properties_t *props = obs_properties_create();

	filter_make_ch_list(props, state->parent, "echo");

	obs_property_t *p = obs_properties_add_float_slider(props, "echo-delay", "Delay", 50.0, 5000.0, 1.0);
	obs_property_float_set_suffix(p, " ms");
	obs_property_t *f = obs_properties_add_float_slider(props, "echo-decay", "Decay Ratio", 0.0, 1.0, 0.01);
	obs_property_float_set_suffix(f, "x");
	obs_property_t *wm = obs_properties_add_float_slider(props, "echo-wet-mix", "Wet Mix", 0.0, 1.0, 0.01);
	obs_property_float_set_suffix(wm, "x");
	obs_property_t *dm = obs_properties_add_float_slider(props, "echo-dry-mix", "Dry Mix", 0.0, 1.0, 0.01);
	obs_property_float_set_suffix(dm, "x");
	return props;
}

void stir_echo_defaults(obs_data_t *settings)
{
	for (size_t c = 0; c < MAX_CONTEXTS; ++c) {
		for (size_t k = 0; k < audio_output_get_channels(obs_get_audio()); ++k) {
			char id[12];
			snprintf(id, sizeof(id), "%zu_echo_ch_%zu", c, k % 8u);
			obs_data_set_default_bool(settings, id, false);
		}
	}
	obs_data_set_default_double(settings, "echo-delay", 50.0);
	obs_data_set_default_double(settings, "echo-decay", 0.5);
	obs_data_set_default_double(settings, "echo-wet-mix", 1.0);
	obs_data_set_default_double(settings, "echo-dry-mix", 1.0);
}

struct obs_source_info stir_echo_info = {.id = "stir_echo",
					 .type = OBS_SOURCE_TYPE_FILTER,
					 .output_flags = OBS_SOURCE_AUDIO,
					 .get_name = stir_echo_get_name,
					 .create = stir_echo_create,
					 .destroy = stir_echo_destroy,
					 .filter_add = stir_echo_add,
					 .filter_remove = stir_echo_remove,
					 .get_properties = stir_echo_properties,
					 .get_defaults = stir_echo_defaults,
					 .update = stir_echo_update};
