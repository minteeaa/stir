## audio bitrate
you may find the default audio bitrate (`320`) is low and sounds compressed in VRChat; to remedy this, set the audio bitrate of the Track STIR uses to something larger (`800` is recommended)

![image](https://github.com/user-attachments/assets/1cc974c9-44f6-4a84-b67a-7b04f98eef82)

> [!IMPORTANT]  
> when streaming into VRChat at Track bitrates higher than `320` using VRCDN, do **not** use RTSPT or RTMP; use MPEG-TS

## sources
it is advised to mute the input source in the Audio Mixer to prevent overlapping input - the source will still output to STIR when muted in OBS

![{853FA2C4-B8E3-49E4-891D-86335B6C0482}](https://github.com/user-attachments/assets/97179a5b-78af-44a7-9776-ccd3dadc21f8)

## tracks
by default, the STIR source is set to output audio on `Track 1`, this will be customizable in the future, but for now make sure to double check the track configuration