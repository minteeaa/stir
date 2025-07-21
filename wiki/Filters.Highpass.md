### overview
<sup>as of: 1.0.0</sup>

the STIR Highpass applies a 2nd-order Butterworth highpass filter to the channel of choice.

### config
<sup>as of: 1.2.0</sup>

`cutoff` controls the cutoff frequency. frequencies above this value will be audible.

`Q` controls the Q factor of the Butterworth formula, calculating the alpha based off of this value. this value most closely relates to the "smoothness" of the falloff at the desired frequency. lower values will make the cutoff "sharper" while higher values will be "smoother."

`Wet Mix` controls the intensity of the highpass signal to be mixed into the output.

`Dry Mix` controls the intensity of the original signal to be mixed into the output.