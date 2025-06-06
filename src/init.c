#include <obs-module.h>
#include <obs-frontend-api.h>
#include <obs-audio-controls.h>
#include <plugin-support.h>

#include "stir-router.h"
#include "stir-lowpass.h"

OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR("mintea");
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

bool obs_module_load(void)
{
	obs_log(LOG_INFO, "STIR loading modules");
	obs_register_source(&stir_router_info);
	obs_log(LOG_INFO, "Router: registered filter");
	obs_register_source(&virtual_audio_info);
	obs_log(LOG_INFO, "Router: registered virtual out");
	obs_register_source(&stir_lowpass_info);
	obs_log(LOG_INFO, "Lowpass: registered filter");
	obs_log(LOG_INFO, "STIR loaded successfully (version %s)", PLUGIN_VERSION);
	return true;
}

void obs_module_unload(void)
{
	obs_log(LOG_INFO, "STIR unloaded");
}