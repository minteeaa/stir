#include <obs-module.h>
#include <obs-frontend-api.h>

OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR("mintea");
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

bool obs_module_load(void)
{
	obs_log(LOG_INFO, "STIR loaded successfully (version %s)", PLUGIN_VERSION);
	obs_register_source(&stir_router);
	obs_register_source(&virtual_audio_info);
	return true;
}

void obs_module_unload(void)
{
	obs_log(LOG_INFO, "STIR unloaded");
}
