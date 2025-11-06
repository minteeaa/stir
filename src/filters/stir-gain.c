#include <obs-module.h>
#include <plugin-support.h>
#include <stdio.h>
#include <media-io/audio-math.h>

#include "stir-context.h"
#include "chain.h"
#include "filters/common.h"

struct gain_state {
	struct filter_base base;

	float gain;
	uint32_t mask;
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

void stir_gain_update(void *data, obs_data_t *settings)
{
	struct gain_state *state = data;
	state->gain = db_to_mul((float)obs_data_get_double(settings, "gain"));
	context_collection_t *ctx_c = stir_ctx_c_find(state->base.parent);

	if (ctx_c) {
		for (size_t c = 0; c < ctx_c->length; ++c) {
			for (size_t ch = 0; ch < state->channels; ++ch) {
				uint8_t id = stir_ctx_get_num_id(ctx_c->ctx[c]);
				const char *cid = stir_ctx_get_id(ctx_c->ctx[c]);
				char key[24];
				snprintf(key, sizeof(key), "%s_gain_ch_%zu", cid, ch % 8u);
				if (obs_data_get_bool(settings, key)) {
					state->mask |= (1 << (id * state->channels + ch));
				} else {
					state->mask &= ~(1 << (id * state->channels + ch));
				}
			}
		}
	}
}

void *stir_gain_create(obs_data_t *settings, obs_source_t *source)
{
	struct gain_state *state = bzalloc(sizeof(struct gain_state));
	state->base.ui_id = "gain";
	state->channels = audio_output_get_channels(obs_get_audio());
	state->base.context = source;
	migrate_pre_13_config(settings, state->base.ui_id, state->base.ui_id);
	return state;
}

static void process_audio(stir_context_t *ctx, void *userdata, uint32_t samplect)
{
	struct gain_state *state = (struct gain_state *)userdata;
	float *buf = stir_ctx_get_buf(ctx);
	uint8_t id = stir_ctx_get_num_id(ctx);
	for (size_t i = 0; i < state->channels; ++i) {
		if (state->mask & (1 << (id * state->channels + i))) {
			for (size_t fr = 0; fr < samplect; ++fr) {
				buf[i * samplect + fr] *= state->gain;
			}
		}
	}
}

void stir_gain_add(void *data, obs_source_t *source)
{
	struct gain_state *state = data;
	state->base.parent = source;
	obs_data_t *settings = obs_source_get_settings(state->base.context);
	obs_data_t *defaults = obs_data_get_defaults(settings);
	obs_data_t *settings_safe = obs_data_create_from_json(obs_data_get_json(settings));
	obs_data_t *defaults_safe = obs_data_create_from_json(obs_data_get_json(defaults));
	obs_data_apply(defaults, settings_safe);
	stir_gain_update(state, settings_safe);
	obs_data_release(settings_safe);
	obs_data_release(settings);
	obs_data_release(defaults_safe);
	obs_data_release(defaults);
	stir_register_filter(source, "gain", state->base.context, process_audio, state);
}

void stir_gain_remove(void *data, obs_source_t *source)
{
	struct gain_state *state = data;
	stir_unregister_filter(source, state->base.context);
}

obs_properties_t *stir_gain_properties(void *data)
{
	struct gain_state *state = data;
	obs_properties_t *props = obs_properties_create();

	filter_make_ctx_dropdown(props, &state->base);
	filter_make_ch_list(props, &state->base);

	obs_property_t *p = obs_properties_add_float_slider(props, "gain", "Gain Amount", -30.0, 30.0, 0.1);
	obs_property_float_set_suffix(p, " dB");
	return props;
}

void stir_gain_defaults(obs_data_t *settings)
{
	obs_data_set_default_double(settings, "gain", 0.0);
}

struct obs_source_info stir_gain_info = {.id = "stir_gain",
					 .type = OBS_SOURCE_TYPE_FILTER,
					 .output_flags = OBS_SOURCE_AUDIO,
					 .get_name = stir_gain_get_name,
					 .create = stir_gain_create,
					 .destroy = stir_gain_destroy,
					 .filter_add = stir_gain_add,
					 .filter_remove = stir_gain_remove,
					 .get_properties = stir_gain_properties,
					 .get_defaults = stir_gain_defaults,
					 .update = stir_gain_update};
