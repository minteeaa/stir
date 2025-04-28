#include <obs-module.h>
#include <obs-frontend-api.h>
#include <obs-audio-controls.h>
#include <plugin-support.h>
#include <util/deque.h>

#include "util.h"

#define PLUGIN_NAME "STIR"
#define STIR_OUT_ID "stir_filter_output"

static const char *stir_filter_get_name(void *data)
{
	UNUSED_PARAMETER(data);
	return obs_module_text("STIR");
}

static const char *virtual_source_get_name(void *data)
{
	UNUSED_PARAMETER(data);
	return obs_module_text("STIR_VIRTUAL_OUT");
}

static void callback_ready(enum obs_frontend_event event, void *private_data)
{
	struct stir_filter_data *stir_filter = private_data;
	if (event == OBS_FRONTEND_EVENT_FINISHED_LOADING) {
		const char *s_pre = "stir_output_";
		const char *s_suf = stir_filter->parent_name;
		char *src_name = concat(s_pre, s_suf);
		obs_source_t *src = obs_get_source_by_name(src_name);
		if (src != NULL) {
			stir_filter->virtual_source = src;
		} else {
			stir_filter->virtual_source = obs_source_create(STIR_OUT_ID, src_name, NULL, NULL);
		}
		bfree(src_name);
		obs_source_set_audio_mixers(stir_filter->virtual_source, 0x1);
	}
}

static void stir_filter_add(void *data, obs_source_t *source) {
	struct stir_filter_data *stir_filter = data;
	stir_filter->parent = source;
	stir_filter->parent_name = obs_source_get_name(source);
}

static void stir_filter_destroy(void *data)
{
	struct stir_filter_data *stir_filter = data;
	if (stir_filter->virtual_source) {
		obs_source_remove(stir_filter->virtual_source);
		obs_source_release(stir_filter->virtual_source);
	}
	for (size_t ch = 0; ch < stir_filter->channels; ch++) {
		bfree(stir_filter->upmix_buffer[ch]);
	}
	bfree(stir_filter);
}

static void stir_filter_update(void *data, obs_data_t *settings)
{
	struct stir_filter_data *stir_filter = data;
	struct filter_channel_state *lowpass_state = &stir_filter->eq[4];
	struct filter_channel_state *highpass_state = &stir_filter->eq[5];
	stir_filter->lp_cutoff = (float)obs_data_get_double(settings, "lp_cutoff_freq");
	stir_filter->hp_cutoff = (float)obs_data_get_double(settings, "hp_cutoff_freq");
	stir_filter->bp_cutoff_upper = (float)obs_data_get_double(settings, "bp_cutoff_freq_upper");
	stir_filter->bp_cutoff_lower = (float)obs_data_get_double(settings, "bp_cutoff_freq_lower");

	stir_filter->sample_rate = (float)audio_output_get_sample_rate(obs_get_audio());

	stir_filter->lp_filter_type = (char *)obs_data_get_string(settings, "lp_filter_type");
	stir_filter->hp_filter_type = (char *)obs_data_get_string(settings, "hp_filter_type");

	stir_filter->lp_intensity = (float)obs_data_get_double(settings, "lp_alpha");
	stir_filter->hp_intensity = (float)obs_data_get_double(settings, "hp_alpha");
	stir_filter->bp_intensity = (float)obs_data_get_double(settings, "bp_alpha");

	butterworth_calculate_lowpass(stir_filter, lowpass_state);
	butterworth_calculate_highpass(stir_filter, highpass_state);
}

static void *stir_filter_create(obs_data_t *settings, obs_source_t *source)
{
	struct stir_filter_data *stir_filter = bzalloc(sizeof(struct stir_filter_data));
	stir_filter->channels = audio_output_get_channels(obs_get_audio());
	stir_filter->context = source;
	for (size_t ch = 0; ch < stir_filter->channels; ch++) {
		stir_filter->upmix_buffer[ch] = bzalloc(sizeof(float) * AUDIO_OUTPUT_FRAMES);
	}
	obs_frontend_add_event_callback(callback_ready, stir_filter);
	stir_filter_update(stir_filter, settings);
	return stir_filter;
}

