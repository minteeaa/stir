for VRChat usage, many streams are tossed into [VRCDN](https://vrcdn.live/) for their low-latency audio livestreaming. streaming 5.1 channel audio has some important notes.

### audio track bitrate
in your OBS Settings, in the `Output` tab and inside the `Audio` section, each Track has a selectable bitrate. if your profile has 5.1+ audio enabled, you will be able to set each Track's bitrate higher than 320. for most intensive purposes, it's recommended to set the bitrate of the Track containing the STIR source to 800 or greater. (this is Track 1 by default.)

if you decide to use a Track bitrate of greater than 320, you may experience some audio glitches and instability throughout the stream's duration using RTSPT. in this case, use MPEG-TS links. note that MPEG-TS is not "low-latency" and can be quite a bit behind.