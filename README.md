<div align="center">
  <h1>STIR</h1>
  <h3>vrchat audio filters for the obs client</h3>
  <h1></h1>
</div>

### what is this
*audio filter/upmixer designed around vrchat usage*

vrchat has had a [lack of audio filter](https://feedback.vrchat.com/sdk-bug-reports/p/proposal-for-fixing-audio-filters-eg-low-pass-support-for-avpro) support for a few years, making systems like [ARC-DSP](https://www.patreon.com/Elevative) very appealing for better sounding audio used in music events

STIR started off as a server-hosted audio upmixer that applied simple audio filters separating the lows, mids, and highs of a stereo input and returned those in a channel setup usable within vrchat; it is now an OBS plugin

### supported environments
*versions n' stuff*

| OBS version | support |
|-------------|---------|
| 30.0.0 | supported |
| 31.0.0 | supported |

|   OS   | support |
|--------|---------|
| windows x64 | supported |
| macOS | unsupported |
| linux | unsupported |
