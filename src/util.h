#include <math.h>
#include <stdint.h>
#include <plugin-support.h>
#include <.deps/include/graphics/math-defs.h>

#include "struct.h"

// TODO: fix this horrible implementation and use for lowpass

void butterworth_init(struct stir_filter_data *filter) 
{
	float omega_c = 2.0f * M_PI * (filter->lp_cutoff / filter->sample_rate);
	float cos_omega = cosf(omega_c);
	float sin_omega = sinf(omega_c);
	float alpha = sin_omega / sqrtf(2.0f);

	float a0 = 1.0f + alpha;

	filter->b0 = (1.0f - cos_omega) / (2.0f * a0);
	filter->b1 = (1.0f - cos_omega) / a0;
	filter->b2 = filter->b0;

	filter->a1 = -2.0f * cos_omega / a0;
	filter->a2 = (1.0f - alpha) / a0;
}

float butterworth_process(struct stir_filter_data *filter, struct filter_channel_state *c, float in)
{
	float lowpass = filter->b0 * in + filter->b1 * c->z1 + filter->b2 * c->z2 - filter->a1 * c->z1 -
			filter->a2 * c->z2;

	c->z2 = c->z1;
	c->z1 = in;

	return lowpass;
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