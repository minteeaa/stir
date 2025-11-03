#include "chain.h"
#include "ext/uthash.h"
#include "util/c99defs.h"

#define MAX_FILTERS 32 // TODO: make limit dynamic perhaps

struct filter_entry {
	void *instance;
	filter_process_fn process;
	void *userdata;
};

typedef struct filter_chain {
	struct filter_entry filters[MAX_FILTERS];
	size_t length;
	bool has_router;
} filter_chain_t;

struct source_chain_info {
	obs_source_t *source;
	filter_chain_t *chain;
	UT_hash_handle hh;
};

static struct source_chain_info *source_chains = NULL;
static size_t enum_index;

void chain_map_insert(obs_source_t *source, filter_chain_t *chain)
{
	struct source_chain_info *entry = bzalloc(sizeof(*entry));
	entry->source = source;
	entry->chain = chain;
	HASH_ADD_PTR(source_chains, source, entry);
}

filter_chain_t *chain_map_find(obs_source_t *source)
{
	struct source_chain_info *entry;
	HASH_FIND_PTR(source_chains, &source, entry);
	return entry ? entry->chain : NULL;
}

void chain_map_remove(obs_source_t *source)
{
	struct source_chain_info *entry;
	HASH_FIND_PTR(source_chains, &source, entry);
	if (entry) {
		HASH_DEL(source_chains, entry);
		bfree(entry);
	}
}

void stir_register_filter(obs_source_t *source, const char *type_name, void *instance, filter_process_fn fn,
			  void *userdata)
{
	UNUSED_PARAMETER(type_name);
	filter_chain_t *chain = chain_map_find(source);
	if (chain == NULL) {
		filter_chain_t *new_filter_chain = bzalloc(sizeof(filter_chain_t));
		new_filter_chain->filters[new_filter_chain->length] =
			(struct filter_entry){.instance = instance, .process = fn, .userdata = userdata};
		new_filter_chain->has_router = false;
		new_filter_chain->length++;
		chain_map_insert(source, new_filter_chain);
	} else {
		if (chain->length >= MAX_FILTERS)
			return;

		chain->filters[chain->length] =
			(struct filter_entry){.instance = instance, .process = fn, .userdata = userdata};
		chain->length++;
	}
}

void stir_unregister_filter(obs_source_t *source, void *instance)
{
	filter_chain_t *chain = chain_map_find(source);
	for (size_t i = 0; i < chain->length; ++i) {
		if (chain->filters[i].instance == instance) {
			for (size_t j = i; j < chain->length - 1; ++j) {
				chain->filters[j] = chain->filters[j + 1];
			}
			chain->length--;
			if (chain->length == 0) {
				obs_source_release(instance);
				bfree(chain);
				chain_map_remove(source);
			}
		}
	}
}

void stir_process_filters(obs_source_t *source, stir_context_t *ctx, uint32_t samples)
{
	filter_chain_t *chain = chain_map_find(source);
	if (chain) {
		for (size_t i = 0; i < chain->length; ++i) {
			chain->filters[i].process(ctx, chain->filters[i].userdata, samples);
		}
	}
}

static void filter_enum_cb(obs_source_t *parent, obs_source_t *child, void *param)
{
	filter_chain_t *chain = chain_map_find(parent);
	filter_chain_t *new_chain = param;
	for (size_t i = 0; i < chain->length; ++i) {
		if (chain->filters[i].instance == child) {
			new_chain->filters[enum_index] = (struct filter_entry){.instance = chain->filters[i].instance,
									       .process = chain->filters[i].process,
									       .userdata = chain->filters[i].userdata};
			enum_index++;
		}
	}
}

void update_stir_filter_order(obs_source_t *source)
{
	filter_chain_t *chain = chain_map_find(source);
	if (chain) {
		filter_chain_t *new_chain = bzalloc(sizeof(filter_chain_t));
		enum_index = 0;
		obs_source_enum_filters(source, filter_enum_cb, new_chain);
		chain = new_chain;
		bfree(new_chain);
	}
}
