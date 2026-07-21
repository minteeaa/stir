#pragma once

#include "obs.h"

const char *stir_vibrato_get_name(void *data);
void *stir_vibrato_create(obs_data_t *settings, obs_source_t *source);
extern struct obs_source_info stir_vibrato_info;
