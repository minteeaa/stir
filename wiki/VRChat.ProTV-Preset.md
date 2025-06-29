> [!NOTE]
> the latest versions of the preset UnityPackages can be found [here](https://github.com/minteeaa/stir/tree/master/meta/packages)

### overview
using the preset UnityPackage for ProTV is straightforward:

- locate the prefab in `/Assets/mintea/STIR/ProTV`.
- drag the STIR prefab into the base hierarchy of the ProTV prefab.

![hierarchy-drag](./img/presets.hierarchy-drag.png)

STIR should now be available as a ProTV preset mode, selectable from both the hierarchy and from the in-game UI.

![select-stir](./img/presets.select-stir.png)

relocate the speakers, experiment and find a setup that works for you!

### defaults
the preset audio sources can be changed to accomodate any setup. by default, the intended channel configuration for this preset is:


| channel   | response          | AVPro mode   |
|-----------|-------------------|--------------|
| 0         | `left [mono]`     | `Mono Left`  |
| 1         | `right [mono]`    | `Mono Right` |
| 2         | `mids (bandpass)` | `Three`      |
| 3         | `unused`          | `Four`       |
| 4         | `lowpass`         | `Five`       |
| 5         | `highpass`        | `Six`        |

`bandpass` is a mixture of `lowpass` and `highpass` to isolate a range of frequencies.