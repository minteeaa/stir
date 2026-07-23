#pragma once

#include "obs.h"

struct filter_base {
	obs_source_t *context;
	obs_source_t *parent;
	const char *ui_id;
};

extern float sample_rate;
extern size_t channels;

void migrate_pre_13_config(obs_data_t *settings, const char *old_id, const char *new_id);
static bool update_ch_list_vis(void *priv, obs_properties_t *props, obs_property_t *property, obs_data_t *settings);
void filter_make_ctx_dropdown(obs_properties_t *props, struct filter_base *data);
void filter_make_ch_list(obs_properties_t *props, struct filter_base *data);
void filter_build_config(void (*filter_update)(void *, obs_data_t *), void *data, obs_source_t *context);