#include "chain.h"

#define MAX_FILTERS 16

typedef struct {
	const char *type_name;
	void *instance;
	filter_process_fn process;
	void *userdata;
} filter_entry_t;

static filter_entry_t chain[MAX_FILTERS];
static size_t chain_length = 0;

void stir_register_filter(const char* type_name, void* instance, filter_process_fn fn, void* userdata) {
	if (chain_length >= MAX_FILTERS)
		return;

	chain[chain_length++] =
		(filter_entry_t){.type_name = type_name, .instance = instance, .process = fn, .userdata = userdata};
}

void stir_unregister_filter(void *instance)
{
	for (size_t i = 0; i < chain_length; ++i) {
		if (chain[i].instance == instance) {
			for (size_t j = i; j < chain_length - 1; ++j) {
				chain[j] = chain[j + 1];
			}
			chain_length--;
		}
	}
}

void stir_process_filters(stir_context_t *ctx)
{
	for (size_t i = 0; i < chain_length; ++i)
		chain[i].process(ctx, chain[i].userdata);
}