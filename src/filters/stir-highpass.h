#pragma once

#include <obs-frontend-api.h>

const char *stir_highpass_get_name(void *data);
void *stir_highpass_create(obs_data_t *settings, obs_source_t *source);
extern struct obs_source_info stir_highpass_info;