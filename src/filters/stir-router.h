#pragma once

#include <obs-module.h>
#include <obs-frontend-api.h>

const char *stir_router_get_name(void *data);
const char *virtual_source_get_name(void *data);
void callback_ready(enum obs_frontend_event event, void *private_data);
void *stir_router_create(obs_data_t *settings, obs_source_t *source);
void stir_router_destroy(void *data);
void stir_router_add(void *data, obs_source_t *source);
void stir_router_update(void *data, obs_data_t *settings);
extern struct obs_audio_data *stir_router_process(void *data, struct obs_audio_data *audio);
obs_properties_t *stir_router_properties(void *data);
void stir_router_defaults(obs_data_t *settings);

extern struct obs_source_info stir_router_info;
extern struct obs_source_info virtual_audio_info;
