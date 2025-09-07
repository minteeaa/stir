#include "stir-context.h"

stir_context_t *stir_context_create(void)
{
	stir_context_t *ctx = bzalloc(sizeof(stir_context_t));
	if (ctx) {
		ctx->channels = MAX_AUDIO_CHANNELS;
		ctx->frames = AUDIO_OUTPUT_FRAMES;
		ctx->buffer = bzalloc(ctx->channels * ctx->frames * sizeof(float));
	}
	return ctx;
}

void stir_context_destroy(stir_context_t *ctx)
{
	bfree(ctx->buffer);
	bfree(ctx->ms_buffer);
	bfree(ctx);
}

float *stir_get_buf(stir_context_t *ctx)
{
	return ctx ? ctx->buffer : NULL;
}

size_t stir_buf_get_channels(stir_context_t *ctx)
{
	return ctx ? ctx->channels : 0;
}

size_t stir_buf_get_frames(stir_context_t *ctx)
{
	return ctx ? ctx->frames : 0;
}
