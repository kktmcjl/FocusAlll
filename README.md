# TrapVox AI VST3 — phase 2 starter

This is a JUCE/CMake C++ project for a VST3 vocal-effects plugin intended for FL Studio.

## Included in this phase
- VST3 plugin project scaffold
- Parameter state saved by the host
- High-pass, simple level-dependent compression, soft saturation, basic de-ess control,
  delay and JUCE reverb
- Style macro: Natural / Melodic Trap / Aggressive Trap
- GUI and automatable controls

## Not included yet
- **Real pitch correction / autotune.** The UI explicitly notes this. The next milestone should add a pitch detector and a pitch-shifting engine, then key/scale selection and retune speed.
- Beat-file analysis or tempo sync from an audio file. A VST insert receives the current vocal stream; it does not automatically get a separate beat file. Tempo can be read from the host playhead in a future version.
- This code has not been compiled or tested on your machine. Treat it as a development starter, not a ready-to-install binary.

## Build on Windows
1. Install Visual Studio 2022 with **Desktop development with C++**.
2. Install CMake 3.22+.
3. Download JUCE from https://juce.com/get-juce/ or clone https://github.com/juce-framework/JUCE
4. Open PowerShell in this folder. Replace `C:/JUCE` below with your JUCE directory:
   ```powershell
   cmake -S . -B build -DJUCE_DIR="C:/JUCE" -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release
   ```
5. Find `TrapVox AI.vst3` in the build output. If CMake's post-build copy did not place it automatically, copy the generated VST3 bundle to:
   `C:\Program Files\Common Files\VST3`
6. In FL Studio, open Plugin Manager, scan for plugins, then load TrapVox AI as an effect on the vocal mixer track.

## Important audio note
This is a prototype DSP chain. It is not a substitute for a properly tuned vocal processor. The current de-ess and compression are intentionally simple, and pitch correction is not present yet. Test at conservative levels and compare against bypass.