struct obs_audio_data *stir_filter_process(void *data, struct obs_audio_data *audio)
{
	struct stir_filter_data *stir_filter = data;
	const size_t channels = stir_filter->channels;
	const uint32_t sample_ct = audio->frames;
	if (sample_ct == 0) {
		return audio;
	}
	if (channels < 6) {
		return audio;
	}
	float *samples0 = (float *)audio->data[0];
	float *samples1 = (float *)audio->data[1];

	struct filter_channel_state *lowpass_state = &stir_filter->eq[4];
	struct filter_channel_state *highpass_state = &stir_filter->eq[5];
	struct filter_channel_state *band_state = &stir_filter->eq[2];

	for (size_t i = 0; i < sample_ct; i++) {
		float left = samples0[i];
		float right = samples1[i];

		stir_filter->upmix_buffer[0][i] = left;
		stir_filter->upmix_buffer[1][i] = right;
		stir_filter->upmix_buffer[2][i] =
			simple_highpass(stir_filter, band_state,
					simple_lowpass(stir_filter, band_state, left, stir_filter->bp_cutoff_upper,
						       stir_filter->bp_intensity),
			stir_filter->bp_cutoff_lower, stir_filter->bp_intensity);
		stir_filter->upmix_buffer[3][i] = 0.0f;
		if (strcmp(stir_filter->lp_filter_type, "lp_filter_type_butterworth") == 0) {
			stir_filter->upmix_buffer[4][i] = butterworth_filter(0, stir_filter, lowpass_state, left);
		} else if (strcmp(stir_filter->lp_filter_type, "lp_filter_type_simple") == 0) {
			stir_filter->upmix_buffer[4][i] = simple_lowpass(stir_filter, lowpass_state, left, stir_filter->lp_cutoff, stir_filter->lp_intensity);
		}
		if (strcmp(stir_filter->hp_filter_type, "hp_filter_type_butterworth") == 0) {
			stir_filter->upmix_buffer[5][i] = butterworth_filter(1, stir_filter, highpass_state, left);
		} else if (strcmp(stir_filter->hp_filter_type, "hp_filter_type_simple") == 0) {
			stir_filter->upmix_buffer[5][i] = simple_highpass(stir_filter, highpass_state, left, stir_filter->hp_cutoff, stir_filter->hp_intensity);
		}
	}

	struct obs_source_audio audio_o = {
		.speakers = SPEAKERS_5POINT1,
		.frames = sample_ct,
		.format = AUDIO_FORMAT_FLOAT_PLANAR,
		.samples_per_sec = audio_output_get_sample_rate(obs_get_audio()),
		.timestamp = audio->timestamp
	};

	for (size_t ch = 0; ch < channels; ch++) {
		audio_o.data[ch] = (uint8_t *)stir_filter->upmix_buffer[ch];
	}

	obs_source_output_audio(stir_filter->virtual_source, &audio_o);
	return audio;
}

