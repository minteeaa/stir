> [!NOTE]
> there are available preset UnityPackages to streamline installing STIR for your project - [more info here](https://github.com/minteeaa/stir/wiki/Video-Player-Presets)

## rundown
this utilizes AVPro's ability to ingest and decode 6-channel audio separately per audio source, we export each filter to a single track and use AVPro to control which sources get which audio

> Unity Audio has **not** been tested and is likely not to work, AVPro is generally a better choice in every circumstance

## usage
* locate (or add) a `VRC AVPro Video Speaker` component attached to an `Audio Source`
* use the `Mode` dropdown to assign a channel to the selected speaker

## channel layout
> [!NOTE]
> this channel layout **will** be customizable in the future, this information is **highly volatile**

### layout
currently, the channel layout is as follows:

| channel   | response          | AVPro mode   |
|-----------|-------------------|--------------|
| 0         | `left [mono]`     | `Mono Left`  |
| 1         | `right [mono]`    | `Mono Right` |
| 2         | `mids (bandpass)` | `Three`      |
| 3         | `none`            | `Four`       |
| 4         | `lowpass`         | `Five`       |
| 5         | `highpass`        | `Six`        |

### details
* channels `0` and `1` receive the original, unedited `left` and `right` channels of the input source
* channel `2` takes the `left` channel, applies a combination of lowpass and highpass to isolate a frequency range
* channel `3` is unused (refer to the footnote)
* channel `4` takes the `left` channel, applies a lowpass filter and an optional tremolo LFO
* channel `5` takes the `left` channel and applies a highpass filter

> while all of these channels ingest the `left` channel, it is important to remember each channel is `mono` and will not be panned left or right by STIR or OBS

## footnote
* the `Stereo Mix` AVPro mode is a mix of channels `0` and `1` and cannot be edited manually
* channel `3` does not receive audio due to AVPro applying its own in-built lowpass filter upon runtime; it has been tested but it sounds like bass played through a glass of water
* 8-channel audio is *technically* supported as AVPro does expose two extra channels; this may become a feature in the future but currently is limited by encoder support