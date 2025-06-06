#include <math.h>
#include <stdint.h>
#include <.deps/include/graphics/math-defs.h>
#include <obs-module.h>
#include <obs-frontend-api.h>

const char *stir_lowpass_get_name(void *data)
{
	UNUSED_PARAMETER(data);
	return obs_module_text("STIR Lowpass");
}

void *stir_lowpass_create(obs_data_t *settings, obs_source_t *source)
{
	return source;
}

struct obs_source_info stir_lowpass_info = {
	.id = "stir_lowpass",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_AUDIO,
	.get_name = stir_lowpass_get_name,
	.create = stir_lowpass_create
};