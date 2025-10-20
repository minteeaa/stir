#include "stir-context.h"
#include "ext/uthash.h"

#define MAX_CONTEXTS 4

typedef struct context_collection {
	stir_context_t *ctx[MAX_CONTEXTS];
	size_t length;
} context_collection_t;

struct source_context {
	obs_source_t *source;
	context_collection_t *ctx_c;
	UT_hash_handle hh;
};

static struct source_context *ctx_list = NULL;

context_collection_t *ctx_c_find(obs_source_t *source)
{
	struct source_context *entry;
	HASH_FIND_PTR(ctx_list, &source, entry);
	return entry ? entry->ctx_c : NULL;
}

void ctx_c_insert(obs_source_t *source, context_collection_t *ctx_c) 
{
	struct source_context *entry = bzalloc(sizeof(*entry));
	entry->ctx_c = ctx_c;
	entry->source = source;
	HASH_ADD_PTR(ctx_list, source, entry);
}

stir_context_t *stir_context_create(obs_source_t *source, const char *id)
{
	stir_context_t *ctx = bzalloc(sizeof(stir_context_t));
	if (ctx) {
		ctx->channels = MAX_AUDIO_CHANNELS;
		ctx->frames = AUDIO_OUTPUT_FRAMES;
		ctx->buffer = bzalloc(ctx->channels * ctx->frames * sizeof(float));
		ctx->id = id;
	}

	context_collection_t *ctx_c = ctx_c_find(source);

	if (ctx_c == NULL) {
		context_collection_t *n_ctx_c = bzalloc(sizeof(context_collection_t));
		n_ctx_c->ctx[n_ctx_c->length] = ctx;
		n_ctx_c->length++;
		ctx_c_insert(source, n_ctx_c);
	} else {
		if (ctx_c->length < MAX_CONTEXTS) {
			ctx_c->ctx[ctx_c->length] = ctx;
			ctx_c->length++;
		}
	}

	return ctx;
}

void ctx_c_delete(obs_source_t *source)
{
	struct source_context *entry;
	HASH_FIND_PTR(ctx_list, &source, entry);
	if (entry) {
		HASH_DEL(ctx_list, entry);
		bfree(entry);
	}
}

void ctx_c_remove(stir_context_t *ctx, obs_source_t *source)
{
	context_collection_t *ctx_c = ctx_c_find(source);
	for (size_t i = 0; i < ctx_c->length; ++i) {
		if (ctx_c->ctx[i] == ctx) {
			for (size_t j = i; j < ctx_c->length - 1; ++j) {
				ctx_c->ctx[j] = ctx_c->ctx[j + 1];
			}
			ctx_c->length--;
			if (ctx_c->length == 0) {
				bfree(ctx_c);
				ctx_c_delete(source);
			}
		}
	}
}

void stir_context_destroy(stir_context_t *ctx, obs_source_t *source)
{
	ctx_c_remove(ctx, source);
	bfree(ctx->buffer);
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
