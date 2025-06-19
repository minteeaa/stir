### overview
<sup>as of: 1.0.0</sup>

the STIR Router is the core of any chain applied to an audio source, as the STIR ecosystem utilizes its own isolated processing chain to apply effects. it is required to be present for any processing to occur. the location of the Router within the chain's order is not important.

within the Router's settings, you can configure which upmixed channels will recieve specific channels from the source audio stream.

`Mono Left` refers to the `left` channel of the source audio.

`Mono Right` refers to the `right` channel of the source audio.

`Stereo Mix` refers to both `left` and `right` channels of audio, averaged together via the formula `(left + right) * 0.5`

this channel configuration is applied before any additional filters are processed, and is important to configure correctly for accurate audio representation.

### defaults

standard 5.1 and 7.1 channel surround setups use the first two channels as `left` and `right`. these channels are set respectively in the Router's config by default, and should work for most compatible setups. the rest of the channels can be set to preference, but from testing sound most normal set to `Stereo Mix`.