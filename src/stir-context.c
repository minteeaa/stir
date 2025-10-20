#include "stir-context.h"
#include "ext/uthash.h"

struct source_context {
	obs_source_t *source;
	context_collection_t *ctx_c;
	UT_hash_handle hh;
};

static struct source_context *ctx_list = NULL;

void ctx_c_insert(obs_source_t *source, context_collection_t *ctx_c) 
{
	struct source_context *entry = bzalloc(sizeof(*entry));
	entry->ctx_c = ctx_c;
	entry->source = source;
	HASH_ADD_PTR(ctx_list, source, entry);
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
	context_collection_t *ctx_c = stir_ctx_c_find(source);
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

context_collection_t *stir_ctx_c_find(obs_source_t *source)
{
	struct source_context *entry;
	HASH_FIND_PTR(ctx_list, &source, entry);
	return entry ? entry->ctx_c : NULL;
}

stir_context_t *stir_context_create(obs_source_t *source, const char *id, const char *disp)
{
	stir_context_t *ctx = bzalloc(sizeof(stir_context_t));
	if (ctx) {
		ctx->buffer = bzalloc(MAX_AUDIO_CHANNELS * AUDIO_OUTPUT_FRAMES * sizeof(float));
		ctx->id = id;
		ctx->disp = disp;
	}

	context_collection_t *ctx_c = stir_ctx_c_find(source);

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

void stir_context_destroy(stir_context_t *ctx, obs_source_t *source)
{
	ctx_c_remove(ctx, source);
	bfree(ctx->buffer);
	bfree(ctx);
}

const char *stir_ctx_get_id(stir_context_t *ctx) 
{
	return ctx ? ctx->id : NULL;
}

const char *stir_ctx_get_disp(stir_context_t *ctx)
{
	return ctx ? ctx->disp : NULL;
}

float *stir_ctx_get_buf(stir_context_t *ctx)
{
	return ctx ? ctx->buffer : NULL;
}