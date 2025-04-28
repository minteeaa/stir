#include <obs-module.h>
#include <obs-audio-controls.h>

struct filter_channel_state 
{
	float hp_output_prev;
	float hp_input_prev;

	float lp_output_prev;
	float z1, z2;
	
	float lp_b0, lp_b1, lp_b2;
	float hp_b0, hp_b1, hp_b2;
	float a0, a1, a2;
	float x1, x2, y1, y2;
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

	char *lp_filter_type;
	float lp_cutoff;
	float lp_intensity;

	float hp_cutoff;
	float hp_intensity;

	float bp_cutoff_upper;
	float bp_cutoff_lower;
	float bp_intensity;

	struct filter_channel_state eq[MAX_AUDIO_CHANNELS];
};