#include <stdint.h>
#include <obs-module.h>

#include "obs-properties.h"
#include "plugin-support.h"
#include "stir-context.h"

char *concat(const char *str1, const char *str2)
{
	size_t len1 = strlen(str1);
	size_t len2 = strlen(str2);

	char *result = (char *)bzalloc(len1 + len2 + 1);
	if (result == NULL) {
		obs_log(LOG_ERROR, "failed to allocate memory for concat job");
		return NULL;
	}

	strcpy(result, str1);
	strcat(result, str2);
	return result;
}

float lerp(float a, float b, float t)
{
	return a + (b - a) * t;
}

float interpexp(float a, float b, float c)
{
	return a * c + b * (1.0f - c);
}

void filter_make_ch_list(obs_properties_t *props, obs_source_t *source, const char *fid)
{
	context_collection_t *ctx_c = stir_ctx_c_find(source);

	if (ctx_c) {
		for (size_t c = 0; c < ctx_c->length; ++c) {
			obs_properties_t *cur_ch = obs_properties_create();
			for (size_t k = 0; k < audio_output_get_channels(obs_get_audio()); ++k) {
				const char *cid = stir_ctx_get_id(ctx_c->ctx[c]);
				char key[24];
				snprintf(key, sizeof(key), "%s_%s_ch_%zu", cid, fid, k % 8u);
				char desc[24];
				snprintf(desc, sizeof(desc), "%s Channel %zu", stir_ctx_get_disp(ctx_c->ctx[c]),
					 (k + 1) % 9u);
				obs_properties_add_bool(cur_ch, key, desc);
			}
			char grc[24];
			snprintf(grc, sizeof(grc), "%s_%s_channels", stir_ctx_get_id(ctx_c->ctx[c]), fid);
			char disp[24];
			snprintf(disp, sizeof(disp), "%s Channels", stir_ctx_get_disp(ctx_c->ctx[c]));
			obs_properties_add_group(props, grc, disp, OBS_GROUP_NORMAL, cur_ch);
		}
	}
}