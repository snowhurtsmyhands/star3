# StarFlux

StarFlux is a JUCE C++17 audio-effect plugin (VST3 + Standalone) featuring a GPU-rendered, audio-reactive cinematic star field.

## Features
- Stereo pass-through audio effect
- OpenGL star rendering with deterministic seed-based layouts
- Audio analysis (RMS, peak, low/mid/high bands, transient)
- Motion presets + manual XYZ velocity offsets
- Constellation, Void, CRT, Twinkle, Wave Distortion toggles
- Glitch section (pixelate, databend, frame skip with controlled randomness)
- Internal visual loop timeline with optional host loop follow
- Clean default UI + expandable advanced panel

## Build (Windows)
1. Install CMake (3.22+) and Visual Studio 2022 with C++ desktop tools.
2. JUCE is included as a git submodule (see Setup below)
## Setup

This project uses JUCE as a git submodule.

Clone the repository with submodules:

```bash
git clone --recurse-submodules <your-repo-url>

3. Configure:
   ```bash
   cmake -B build -G "Visual Studio 17 2022" -A x64
   ```
4. Build:
   ```bash
   cmake --build build --config Release
   ```
5. Load `StarFlux.vst3` from the produced build artifacts in your VST3 host.

## Assumptions
- JUCE is available via `add_subdirectory(JUCE)`.
- OpenGL 3.2+ compatible context is available on host system.
- Host may or may not provide loop/cycle timing; plugin falls back gracefully.
