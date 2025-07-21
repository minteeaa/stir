STIR has no dedicated installer. installation is as follows.

you'll need the latest [release](https://github.com/minteeaa/stir/releases) zip for your platform.
> "rolling" releases can be found in the repo's [actions](https://github.com/minteeaa/stir/actions). bug reports will only be considered for stable and beta (rc) releases.

### windows
* locate and open your OBS installation directory.
* drag and drop (or extract) the `obs-plugins` folder from the release into the installation directory.
> there might be an existing `obs-plugins/` directory present, this is normal.

### linux
obs' plugin directory varies based on user configuration and distro. on many distros, the default is `/usr/lib/obs-plugins/`
* unzip the release.
* drag and drop `stir.so` using a file explorer, or `cp` the file into the plugin directory from the terminal. you may need root permissions.

after installation, you can open OBS and there should be a handful of new filters to use on any audio source.

### prerequisites

you may want to create a separate OBS profile for STIR. in your OBS `Settings`, in the `Audio` tab, set `Channels` to `Stereo` or higher. STIR supports anything above 1 channel (mono) audio.

### applying filters

to apply a filter, right-click any 2-channel audio source and select `Filters`. add a filter denoted by the `STIR` prefix. all STIR filters are processed in their own, isolated chain separate from OBS' normal filter chain, and as such will ignore any other filters applied to the source. 

for STIR to process an audio source, a `STIR Router` filter must be present. to recieve output, add a new source to the scene of type `STIR Virtual Out`. when prompted, `Add Existing` and select the `STIR -` prefixed source that matches the name of your original source.

```
source1 -> STIR - source1
```

the order of the filters does affect the final output, with the exception of the Router. the chain is processed from top to bottom, and can be updated in realtime through the OBS and filter's UI. any filter can affect a specific channel, selectable via the toggle boxes within the `Channels` section of each filter's UI. 

editing the STIR generated source is generally discouraged. standard OBS filters (for example, a limiter) can be applied without issue, but applying STIR filters to a STIR Virtual Out source is not supported and may result in unintended behavior.