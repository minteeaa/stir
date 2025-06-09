#pragma once

#include "stir-context.h"

typedef void (*filter_process_fn)(stir_context_t *ctx, void *userdata);

void stir_register_filter(const char *type_name, void *instance, filter_process_fn fn, void *userdata);
void stir_unregister_filter(void *instance);
void stir_process_filters(stir_context_t *ctx);