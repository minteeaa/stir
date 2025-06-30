<img src="meta/img/stir_banner.png" align="center"></img>
<h3 align="center">multichannel audio filter tools for OBS</h3>
<p align="center">
  <a title="OBS30+" href="https://obsproject.com/"><img src="https://img.shields.io/badge/OBS-30.0.0+-blue?style=flat-square" alt="OBS 30.0.0+"></a>
  <a title="GPL2" href="https://www.gnu.org/licenses/old-licenses/gpl-2.0"><img src="https://img.shields.io/github/license/minteeaa/stir?style=flat-square" alt="GPL 2.0"></a>
</p>

---

### what is this
*audio filter/upmixer designed for many channels*

STIR aims to be an easy-to-approach solution for virtual upmixing of 2-channel sources and applying filters onto their individual channels.

the primary fix this attempts to present is for VRChat's [ongoing lack of audio filter support](https://feedback.vrchat.com/sdk-bug-reports/p/proposal-for-fixing-audio-filters-eg-low-pass-support-for-avpro).

STIR started off as a server-hosted audio upmixer that applied simple audio filters separating the lows, mids, and highs of a stereo input and returned those in a channel setup usable within vrchat; it is now an OBS plugin with an expandable feature set.

### usage
*the info, the guide*

refer to the [wiki](https://github.com/minteeaa/stir/wiki) for setup and usage.

### supported environments
*versions n' stuff*

| OBS    | support   |
|--------|-----------|
| 30.0.0 | supported |
| 31.0.0 | supported |

| OS          | support     |
|-------------|-------------|
| windows x64 | supported   |
| linux       | supported   |
| macOS       | unsupported |

> linux has been tested and confirmed to work on Arch Linux. other distros *should* be supported by default. there are no current plans to support macOS.