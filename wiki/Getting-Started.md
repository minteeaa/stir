STIR has no dedicated installer, the process is straightforward:
* grab the latest [release](https://github.com/minteeaa/stir/releases) zip.
* locate and open your OBS installation directory.
* drag and drop (or extract) the `obs-plugins` folder from the release into the installation directory.
> there might be an existing `obs-plugins/` directory present, this is normal.

after installation, you can open OBS and there should be a handful of new filters to use on any audio source.

### prerequisites

you may want to create a separate OBS profile for STIR.

in your OBS `Settings`, in the `Audio` tab, set `Channels` to `5.1` or higher. 

### applying filters

to apply a filter, right-click any 2-channel audio source and select `Filters`. add a filter denoted by the `STIR` prefix. all STIR filters are processed in their own, isolated chain separate from OBS' normal filter chain, and as such will ignore any other filters applied to the source.

to recieve output, add a new source to the scene of type `STIR Virtual Out`. when prompted, `Add Existing` and select the `stir_output_` prefixed source that matches the name of your original source.

```
source1 -> stir_output_source1
```

the order of the filters does affect the final output, with the exception of the Router. the chain is processed from top to bottom, and can be updated in realtime through the OBS and filter's UI.

any filter can affect a specific channel, selectable via the toggle boxes within the `Channels` section of each filter's UI.

