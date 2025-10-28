#pragma once

#include <obs-module.h>

const char *stir_lowpass_get_name(void *data);
void *stir_lowpass_create(obs_data_t *settings, obs_source_t *source);
extern struct obs_source_info stir_lowpass_info;
