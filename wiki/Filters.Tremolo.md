### overview
<sup>as of: 1.0.0</sup>

the STIR Tremolo filter applies a Low Frequency Oscillator (LFO) to the channel of choice. it is recommended to apply this to a channel affected by a STIR Lowpass.

an LFO will oscillate the volume of an input audio stream at a consistent rate. in graphs, this is the equivalent of a standard Sine wave, where `depth` is the amplitude and `rate` is the wavelength.

in audio, this is applied to a low-frequency audio stream to emulate physical vibrations of a subwoofer speaker and reflection from the surrounding environment.

[hhsprings' ffmpeg examples](https://hhsprings.bitbucket.io/docs/programming/examples/ffmpeg/manipulating_audio/tremolo.html) contains a visualization.

### config
<sup>as of: 1.0.0</sup>

`rate` controls the speed at which oscillation occurs. higher values oscillate faster.

`depth` controls the intensity of the volume modulation from the oscillation. the volume of the input audio will be reduced during oscillation by a maximum defined by this value.