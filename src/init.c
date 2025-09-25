#include <obs-module.h>
#include <obs-frontend-api.h>
#include <obs-audio-controls.h>
#include <plugin-support.h>
#include <stdio.h>

#include "init.h"
#include "ext/uthash.h"
#include "filters/stir-echo.h"
#include "filters/stir-router.h"
#include "filters/stir-lowpass.h"
#include "filters/stir-gain.h"
#include "filters/stir-tremolo.h"
#include "filters/stir-highpass.h"
#include "util/base.h"
#include "util/c99defs.h"

OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR("mintea");
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

struct init_cb_c {
	init_cb cb;
	void *ptr;
	void *userdata;
	UT_hash_handle hh;
};

uint8_t front_init = 0;
uint8_t scene_changing = 0;
static struct init_cb_c *init_chain = NULL;

void register_front_ready_cb(init_cb cb, void *ptr, void *userdata)
{
	struct init_cb_c *entry = bzalloc(sizeof(struct init_cb_c));
	entry->cb = cb;
	entry->ptr = ptr;
	entry->userdata = userdata;
	HASH_ADD_PTR(init_chain, ptr, entry);
}

void free_cb_registry(void)
{
	struct init_cb_c *entry, *tmp;
	HASH_ITER (hh, init_chain, entry, tmp) {
		HASH_DEL(init_chain, entry);
		bfree(entry);
	}
}

void front_ready_cb(enum obs_frontend_event event, void *private_data)
{
	UNUSED_PARAMETER(private_data);
	if (event == OBS_FRONTEND_EVENT_FINISHED_LOADING) {
		front_init = 1;
		obs_log(LOG_INFO, "OBS frontend finished loading. Invoking init callbacks.");
		struct init_cb_c *entry, *tmp;
		HASH_ITER (hh, init_chain, entry, tmp) {
			entry->cb(entry->userdata);
		}
		free_cb_registry();
	}
}

void scene_change_cb(enum obs_frontend_event event, void *private_data)
{
	UNUSED_PARAMETER(private_data);
	if (event == OBS_FRONTEND_EVENT_SCENE_COLLECTION_CLEANUP) {
		scene_changing = 1;
	}

	if (event == OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGED ||
	    event == OBS_FRONTEND_EVENT_SCENE_COLLECTION_RENAMED) {
		scene_changing = 0;
	}
}

bool obs_module_load(void)
{
	obs_frontend_add_event_callback(front_ready_cb, NULL);
	obs_frontend_add_event_callback(scene_change_cb, NULL);
	obs_log(LOG_INFO, "STIR loading modules");
	obs_register_source(&stir_router_info);
	obs_log(LOG_INFO, "Router: registered filter");
	obs_register_source(&virtual_audio_info);
	obs_log(LOG_INFO, "Router: registered virtual out");
	obs_register_source(&stir_lowpass_info);
	obs_log(LOG_INFO, "Lowpass: registered filter");
	obs_register_source(&stir_gain_info);
	obs_log(LOG_INFO, "Gain: registered filter");
	obs_register_source(&stir_tremolo_info);
	obs_log(LOG_INFO, "Tremolo: registered filter");
	obs_register_source(&stir_highpass_info);
	obs_log(LOG_INFO, "Highpass: registered filter");
	obs_register_source(&stir_echo_info);
	obs_log(LOG_INFO, "Echo: registered filter");
	obs_log(LOG_INFO, "STIR modules loaded successfully (version %s)", PLUGIN_VERSION);
	return true;
}

void obs_module_unload(void)
{
	obs_frontend_remove_event_callback(front_ready_cb, NULL);
	obs_frontend_remove_event_callback(scene_change_cb, NULL);
	obs_log(LOG_INFO, "STIR unloaded");
}
