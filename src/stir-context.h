#pragma once

#include <obs-module.h>

typedef struct stir_context {
	float *buffer;
	size_t channels;
	size_t frames;
	const char *id;
} stir_context_t;

typedef struct context_collection context_collection_t;

context_collection_t *ctx_c_find(obs_source_t *source);
void ctx_c_insert(obs_source_t *source, context_collection_t *ctx_c);
void ctx_c_remove(stir_context_t *ctx, obs_source_t *source);
void ctx_c_delete(obs_source_t *source);
stir_context_t *stir_context_create(obs_source_t *source, const char *id);
void stir_context_destroy(stir_context_t *ctx, obs_source_t *source);
float *stir_get_buf(stir_context_t *ctx);
size_t stir_buf_get_channels(stir_context_t *ctx);
size_t stir_buf_get_frames(stir_context_t *ctx);
