#pragma once

#include <obs-module.h>

const char *stir_echo_get_name(void *data);
void *stir_echo_create(obs_data_t *settings, obs_source_t *source);
extern struct obs_source_info stir_echo_info;
