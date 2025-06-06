#include <obs-module.h>
#include <obs-frontend-api.h>

#pragma once

struct stir_router_data {
	obs_source_t *virtual_source;
	float *upmix_buffer[MAX_AUDIO_CHANNELS];
	obs_source_t *context;
	obs_source_t *parent;
	const char *parent_name;
	size_t channels;
	float sample_rate;
};

static const char *stir_router_get_name(void *data);
static const char *virtual_source_get_name(void *data);
static void register_new_stir_source(void *private_data);
static void callback_ready(enum obs_frontend_event event, void *private_data);
static void *stir_router_create(obs_data_t *settings, obs_source_t *source);
static void stir_router_destroy(void *data);
static void stir_router_add(void *data, obs_source_t *source);
static void stir_router_update(void *data, obs_data_t *settings);
struct obs_audio_data *stir_router_process(void *data, struct obs_audio_data *audio);
static obs_properties_t *stir_router_properties(void *data);
void stir_router_defaults(obs_data_t *settings);

static struct obs_source_info stir_router;
struct obs_source_info virtual_audio_info;