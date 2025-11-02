#pragma once

#include <obs/obs.h>

const char *stir_tremolo_get_name(void *data);
void *stir_tremolo_create(obs_data_t *settings, obs_source_t *source);
extern struct obs_source_info stir_tremolo_info;
