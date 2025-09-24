<img src="meta/img/stir_banner.png" align="center"></img>
<h3 align="center">multichannel audio filter tools for OBS</h3>
<p align="center">
  <a title="OBS30+" href="https://obsproject.com/"><img src="https://img.shields.io/badge/OBS-30.0.0+-blue?style=flat-square" alt="OBS 30.0.0+"></a>
  <a title="GPL2" href="https://www.gnu.org/licenses/old-licenses/gpl-2.0"><img src="https://img.shields.io/github/license/minteeaa/stir?style=flat-square" alt="GPL 2.0"></a>
  <a title="Build" href="https://github.com/minteeaa/stir/actions"><img src="https://img.shields.io/github/actions/workflow/status/minteeaa/stir/push.yaml?style=flat-square" alt="Build Status"></a>
  <br>
  <a title="Codeberg" href="https://codeberg.org/mintea/stir"><img src="https://img.shields.io/badge/on-codeberg-blue?style=flat-square&logo=codeberg" alt="Codeberg @ mintea/stir"></a>
  <a title="Github" href="https://github.com/minteeaa/stir"><img src="https://img.shields.io/badge/on-github-black?style=flat-square&logo=github" alt="Github @ mintea/stir"></a>
</p>

---

> [!NOTE]
> the GitHub repository is a **mirror** of the primary repository on [Codeberg](https://codeberg.org/mintea/stir), and is subject to inconsistencies at any given time.

### what is this
*audio filter/upmixer designed for many channels*

STIR aims to be an easy-to-approach solution for virtual upmixing of 2-channel sources and applying filters onto their individual channels.

the primary inspiration for this project comes from VRChat's [ongoing lack of audio filter support](https://feedback.vrchat.com/sdk-bug-reports/p/proposal-for-fixing-audio-filters-eg-low-pass-support-for-avpro), but can be expanded to any valid usage case within OBS.

### usage
*the info, the guide*

refer to the [wiki](https://codeberg.org/mintea/stir/wiki) for setup and usage.

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

### credits
*friends who contributed directly and indirectly to this project*

**Crunchy Joints**, for constantly pushing STIR to its limits, most of the early/ongoing bughunting and testing efforts, and being an incredible second brain to spin up new ideas.

**Llamahat**, for the initial idea of turning this concept into an OBS plugin.
