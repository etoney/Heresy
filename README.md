# Heresy

Horror sound design synth by **Castle Danger Music Company**, as an AU / VST3 instrument and standalone app for macOS (Apple Silicon and Intel) and Windows.

It is the DAW version of the Heresy web app at [castledangermusic.com/project/heresy](https://castledangermusic.com/project/heresy), with the same engine, parameters and 15 factory presets.

## What's in it

- **Two oscillators + sub**, FM, ring mod, analog drift and white/pink/brown noise
- **Voicings**: Single, Octave Doom, Tritone, Minor 2nd, Diminished, Cluster, Abyss, Stinger (one key plays the whole voicing)
- **Dive**: pitch envelope for drops and rises
- **Filter**: low/high/band pass, notch and a formant (voice) filter with a vowel control, plus its own envelope
- **LFO** to pitch, filter and amp (sine, triangle, square, saw, random S&H)
- **Damage**: drive, bit crush, tape warble
- **Space**: delay and a long convolution reverb that can run in reverse
- **TORMENT** and **CRUSH** macros (the mod wheel adds Torment)
- **Touch pads**: Fear, Space and Motion, as in the web app
- Pitch bend (±2 semitones) and sustain pedal

User presets are saved to
`~/Library/Application Support/Castle Danger Music Company/Heresy/Presets` (Mac) or
`%APPDATA%\Castle Danger Music Company\Heresy\Presets` (Windows).

## Installers

Every push to `main` builds on GitHub Actions (**Actions → Build Heresy**). Download from the run's artifacts:

- `Heresy-mac` → `Heresy-<version>-mac.pkg` (choose AU, VST3 and/or the app)
- `Heresy-windows` → `Heresy-<version>-windows-setup.exe` (VST3 and/or the app)
- `Heresy-preset-renders` → a WAV of every factory preset from the headless test

Push a tag like `v1.0.0` to also publish both installers as a GitHub release.

### Mac signing and notarization

Without signing secrets the Mac installer is unsigned, so macOS will warn the first time. Right-click the .pkg and choose **Open**, or allow it in **System Settings → Privacy & Security**.

To ship a signed, notarized installer, add these repository secrets (**Settings → Secrets and variables → Actions**):

| Secret | What it is |
| --- | --- |
| `MACOS_CERT_APP_P12` | Developer ID Application certificate exported as .p12, base64 encoded (`base64 -i app.p12 \| pbcopy`) |
| `MACOS_CERT_INSTALLER_P12` | Developer ID Installer certificate as .p12, base64 encoded |
| `MACOS_CERT_PASSWORD` | Password used when exporting the .p12 files |
| `APPLE_ID` | Apple ID email of the developer account |
| `APPLE_APP_PASSWORD` | App-specific password from appleid.apple.com |
| `APPLE_TEAM_ID` | 10-character Team ID |

## Building locally

```sh
cmake -B build -G Xcode -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build build --config Release
installer/mac/build_pkg.sh 1.0.0 build/Heresy_artefacts/Release dist
```

JUCE 8 is downloaded automatically (or pass `-DJUCE_DIR=/path/to/JUCE`).

Fonts: Anton, Oswald and Courier Prime, SIL Open Font License 1.1 (see `assets/fonts`).
