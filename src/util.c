#include <stdint.h>
#include <obs-module.h>

#include "plugin-support.h"

char *concat(const char *str1, const char *str2)
{
	size_t len1 = strlen(str1);
	size_t len2 = strlen(str2);

	char *result = (char *)bzalloc(len1 + len2 + 1);
	if (result == NULL) {
		obs_log(LOG_ERROR, "failed to allocate memory for concat job");
		return NULL;
	}

	strcpy(result, str1);
	strcat(result, str2);
	return result;
}
