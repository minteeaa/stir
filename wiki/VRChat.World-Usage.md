### overview 

AVPro is the primary supported audio implementation in VRChat. STIR utilizes AVPro's ability to ingest and decode 6-channel audio separately per audio source, where we export each filter to a single track and use AVPro to control which sources get which audio.

Unity Audio is not supported.

### usage

* locate (or add) a `VRC AVPro Video Speaker` component attached to an `Audio Source`.
* use the `Mode` dropdown to assign a channel to the selected speaker.

AVPro's channel layout is as follows:

| channel |  AVPro mode  |
|---------|--------------|
| 1       | `Mono Left`  |
| 2       | `Mono Right` |
| 3       | `Three`      |
| 4       | `Four`       |
| 5       | `Five`       |
| 6       | `Six`        |

### important notes

* the `Stereo Mix` AVPro mode is a mix of channels `1` and `2` and cannot be edited manually.
* channel `4` is affected during runtime by AVPro applying its own in-built lowpass filter; it has been tested but it sounds like bass played through a glass of water. your mileage may vary.
* 8-channel audio is *technically* supported as AVPro does expose two extra channels; this may become a feature in the future but currently is limited by decoder support.