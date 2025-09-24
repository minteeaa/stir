#pragma once

#include <stdint.h>

typedef void (*init_cb)(void *userdata);

extern uint8_t front_init;
extern uint8_t scene_changing;
void register_front_ready_cb(init_cb cb, void *ptr, void *userdata);