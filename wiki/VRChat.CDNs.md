for VRChat usage, many streams are tossed into [VRCDN](https://vrcdn.live/) for their low-latency audio livestreaming. streaming 5.1 channel audio has some important notes.

### audio track bitrate
in your OBS settings, `Output` tab in the `Audio` section, each Track has a selectable bitrate. if your profile has 5.1+ audio enabled, you will be able to set each Track's bitrate much higher than 320. for most intensive purposes, it's recommended to set the bitrate of the Track containing the STIR source to 800 or greater.

if you decide to use a Track bitrate of greater than 320, you **should not** use RTSPT links in VRChat's video players. there is a high probability of experiencing major audio glitches and instability throughout the stream's duration. setting the Track bitrate to 320 or lower leaves RTSPT links unaffected, this is an issue with higher bitrates only.

if you decide to use higher bitrates, use MPEG-TS links. note that you will be sacrificing the low-latency nature of RTSPT for the stability and quality of MPEG-TS.