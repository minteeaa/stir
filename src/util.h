#pragma once

#include "obs-properties.h"
#include "obs.h"

char *concat(const char *str1, const char *str2);
float lerp(float a, float b, float t);
float interpexp(float a, float b, float c);
void filter_make_ch_list(obs_properties_t *props, obs_source_t *source, const char *fid);