static obs_properties_t *stir_filter_properties(void *data)
{
	struct stir_filter_data *stir_filter = data;
	obs_properties_t *props = obs_properties_create();
	obs_properties_t *g_lows = obs_properties_create();
	obs_properties_t *g_highs = obs_properties_create();
	obs_properties_t *g_mids = obs_properties_create();
	// obs_properties_t *g_chconfig = obs_properties_create();

	obs_properties_add_group(props, "g_lows", "Lows", OBS_GROUP_CHECKABLE, g_lows);
	obs_properties_add_group(props, "g_highs", "Highs", OBS_GROUP_CHECKABLE, g_highs);
	obs_properties_add_group(props, "g_mids", "Mids", OBS_GROUP_CHECKABLE, g_mids);
	// obs_properties_add_group(props, "g_chconfig", "Channel Configuration", OBS_GROUP_NORMAL, g_chconfig);

	obs_property_t *lp_filter_type = obs_properties_add_list(g_lows, "lp_filter_type", "Filter Type", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	obs_property_list_add_string(lp_filter_type, "Simple", "lp_filter_type_simple");
	obs_property_list_add_string(lp_filter_type, "Butterworth", "lp_filter_type_butterworth");
	obs_properties_add_float_slider(g_lows, "lp_cutoff_freq", "Cutoff Frequency", 10.0, 350.0, 1.0);
	obs_properties_add_float_slider(g_lows, "lp_alpha", "Intensity", 0.01, 1.0, 0.01);

	obs_property_t *hp_filter_type = obs_properties_add_list(g_highs, "hp_filter_type", "Filter Type", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	obs_property_list_add_string(hp_filter_type, "Simple", "hp_filter_type_simple");
	obs_property_list_add_string(hp_filter_type, "Butterworth", "hp_filter_type_butterworth");
	obs_properties_add_float_slider(g_highs, "hp_cutoff_freq", "Cutoff Frequency", 1000.0, 2500.0, 1.0);
	obs_properties_add_float_slider(g_highs, "hp_alpha", "Intensity", 0.01, 1.0, 0.01);

	obs_properties_add_float_slider(g_mids, "bp_cutoff_freq_upper", "Upper Cutoff Frequency", 500.0, 1500.0, 1.0);
	obs_properties_add_float_slider(g_mids, "bp_cutoff_freq_lower", "Lower Cutoff Frequency", 100.0, 500.0, 1.0);
	obs_properties_add_float_slider(g_mids, "bp_alpha", "Intensity", 0.01, 1.0, 0.01);
	/*
	obs_properties_add_list(g_chconfig, "ch_ch0", "Channel 1", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	obs_properties_add_list(g_chconfig, "ch_ch1", "Channel 2", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	obs_properties_add_list(g_chconfig, "ch_ch2", "Channel 3", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	obs_properties_add_list(g_chconfig, "ch_ch3", "Channel 4", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	obs_properties_add_list(g_chconfig, "ch_ch4", "Channel 5", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	obs_properties_add_list(g_chconfig, "ch_ch5", "Channel 6", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	*/
	return props;
}

void stir_filter_defaults(obs_data_t* settings) {
	obs_data_set_default_double(settings, "lp_cutoff_freq", 100.0);
	obs_data_set_default_double(settings, "lp_alpha", 1.0);

	obs_data_set_default_string(settings, "lp_filter_type", "lp_filter_type_butterworth");
	obs_data_set_default_string(settings, "hp_filter_type", "hp_filter_type_butterworth");

	obs_data_set_default_double(settings, "hp_cutoff_freq", 2000.0);
	obs_data_set_default_double(settings, "hp_alpha", 1.0);

	obs_data_set_default_double(settings, "bp_cutoff_freq_upper", 1000.0);
	obs_data_set_default_double(settings, "bp_cutoff_freq_lower", 150.0);
	obs_data_set_default_double(settings, "bp_alpha", 1.0);
}

OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR("mintea");
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

static struct obs_source_info stir_filter = {
	.id = "stir_audio_filter",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_AUDIO,
	.get_name = stir_filter_get_name,
	.create = stir_filter_create,
	.get_defaults = stir_filter_defaults,
	.destroy = stir_filter_destroy,
	.update = stir_filter_update,
	.filter_audio = stir_filter_process,
	.get_properties = stir_filter_properties,
	.filter_add = stir_filter_add,
};

struct obs_source_info virtual_audio_info = {
	.id = STIR_OUT_ID,
	.type = OBS_SOURCE_TYPE_INPUT,
	.output_flags = OBS_SOURCE_AUDIO,
	.get_name = virtual_source_get_name,
	.icon_type = OBS_ICON_TYPE_AUDIO_OUTPUT,
};

bool obs_module_load(void)
{
	obs_log(LOG_INFO, "STIR loaded successfully (version %s)", PLUGIN_VERSION);
	obs_register_source(&stir_filter);
	obs_register_source(&virtual_audio_info);
	return true;
}

void obs_module_unload(void)
{
	obs_log(LOG_INFO, "STIR unloaded");
}
