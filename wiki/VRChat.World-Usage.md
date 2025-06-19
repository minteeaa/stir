### overview 

AVPro is the primary supported audio implementation in VRChat. STIR utilizes AVPro's ability to ingest and decode 6-channel audio separately per audio source, where we export each filter to a single track and use AVPro to control which sources get which audio.

Unity Audio is not supported.

### usage

* locate (or add) a `VRC AVPro Video Speaker` component attached to an `Audio Source`.
* use the `Mode` dropdown to assign a channel to the selected speaker.

AVPro's channel layout is as follows:

| channel |  AVPro mode  |
|---------|--------------|
| 0       | `Mono Left`  |
| 1       | `Mono Right` |
| 2       | `Three`      |
| 3       | `Four`       |
| 4       | `Five`       |
| 5       | `Six`        |

### important notes

* the `Stereo Mix` AVPro mode is a mix of channels `0` and `1` and cannot be edited manually.
* channel `3` is affected during runtime by AVPro applying its own in-built lowpass filter; it has been tested but it sounds like bass played through a glass of water. your mileage may vary.
* 8-channel audio is *technically* supported as AVPro does expose two extra channels; this may become a feature in the future but currently is limited by decoder support.