#pragma once

#include <obs-module.h>
#include <obs-frontend-api.h>

const char *stir_gain_get_name(void *data);
void *stir_gain_create(obs_data_t *settings, obs_source_t *source);
extern struct obs_source_info stir_gain_info;