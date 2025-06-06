#include <math.h>
#include <stdint.h>
#include <obs-module.h>

char *concat(const char *str1, const char *str2)
{
	size_t len1 = strlen(str1);
	size_t len2 = strlen(str2);

	char *result = (char *)bzalloc(len1 + len2 + 1);
	if (result == NULL) {
		perror("Failed to allocate memory");
		return NULL;
	}

	strcpy(result, str1);
	strcat(result, str2);
	return result;
}