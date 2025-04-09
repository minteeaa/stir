#include <obs-module.h>
#include <obs-audio-controls.h>

#pragma once

struct filter_channel_state 
{
	float hp_output_prev;
	float hp_input_prev;

	float lp_output_prev;
	float z1, z2;
};

struct stir_filter_data
{
	obs_source_t *virtual_source;
	float *upmix_buffer[MAX_AUDIO_CHANNELS];
	obs_source_t *context;
	obs_source_t *parent;
	const char *parent_name;
	size_t channels;
	float sample_rate;

	float lp_cutoff;
	float lp_intensity;

	float hp_cutoff;
	float hp_intensity;

	float bp_cutoff_upper;
	float bp_cutoff_lower;
	float bp_intensity;

	float b0, b1, b2, a1, a2;

	struct filter_channel_state eq[MAX_AUDIO_CHANNELS];
};