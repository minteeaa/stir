#include <math.h>
#include <stdint.h>
#include <plugin-support.h>
#include <.deps/include/graphics/math-defs.h>

#include "struct.h"

void butterworth_calculate_lowpass(struct stir_filter_data *filter, struct filter_channel_state *c)
{
	float omega_c = 2.0f * M_PI * (filter->lp_cutoff / filter->sample_rate);
	float alpha = sinf(omega_c) / (2 * sqrtf(2.0f));

	float cos_omega_c = cosf(omega_c);

	c->lp_b0 = (1.0f - cos_omega_c) / 2.0f;
	c->lp_b1 = 1.0f - cos_omega_c;
	c->lp_b2 = (1.0f - cos_omega_c) / 2.0f;

	c->a0 = 1 + alpha;
	c->a1 = -2.0f * cos_omega_c;
	c->a2 = 1.0f - alpha;

	c->lp_b0 /= c->a0;
	c->lp_b1 /= c->a0;
	c->lp_b2 /= c->a0;

	c->a1 /= c->a0;
	c->a2 /= c->a0;
}

void butterworth_calculate_highpass(struct stir_filter_data *filter, struct filter_channel_state *c)
{
	float omega_c = 2.0f * M_PI * (filter->hp_cutoff / filter->sample_rate);
	float alpha = sinf(omega_c) / (2 * sqrtf(2.0f));

	float cos_omega_c = cosf(omega_c);

	c->hp_b0 = (1.0f + cos_omega_c) / 2.0f;
	c->hp_b1 = -(1.0f + cos_omega_c);
	c->hp_b2 = (1.0f + cos_omega_c) / 2.0f;

	c->a0 = 1 + alpha;
	c->a1 = -2.0f * cos_omega_c;
	c->a2 = 1.0f - alpha;

	c->hp_b0 /= c->a0;
	c->hp_b1 /= c->a0;
	c->hp_b2 /= c->a0;

	c->a1 /= c->a0;
	c->a2 /= c->a0;
}


float butterworth_filter(int type, struct stir_filter_data *filter, struct filter_channel_state *c, float in)
{
	float out = 0.0f;

	if (type == 0) {
		out = (c->lp_b0 * in + c->lp_b1 * c->x1 + c->lp_b2 * c->x2 - c->a1 * c->y1 - c->a2 * c->y2) * filter->lp_intensity;
	} else if (type == 1) {
		out = (c->hp_b0 * in + c->hp_b1 * c->x1 + c->hp_b2 * c->x2 - c->a1 * c->y1 - c->a2 * c->y2) * filter->hp_intensity;
	}

	c->x2 = c->x1;
	c->x1 = in;
	c->y2 = c->y1;
	c->y1 = out;

	return out;
}

float simple_lowpass(struct stir_filter_data *filter, struct filter_channel_state *c, float in, float cutoff, float intensity)
{
	float rc = 1.0f / (cutoff * 2 * M_PI);
	float dt = 1.0f / filter->sample_rate;

	float lp_raw_alpha = dt / (rc + dt);
	float alpha = lp_raw_alpha * intensity;

	float out = c->lp_output_prev + (alpha * (in - c->lp_output_prev));
	c->lp_output_prev = out;

	return out;
}

float simple_highpass(struct stir_filter_data *filter, struct filter_channel_state *c, float in, float cutoff, float intensity)
{
	float rc = 1.0f / (cutoff * 2 * M_PI);
	float dt = 1.0f / filter->sample_rate;

	float hp_raw_alpha = rc / (rc + dt);
	float alpha = hp_raw_alpha * intensity;

	float out = alpha * (c->hp_output_prev + in - c->hp_input_prev);
	c->hp_input_prev = in;
	c->hp_output_prev = out;

	return out;
}

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