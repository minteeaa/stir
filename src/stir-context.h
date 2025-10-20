#pragma once

#include <obs-module.h>

#define MAX_CONTEXTS 4

typedef struct stir_context {
	float *buffer;
	const char *id;
	const char *disp;
} stir_context_t;

typedef struct context_collection {
	stir_context_t *ctx[MAX_CONTEXTS];
	size_t length;
} context_collection_t;

context_collection_t *stir_ctx_c_find(obs_source_t *source);
stir_context_t *stir_context_create(obs_source_t *source, const char *id, const char *disp);
void stir_context_destroy(stir_context_t *ctx, obs_source_t *source);
const char *stir_ctx_get_id(stir_context_t *ctx);
float *stir_ctx_get_buf(stir_context_t *ctx);