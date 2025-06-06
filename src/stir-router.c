#include <obs-module.h>
#include <obs-frontend-api.h>

int front_loaded;

static const char *stir_router_get_name(void *data)
{
	UNUSED_PARAMETER(data);
	return obs_module_text("STIR Router");
}

static const char *virtual_source_get_name(void *data)
{
	UNUSED_PARAMETER(data);
	return obs_module_text("STIR Virtual Out");
}

static void register_new_stir_source(void *private_data)
{
	struct stir_router_data *stir_router = private_data;
	const char *s_pre = "stir_output_";
	const char *s_suf = stir_router->parent_name;
	char *src_name = concat(s_pre, s_suf);
	obs_source_t *src = obs_get_source_by_name(src_name);
	if (src != NULL) {
		stir_router->virtual_source = src;
	} else {
		stir_router->virtual_source = obs_source_create(STIR_OUT_ID, src_name, NULL, NULL);
	}
	bfree(src_name);
	obs_source_set_audio_mixers(stir_router->virtual_source, 0x1);
}

static void callback_ready(enum obs_frontend_event event, void *private_data)
{
	struct stir_router_data *stir_router = private_data;
	if (event == OBS_FRONTEND_EVENT_FINISHED_LOADING) {
		front_loaded = 1;
		register_new_stir_source(stir_router);
	}
}

static void *stir_router_create(obs_data_t *settings, obs_source_t *source)
{
	struct stir_router_data *stir_router = bzalloc(sizeof(struct stir_router_data));
	stir_router->channels = audio_output_get_channels(obs_get_audio());
	stir_router->context = source;
	for (size_t ch = 0; ch < stir_router->channels; ch++) {
		stir_router->upmix_buffer[ch] = bzalloc(sizeof(float) * AUDIO_OUTPUT_FRAMES);
	}
	stir_router->sample_rate = (float)audio_output_get_sample_rate(obs_get_audio());
	obs_frontend_add_event_callback(callback_ready, stir_router);
	stir_router_update(stir_router, settings);
	return stir_router;
}

static void stir_router_destroy(void *data)
{
	struct stir_router_data *stir_router = data;
	if (stir_router->virtual_source) {
		obs_source_remove(stir_router->virtual_source);
		obs_source_release(stir_router->virtual_source);
	}
	for (size_t ch = 0; ch < stir_router->channels; ch++) {
		bfree(stir_router->upmix_buffer[ch]);
	}
	bfree(stir_router);
}

static void stir_router_add(void *data, obs_source_t *source)
{
	struct stir_router_data *stir_router = data;
	stir_router->parent = source;
	stir_router->parent_name = obs_source_get_name(source);

	if (front_loaded == 1) {
		register_new_stir_source(stir_router);
	} else {
		obs_frontend_add_event_callback(callback_ready, stir_router);
	}
}

static void stir_router_update(void *data, obs_data_t *settings)
{
	struct stir_router_data *stir_router = data;
}

struct obs_audio_data *stir_router_process(void *data, struct obs_audio_data *audio)
{
	struct stir_router_data *stir_router = data;
	const size_t channels = stir_router->channels;
	const uint32_t sample_ct = audio->frames;
	if (sample_ct == 0) {
		return audio;
	}
	if (channels < 6) {
		return audio;
	}
	float *samples0 = (float *)audio->data[0];
	float *samples1 = (float *)audio->data[1];

	for (size_t i = 0; i < sample_ct; i++) {
		float left = samples0[i];
		float right = samples1[i];
		float buf = left + right * 0.5;

		stir_router->upmix_buffer[0][i] = buf;
		stir_router->upmix_buffer[1][i] = buf;
		stir_router->upmix_buffer[2][i] = buf;
		stir_router->upmix_buffer[3][i] = buf;
		stir_router->upmix_buffer[4][i] = buf;
		stir_router->upmix_buffer[5][i] = buf;
	}

	struct obs_source_audio audio_o = {
		.speakers = SPEAKERS_5POINT1,
		.frames = sample_ct,
		.format = AUDIO_FORMAT_FLOAT_PLANAR,
		.samples_per_sec = audio_output_get_sample_rate(obs_get_audio()),
		.timestamp = audio->timestamp
	};

	for (size_t ch = 0; ch < channels; ch++) {
		audio_o.data[ch] = (uint8_t *)stir_router->upmix_buffer[ch];
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

static struct obs_source_info stir_router = {
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