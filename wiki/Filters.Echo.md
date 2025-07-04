### overview
<sup>as of: 1.1.0</sup>

the STIR Echo filter repeats the current sampled audio after a given delay, feeding back into itself and attenuating the sound for an 'echo-like' falloff.

### config
<sup>as of: 1.1.0</sup>

`Delay` controls the delay of the attenuated echoes in milliseconds.

`Decay Ratio` is the multiplier for the attenuation of each echo in the series. `1.0` is no decay, `0.0` is full decay.

`Wet Mix` controls the intensity of the echo signal to be mixed into the output.

`Dry Mix` controls the intensity of the original signal to be mixed into the output.