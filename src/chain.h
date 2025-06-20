#pragma once

#include "stir-context.h"

typedef void (*filter_process_fn)(stir_context_t *ctx, void *userdata, uint32_t samples);
typedef struct filter_chain filter_chain_t;

void chain_map_insert(obs_source_t *source, filter_chain_t *chain);
filter_chain_t *chain_map_find(obs_source_t *source);
void chain_map_remove(obs_source_t *source);
void stir_register_filter(obs_source_t *source, const char *type_name, void *instance, filter_process_fn fn, void *userdata);
void stir_unregister_filter(obs_source_t *source, void *instance);
void stir_process_filters(obs_source_t *source, stir_context_t *ctx, uint32_t samples);
void update_stir_filter_order(obs_source_t *source);