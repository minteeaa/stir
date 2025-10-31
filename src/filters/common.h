#pragma once

#include <obs-module.h>
#include "obs-properties.h"
#include "obs.h"

struct filter_base {
	obs_source_t *context;
	obs_source_t *parent;
	const char *ui_id;
};

void migrate_pre_13_config(obs_data_t *settings, const char *old_id, const char *new_id);
static bool update_ch_list_vis(void *priv, obs_properties_t *props, obs_property_t *property, obs_data_t *settings);
void filter_make_ctx_dropdown(obs_properties_t *props, struct filter_base *data);
void filter_make_ch_list(obs_properties_t *props, struct filter_base *data);