## installation
- grab the latest [release](https://github.com/minteeaa/stir/releases)
- locate and open your OBS installation directory
- drag and drop (or extract) the `obs-plugins` folder from the release into the installation directory 
> there should be an existing `obs-plugins` folder present, this is normal; you're just merging the two

## obs profile setup
> [!NOTE]
> it is recommended to create a new OBS profile for STIR - these settings will not be handled correctly on a non-STIR video player

- the `channels` under the `Audio` tab of your settings should be set to `5.1` - STIR will not enable itself otherwise
- select any stereo audio source in your scene and add STIR as a `filter`
> currently, only `stereo` (2-channel) sources are supported; this may change in the future
- add a new `source` to your scene - STIR will have registered a new source type called `STIR_VIRTUAL_OUT`
- given the option, **add an existing source** and select the option that matches the name of the source STIR is applied to

any modifications to the filter UI will update immediately to the source