#include <obs-module.h>
#include <obs-frontend-api.h>
#include <plugin-support.h>

#include "util.h"
#include "filters/stir-router.h"
#include "stir-context.h"
#include "chain.h"

bool front_loaded = false;

struct stir_router_data {
	obs_source_t *virtual_source;
	obs_source_t *context;
	obs_source_t *parent;
	const char *parent_name;
	size_t channels;
	float sample_rate;

	stir_context_t *buffer_context;
	signal_handler_t *parent_sig_handler;
};

const char *stir_router_get_name(void *data)
{
	UNUSED_PARAMETER(data);
	return obs_module_text("STIR Router");
}

const char *virtual_source_get_name(void *data)
{
	UNUSED_PARAMETER(data);
	return obs_module_text("STIR Virtual Out");
}

void register_new_stir_source(void *private_data)
{
	struct stir_router_data *stir_router = private_data;
	const char *s_pre = "stir_output_";
	const char *s_suf = stir_router->parent_name;
	char *src_name = concat(s_pre, s_suf);
	obs_source_t *src = obs_get_source_by_name(src_name);
	if (src != NULL) {
		stir_router->virtual_source = src;
	} else {
		stir_router->virtual_source = obs_source_create("stir_virtual_out", src_name, NULL, NULL);
	}
	bfree(src_name);
	obs_source_set_audio_mixers(stir_router->virtual_source, 0x1);
}

static void update_filter_chain(void *private_data, calldata_t *cd)
{
	struct stir_router_data *stir_router = private_data;
	update_stir_filter_order(stir_router->parent);
}

void callback_ready(enum obs_frontend_event event, void *private_data)
{
	struct stir_router_data *stir_router = private_data;
	if (event == OBS_FRONTEND_EVENT_FINISHED_LOADING) {
		front_loaded = true;
		register_new_stir_source(stir_router);
	}
}

void stir_router_update(void *data, obs_data_t *settings)
{
	struct stir_router_data *stir_router = data;
}

void *stir_router_create(obs_data_t *settings, obs_source_t *source)
{
	struct stir_router_data *stir_router = bzalloc(sizeof(struct stir_router_data));
	stir_context_t *ctx = stir_context_create();
	stir_router->buffer_context = ctx;
	stir_router->channels = audio_output_get_channels(obs_get_audio());
	stir_router->context = source;
	stir_router->sample_rate = (float)audio_output_get_sample_rate(obs_get_audio());
	obs_frontend_add_event_callback(callback_ready, stir_router);
	stir_router_update(stir_router, settings);
	return stir_router;
}

void stir_router_destroy(void *data)
{
	struct stir_router_data *stir_router = data;
	if (stir_router->virtual_source) {
		obs_source_remove(stir_router->virtual_source);
		obs_source_release(stir_router->virtual_source);
	}
	stir_context_destroy(stir_router->buffer_context);
	signal_handler_disconnect(obs_source_get_signal_handler(stir_router->parent), "reorder_filters", update_filter_chain, stir_router);
	bfree(stir_router);
}

void stir_router_add(void *data, obs_source_t *source)
{
	struct stir_router_data *stir_router = data;
	stir_router->parent = source;
	stir_router->parent_name = obs_source_get_name(source);

	if (front_loaded == true) {
		register_new_stir_source(stir_router);
	} else {
		obs_frontend_add_event_callback(callback_ready, stir_router);
	}
	signal_handler_connect(obs_source_get_signal_handler(stir_router->parent), "reorder_filters", update_filter_chain, stir_router);
}

struct obs_audio_data *stir_router_process(void *data, struct obs_audio_data *audio)
{
	struct stir_router_data *stir_router = data;
	const size_t channels = stir_router->channels;
	const uint32_t sample_ct = audio->frames;
	stir_context_t *ctx = stir_router->buffer_context;

	if (sample_ct == 0) {
		return audio;
	}
	if (channels < 6) {
		return audio;
	}

	float **audio_data = (float **)audio->data;

	float *buffer = stir_get_buf(ctx);
	size_t buf_frames = stir_buf_get_frames(ctx);

	for (size_t i = 0; i < sample_ct; i++) {
		float left = audio_data[0][i];
		float right = audio_data[1][i];
		float buf = left + right * 0.5f;

		buffer[0 * buf_frames + i] = buf;
		buffer[1 * buf_frames + i] = buf;
		buffer[2 * buf_frames + i] = buf;
		buffer[3 * buf_frames + i] = buf;
		buffer[4 * buf_frames + i] = buf;
		buffer[5 * buf_frames + i] = buf;
	}

	stir_process_filters(stir_router->parent, ctx, sample_ct);

	struct obs_source_audio audio_o = {
		.speakers = SPEAKERS_5POINT1,
		.frames = sample_ct,
		.format = AUDIO_FORMAT_FLOAT_PLANAR,
		.samples_per_sec = audio_output_get_sample_rate(obs_get_audio()),
		.timestamp = audio->timestamp
	};

	for (size_t ch = 0; ch < channels; ch++) {
		audio_o.data[ch] = (uint8_t *)(buffer + ch * buf_frames);
	}

	obs_source_output_audio(stir_router->virtual_source, &audio_o);
	return audio;
}

static obs_properties_t *stir_router_properties(void *data)
{
	struct stir_router_data *stir_router = data;
	obs_properties_t *props = obs_properties_create();
	return props;
}

void stir_router_defaults(obs_data_t *settings)
{
	return;
}

void stir_router_filter_order(obs_source_t *source, void *data) {

}

struct obs_source_info stir_router_info = {
	.id = "stir_router",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_AUDIO,
	.get_name = stir_router_get_name,
	.create = stir_router_create,
	.get_defaults = stir_router_defaults,
	.destroy = stir_router_destroy,
	.update = stir_router_update,
	.filter_audio = stir_router_process,
	.get_properties = stir_router_properties,
	.filter_add = stir_router_add,
};

struct obs_source_info virtual_audio_info = {
	.id = "stir_virtual_out",
	.type = OBS_SOURCE_TYPE_INPUT,
	.output_flags = OBS_SOURCE_AUDIO,
	.get_name = virtual_source_get_name,
	.icon_type = OBS_ICON_TYPE_AUDIO_OUTPUT,
};