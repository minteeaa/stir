#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <obs-module.h>
#include <obs-frontend-api.h>

#include "filters/common.h"
#include "filters/stir-vibrato.h"
#include "stir-context.h"
#include "chain.h"
#include "util/c99defs.h"

#define VIBRATO_READ_DELAY_MS 5.0f

struct channel_state {
	float *cbuf;
	float phase;
	int write;
};

struct vibrato_state {
	struct filter_base base;
	struct channel_state *ch_state[MAX_CONTEXTS * MAX_AUDIO_CHANNELS];

	float rate, depth;
	float wetmix, drymix;

	uint32_t mask;
	size_t max_cbuf_frames;
};

const char *stir_vibrato_get_name(void *data)
{
	UNUSED_PARAMETER(data);
	return obs_module_text("STIR Vibrato");
}

void stir_vibrato_destroy(void *data)
{
	struct vibrato_state *state = data;
	for (size_t ch = 0; ch < MAX_CONTEXTS * MAX_AUDIO_CHANNELS; ++ch) {
		if (state->ch_state[ch])
			bfree(state->ch_state[ch]);
	}
	bfree(state);
}

void stir_vibrato_update(void *data, obs_data_t *settings)
{
	struct vibrato_state *state = data;
	state->rate = (float)obs_data_get_double(settings, "vibrato_rate");
	state->depth = (float)obs_data_get_double(settings, "vibrato_depth") * 0.01;
	state->wetmix = (float)obs_data_get_double(settings, "vibrato_wet_mix");
	state->drymix = (float)obs_data_get_double(settings, "vibrato_dry_mix");
	context_collection_t *ctx_c = stir_ctx_c_find(state->base.parent);

	if (ctx_c) {
		for (size_t c = 0; c < ctx_c->length; ++c) {
			for (size_t ch = 0; ch < channels; ++ch) {
				uint8_t id = stir_ctx_get_num_id(ctx_c->ctx[c]);
				const char *cid = stir_ctx_get_id(ctx_c->ctx[c]);
				size_t index = id * channels + ch;
				char key[24];
				snprintf(key, sizeof(key), "%s_vib_ch_%zu", cid, ch % 8u);
				if (obs_data_get_bool(settings, key)) {
					state->mask |= (1 << index);
					if (!state->ch_state[index]) {
						state->ch_state[index] = bzalloc(sizeof(struct channel_state));
						if (state->ch_state[index]) {
							state->ch_state[index]->cbuf =
								bzalloc(state->max_cbuf_frames * sizeof(float));
							state->ch_state[index]->write = 0;
						}
					}
				} else {
					state->mask &= ~(1 << index);
					if (state->ch_state[index]) {
						bfree(state->ch_state[index]->cbuf);
						bfree(state->ch_state[index]);
						state->ch_state[index]->cbuf = NULL;
						state->ch_state[index] = NULL;
					}
				}
			}
		}
	}
}

void *stir_vibrato_create(obs_data_t *settings, obs_source_t *source)
{
	struct vibrato_state *state = bzalloc(sizeof(struct vibrato_state));
	state->base.ui_id = "vib";
	state->base.context = source;
	state->max_cbuf_frames = (size_t)((VIBRATO_READ_DELAY_MS * sample_rate) / 1000.0f) + 1;
	migrate_pre_13_config(settings, state->base.ui_id, state->base.ui_id);
	return state;
}

