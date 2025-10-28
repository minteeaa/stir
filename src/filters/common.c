#include <stdint.h>
#include <obs-module.h>

#include "stir-context.h"
#include "obs-properties.h"
#include "util/c99defs.h"
#include "filters/common.h"

/* TODO: add method for setting defaults */

static bool update_ch_list_vis(void *priv, obs_properties_t *props, obs_property_t *property, obs_data_t *settings)
{
	UNUSED_PARAMETER(property);
	struct filter_base *data = priv;
	context_collection_t *ctx_c = stir_ctx_c_find(data->parent);
	if (ctx_c) {
		for (size_t c = 0; c < ctx_c->length; ++c) {
			const char *cid = stir_ctx_get_id(ctx_c->ctx[c]);
			char trc[24];
			snprintf(trc, sizeof(trc), "%s_%s_channels", stir_ctx_get_id(ctx_c->ctx[c]), data->ui_id);
			obs_property_t *target = obs_properties_get(props, trc);
			const char *fil = obs_data_get_string(settings, "ch_fil");
			char ie[14];
			snprintf(ie, sizeof(ie), "ch_g_%s", cid);
			if (strcmp(ie, fil) == 0) {
				obs_property_set_visible(target, true);
			} else {
				obs_property_set_visible(target, false);
			}
		}
	}
	return true;
}

void filter_make_ch_list(obs_properties_t *props, struct filter_base *data)
{
	context_collection_t *ctx_c = stir_ctx_c_find(data->parent);
	obs_data_t *settings = obs_source_get_settings(data->context);
	obs_data_t *settings_safe = obs_data_create_from_json(obs_data_get_json(settings));
	if (ctx_c) {
		for (size_t c = 0; c < ctx_c->length; ++c) {
			const char *cid = stir_ctx_get_id(ctx_c->ctx[c]);
			obs_properties_t *cur_ch = obs_properties_create();
			for (size_t k = 0; k < audio_output_get_channels(obs_get_audio()); ++k) {
				char key[24];
				snprintf(key, sizeof(key), "%s_%s_ch_%zu", cid, data->ui_id, k % 8u);
				char desc[24];
				snprintf(desc, sizeof(desc), "Channel %zu", (k + 1) % 9u);
				obs_properties_add_bool(cur_ch, key, desc);
			}
			char disp[24];
			snprintf(disp, sizeof(disp), "%s Channels", stir_ctx_get_disp(ctx_c->ctx[c]));
			char grc[24];
			snprintf(grc, sizeof(grc), "%s_%s_channels", stir_ctx_get_id(ctx_c->ctx[c]), data->ui_id);
			obs_property_t *target = obs_properties_add_group(props, grc, disp, OBS_GROUP_NORMAL, cur_ch);
			const char *fil = obs_data_get_string(settings_safe, "ch_fil");
			char ie[14];
			snprintf(ie, sizeof(ie), "ch_g_%s", cid);
			if (strcmp(ie, fil) == 0) {
				obs_property_set_visible(target, true);
			} else {
				obs_property_set_visible(target, false);
			}
		}
	}
	obs_data_release(settings_safe);
	obs_data_release(settings);
}

void filter_make_ctx_dropdown(obs_properties_t *props, struct filter_base *data)
{
	context_collection_t *ctx_c = stir_ctx_c_find(data->parent);
	obs_property_t *fil = obs_properties_add_list(props, "ch_fil", "Channel Filter", OBS_COMBO_TYPE_LIST,
						      OBS_COMBO_FORMAT_STRING);

	if (ctx_c) {
		for (size_t c = 0; c < ctx_c->length; ++c) {
			const char *cd = stir_ctx_get_disp(ctx_c->ctx[c]);
			const char *cid = stir_ctx_get_id(ctx_c->ctx[c]);
			char ie[14];
			snprintf(ie, sizeof(ie), "ch_g_%s", cid);
			obs_property_list_add_string(fil, cd, ie);
		}
		obs_property_list_add_string(fil, "All", "ch_g_none");
	}
	obs_property_set_modified_callback2(fil, update_ch_list_vis, data);
}