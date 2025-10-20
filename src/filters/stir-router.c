#include <obs-module.h>
#include <obs-frontend-api.h>
#include <plugin-support.h>
#include <string.h>

#include "init.h"
#include "callback/calldata.h"
#include "callback/signal.h"
#include "media-io/audio-io.h"
#include "obs.h"
#include "util.h"
#include "filters/stir-router.h"
#include "stir-context.h"
#include "chain.h"
#include "util/c99defs.h"

struct stir_router_data {
	obs_source_t *virtual_source;
	obs_source_t *context;
	obs_source_t *parent;
	const char *parent_name;
	size_t channels;
	float sample_rate;

	bool ms_encoding;
	float add_w, sub_w;

	uint8_t ch_config[MAX_AUDIO_CHANNELS];

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

static void update_stir_source(void *private_data)
{
	struct stir_router_data *stir_router = private_data;
	if (stir_router->virtual_source == NULL) {
		const char *s_pre = "STIR - ";
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
}

static void update_name(void *private_data, calldata_t *cd)
{
	struct stir_router_data *stir_router = private_data;
	const char *new_name;
	if (calldata_get_string(cd, "new_name", &new_name)) {
		const char *s_pre = "STIR - ";
		const char *s_suf = new_name;
		char *src_name = concat(s_pre, s_suf);
		obs_source_set_name(stir_router->virtual_source, src_name);
		bfree(src_name);
	}
}

static void update_filter_chain(void *private_data, calldata_t *cd)
{
	UNUSED_PARAMETER(cd);
	struct stir_router_data *stir_router = private_data;
	update_stir_filter_order(stir_router->parent);
}

void stir_router_update(void *data, obs_data_t *settings)
{
	struct stir_router_data *stir_router = data;
	bool mse = obs_data_get_bool(settings, "ms_encoding");

	char id[12];
	for (size_t ch = 0; ch < stir_router->channels; ++ch) {
		snprintf(id, sizeof(id), "ch_src_%zu", ch);
		if (strcmp(obs_data_get_string(settings, id), "mono_left") == 0) {
			stir_router->ch_config[ch] = 0;
		} else if (strcmp(obs_data_get_string(settings, id), "mono_right") == 0) {
			stir_router->ch_config[ch] = 1;
		} else if (strcmp(obs_data_get_string(settings, id), "stereo_mix") == 0) {
			stir_router->ch_config[ch] = 2;
		} else if (strcmp(obs_data_get_string(settings, id), "ms_add") == 0) {
			if (!mse) obs_data_set_string(settings, id, obs_data_get_default_string(settings, id));
			stir_router->ch_config[ch] = 4;
		} else if (strcmp(obs_data_get_string(settings, id), "ms_sub") == 0) {
			if (!mse) obs_data_set_string(settings, id, obs_data_get_default_string(settings, id));
			stir_router->ch_config[ch] = 5;
		} else {
			stir_router->ch_config[ch] = 3;
		}
	}

	stir_router->ms_encoding = obs_data_get_bool(settings, "ms_encoding");
	
	stir_router->add_w = (float)obs_data_get_double(settings, "ms_width_add");
	stir_router->sub_w = (float)obs_data_get_double(settings, "ms_width_sub");
}

void *stir_router_create(obs_data_t *settings, obs_source_t *source)
{
	struct stir_router_data *stir_router = bzalloc(sizeof(struct stir_router_data));
	stir_context_t *ctx = stir_context_create(source, "main");
	stir_router->buffer_context = ctx;
	stir_router->channels = audio_output_get_channels(obs_get_audio());
	stir_router->context = source;
	stir_router->sample_rate = (float)audio_output_get_sample_rate(obs_get_audio());
	stir_router_update(stir_router, settings);
	return stir_router;
}

void stir_router_destroy(void *data)
{
	struct stir_router_data *stir_router = data;
	if (stir_router->virtual_source) {
		obs_source_release(stir_router->virtual_source);
	}
	stir_context_destroy(stir_router->buffer_context);
	signal_handler_disconnect(obs_source_get_signal_handler(stir_router->parent), "rename", update_name,
				  stir_router);
	signal_handler_disconnect(obs_source_get_signal_handler(stir_router->parent), "reorder_filters",
				  update_filter_chain, stir_router);
	bfree(stir_router);
}

void stir_router_add(void *data, obs_source_t *source)
{
	struct stir_router_data *stir_router = data;
	stir_router->parent = source;
	stir_router->parent_name = obs_source_get_name(source);

	if (front_init == 1) {
		if (scene_changing == 0) {
			update_stir_source(stir_router);
		} else {
			obs_frontend_add_event_callback(stir_router_scene_change_cb, stir_router);
		}
	} else {
		register_front_ready_cb(update_stir_source, stir_router, stir_router);
	}

	signal_handler_connect(obs_source_get_signal_handler(stir_router->parent), "rename", update_name, stir_router);
	signal_handler_connect(obs_source_get_signal_handler(stir_router->parent), "reorder_filters",
			       update_filter_chain, stir_router);
}

struct obs_audio_data *stir_router_process(void *data, struct obs_audio_data *audio)
{
	struct stir_router_data *stir_router = data;
	const size_t channels = stir_router->channels;
	const uint32_t sample_ct = audio->frames;
	stir_context_t *ctx = stir_router->buffer_context;

	if (sample_ct == 0)
		return audio;

	if (channels < 2)
		return audio;

	float **audio_data = (float **)audio->data;
	float *buffer = stir_get_buf(ctx);
	float *ms_buffer = stir_get_ms_buf(ctx);

	for (size_t ch = 0; ch < channels; ch++) {
		for (uint32_t i = 0; i < sample_ct; i++) {
			float left = audio_data[0][i];
			float right = audio_data[1][i];
			float mid = 0.5f * (left + right);
			float side = 0.5f * (left - right);

			if (stir_router->ch_config[ch] == 0)
				buffer[ch * sample_ct + i] = left;

			if (stir_router->ch_config[ch] == 1)
				buffer[ch * sample_ct + i] = right;

			if (stir_router->ch_config[ch] == 2)
				buffer[ch * sample_ct + i] = 0.5f * (left + right);

			if (stir_router->ch_config[ch] == 3)
				buffer[ch * sample_ct + i] = 0.0f;

			ms_buffer[0 * sample_ct + i] = stir_router->ms_encoding ? mid : 0.0f;
			ms_buffer[1 * sample_ct + i] = stir_router->ms_encoding ? side : 0.0f;
		}
	}

	stir_process_filters(stir_router->parent, ctx, sample_ct);

	struct obs_source_audio audio_o = {.speakers = channels,
					   .frames = sample_ct,
					   .format = AUDIO_FORMAT_FLOAT_PLANAR,
					   .samples_per_sec = audio_output_get_sample_rate(obs_get_audio()),
					   .timestamp = audio->timestamp};

	for (size_t ch = 0; ch < channels; ch++) {
		for (uint32_t i = 0; i < sample_ct; i++) {
			if (stir_router->ch_config[ch] == 4)
				buffer[ch * sample_ct + i] = ms_buffer[0 * sample_ct + i] + (stir_router->add_w * ms_buffer[1 * sample_ct + i]);
			
			if (stir_router->ch_config[ch] == 5)
				buffer[ch * sample_ct + i] = ms_buffer[0 * sample_ct + i] - (stir_router->sub_w * ms_buffer[1 * sample_ct + i]);
		}
		
		audio_o.data[ch] = (uint8_t *)(buffer + ch * sample_ct);
	}

	obs_source_output_audio(stir_router->virtual_source, &audio_o);
	return audio;
}

void rebuild_ch_list(obs_properties_t *props, bool ms) {
	for (size_t k = 0; k < audio_output_get_channels(obs_get_audio()); ++k) {
		char id[12];
		snprintf(id, sizeof(id), "ch_src_%zu", k);
		char desc[12];
		snprintf(desc, sizeof(desc), "Channel %zu", k + 1);	
		obs_property_t *chs = obs_properties_get(props, id);
		obs_property_list_clear(chs);
		obs_property_list_add_string(chs, "Mono Left", "mono_left");
		obs_property_list_add_string(chs, "Mono Right", "mono_right");
		obs_property_list_add_string(chs, "Stereo Mix (L+R)", "stereo_mix");
		if (ms) {
			obs_property_list_add_string(chs, "Mid + Side", "ms_add");
			obs_property_list_add_string(chs, "Mid - Side", "ms_sub");
		}
		obs_property_list_add_string(chs, "None", "none");
	}
}

static bool ms_cb(obs_properties_t *props, obs_property_t *property, obs_data_t *settings) {
	bool ms = obs_data_get_bool(settings, "ms_encoding");
	rebuild_ch_list(props, ms);
	return true;
}

obs_properties_t *stir_router_properties(void *data)
{
	struct stir_router_data *stir_router = data;
	obs_properties_t *props = obs_properties_create();
	obs_properties_t *channel_sources = obs_properties_create();
	obs_properties_add_group(props, "channel_sources", "Channel Sources", OBS_GROUP_NORMAL, channel_sources);
	for (size_t k = 0; k < audio_output_get_channels(obs_get_audio()); ++k) {
		char id[12];
		snprintf(id, sizeof(id), "ch_src_%zu", k);
		char desc[12];
		snprintf(desc, sizeof(desc), "Channel %zu", k + 1);	
		obs_property_t *chs = obs_properties_add_list(channel_sources, id, desc, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	}
	obs_properties_t *ms = obs_properties_create();
	obs_property_t *msg = obs_properties_add_group(props, "ms_encoding", "Mid Side Encoding", OBS_GROUP_CHECKABLE, ms);
	obs_properties_add_float_slider(ms, "ms_width_add", "+Side Width", -1.0, 2.0, 0.01);
	obs_properties_add_float_slider(ms, "ms_width_sub", "-Side Width", -1.0, 2.0, 0.01);

	rebuild_ch_list(props, stir_router->ms_encoding);
	obs_property_set_modified_callback(msg, ms_cb);
	
	return props;
}

void stir_router_defaults(obs_data_t *settings)
{
	for (size_t k = 0; k < MAX_AUDIO_CHANNELS; ++k) {
		char id[12];
		snprintf(id, sizeof(id), "ch_src_%zu", k);
		if (k == 0)
			obs_data_set_default_string(settings, id, "mono_left");
		else if (k == 1)
			obs_data_set_default_string(settings, id, "mono_right");
		else
			obs_data_set_default_string(settings, id, "stereo_mix");

		obs_data_set_default_bool(settings, "ms_encoding", false);
		obs_data_set_default_double(settings, "ms_width_mid", 1.0);
		obs_data_set_default_double(settings, "ms_width_side", 1.0);
	}
}

struct obs_source_info stir_router_info = {.id = "stir_router",
					   .type = OBS_SOURCE_TYPE_FILTER,
					   .output_flags = OBS_SOURCE_AUDIO,
					   .get_name = stir_router_get_name,
					   .create = stir_router_create,
					   .get_defaults = stir_router_defaults,
					   .destroy = stir_router_destroy,
					   .update = stir_router_update,
					   .filter_audio = stir_router_process,
					   .get_properties = stir_router_properties,
					   .filter_add = stir_router_add};

struct obs_source_info virtual_audio_info = {.id = "stir_virtual_out",
					     .type = OBS_SOURCE_TYPE_INPUT,
					     .output_flags = OBS_SOURCE_AUDIO,
					     .get_name = virtual_source_get_name,
					     .icon_type = OBS_ICON_TYPE_AUDIO_OUTPUT};
