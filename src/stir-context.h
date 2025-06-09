#pragma once

#include <obs-module.h>

typedef struct stir_context {
	float *buffer;
	size_t channels;
	size_t frames;
} stir_context_t;

stir_context_t *stir_context_create(void);
void stir_context_destroy(stir_context_t *ctx);
float *stir_get_buf(stir_context_t *ctx);
size_t stir_buf_get_channels(stir_context_t *ctx);
size_t stir_buf_get_frames(stir_context_t *ctx);