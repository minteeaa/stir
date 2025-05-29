## audio bitrate
you may find the default audio bitrate (`320`) is low and sounds compressed in VRChat; to remedy this, set the audio bitrate of the Track STIR uses to something larger (`800` is recommended)

![audio-bitrate](./img/post-setup.audio-bitrate)

> [!IMPORTANT]  
> when streaming into VRChat at Track bitrates higher than `320` using VRCDN, do **not** use RTSPT or RTMP; use MPEG-TS

## sources
it is advised to mute the input source in the Audio Mixer to prevent overlapping input - the source will still output to STIR when muted in OBS

![mute-obs](./img/post-setup.mute-obs)

## tracks
by default, the STIR source is set to output audio on `Track 1`, this will be customizable in the future, but for now make sure to double check the track configuration