float vibrato(struct vibrato_state *state, struct channel_state *ch, float in)
{
	float out, wet = 0.0f;
	double s_dec, s_int = 0.0;

	// remap original sine [-1, 1] to [0, max_cbuf_frames - 1]
	// TODO: external method to generate a wave table instead of realtime sine calc would be faster here

	float vibrato_lfo = (sinf(2.0f * M_PI * ch->phase) + 1.0f) * (state->max_cbuf_frames - 1) * 0.5f;
	s_dec = modf((state->depth * vibrato_lfo), &s_int);

	ch->phase += state->rate / sample_rate;
	if (ch->phase >= 1.0f)
		ch->phase -= 1.0f;

	size_t read_index = ch->write + s_int;
	if (read_index >= state->max_cbuf_frames)
		read_index -= state->max_cbuf_frames;
	size_t read2_index = read_index + 1;
	if (read2_index >= state->max_cbuf_frames)
		read2_index -= state->max_cbuf_frames;

	wet = ch->cbuf[read_index] + (s_dec * (ch->cbuf[read2_index] - ch->cbuf[read_index]));
	out = (in * state->drymix) + (wet * state->wetmix);

	ch->cbuf[ch->write] = in;
	ch->write = (ch->write + 1) % state->max_cbuf_frames;

	return out;
}

static void process_audio(stir_context_t *ctx, void *userdata, uint32_t samplect)
{
	struct vibrato_state *state = (struct vibrato_state *)userdata;
	float *buf = stir_ctx_get_buf(ctx);
	uint8_t id = stir_ctx_get_num_id(ctx);
	for (size_t i = 0; i < channels; ++i) {
		size_t index = id * channels + i;
		if (state->mask & (1 << index)) {
			struct channel_state *channel_vars = state->ch_state[index];
			for (size_t fr = 0; fr < samplect; ++fr) {
				buf[i * samplect + fr] = vibrato(state, channel_vars, buf[i * samplect + fr]);
			}
		}
	}
}

void stir_vibrato_add(void *data, obs_source_t *source)
{
	struct vibrato_state *state = data;
	state->base.parent = source;
	filter_build_config(stir_vibrato_update, state, state->base.context);
	stir_register_filter(source, "vibrato", state->base.context, process_audio, state);
}

void stir_vibrato_remove(void *data, obs_source_t *source)
{
	struct vibrato_state *state = data;
	stir_unregister_filter(source, state->base.context);
}

obs_properties_t *stir_vibrato_properties(void *data)
{
	struct vibrato_state *state = data;
	obs_properties_t *props = obs_properties_create();

	filter_make_ctx_dropdown(props, &state->base);
	filter_make_ch_list(props, &state->base);

	obs_property_t *r = obs_properties_add_float_slider(props, "vibrato_rate", "Rate", 0.0, 20.0, 0.1);
	obs_property_t *d = obs_properties_add_float_slider(props, "vibrato_depth", "Depth", 0.0, 100.0, 0.1);
	obs_property_float_set_suffix(r, " Hz");
	obs_property_float_set_suffix(d, "%");
	obs_property_t *wm = obs_properties_add_float_slider(props, "vibrato_wet_mix", "Wet Mix", 0.0, 1.0, 0.01);
	obs_property_float_set_suffix(wm, "x");
	obs_property_t *dm = obs_properties_add_float_slider(props, "vibrato_dry_mix", "Dry Mix", 0.0, 1.0, 0.01);
	obs_property_float_set_suffix(dm, "x");

	return props;
}

void stir_vibrato_defaults(obs_data_t *settings)
{
	obs_data_set_default_double(settings, "vibrato_rate", 1.0);
	obs_data_set_default_double(settings, "vibrato_depth", 100.0);
	obs_data_set_default_double(settings, "vibrato_wet_mix", 1.0);
	obs_data_set_default_double(settings, "vibrato_dry_mix", 0.0);
}

struct obs_source_info stir_vibrato_info = {.id = "stir_vibrato",
					    .type = OBS_SOURCE_TYPE_FILTER,
					    .output_flags = OBS_SOURCE_AUDIO,
					    .get_name = stir_vibrato_get_name,
					    .create = stir_vibrato_create,
					    .destroy = stir_vibrato_destroy,
					    .filter_add = stir_vibrato_add,
					    .filter_remove = stir_vibrato_remove,
					    .get_properties = stir_vibrato_properties,
					    .get_defaults = stir_vibrato_defaults,
					    .update = stir_vibrato_update};
