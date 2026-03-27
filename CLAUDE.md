# AMLP - Audio Multi-Loop Performer

## Build & Test Pipeline

**IMPORTANT**: After every build or test, ALWAYS read `logs/errors_latest.log` to check for errors/warnings. Never ask the user to paste logs — they are all in `logs/`.

### Scripts
- `scripts/build.sh` — Configure + build. All output saved to `logs/`.
- `scripts/test.sh` — Build + run test target. All output saved to `logs/`.
- `scripts/ci.sh` — Full pipeline: build then test. All output saved to `logs/`.

### Log Files (always check after builds)
| File | Contents |
|---|---|
| `logs/errors_latest.log` | **Check this first.** Extracted errors + warnings from last run. |
| `logs/build_latest.log` | Full build stdout+stderr from last build. |
| `logs/test_latest.log` | Full test output from last test run. |
| `logs/cmake_latest.log` | CMake configure output. |
| `logs/ci_latest.log` | Full CI pipeline output. |
| `logs/pipeline_latest.log` | Build script's own pipeline output. |
| `logs/*_YYYYMMDD_HHMMSS.log` | Timestamped archives of every run. |

### Workflow
```bash
# Full build + test
scripts/ci.sh

# Then check errors (do this every time!)
cat logs/errors_latest.log
```

## Architecture
- **JUCE** (develop, aligned with Tracktion Engine) + **Tracktion Engine** (develop, v3.2.0+)
- CMake build system, C++20, macOS standalone app
- `DONT_SET_USING_JUCE_NAMESPACE=1` — always use `juce::` and `te::` prefixes
- Test framework: JUCE UnitTest (`juce::UnitTest`)

## Key Rules
- **NEVER cause audio gaps or stops.** Gapless operation is the #1 constraint.
- Audio thread: no allocations, no locks, no model calls.
- Overdubs use multi-clip layering (not bounce-on-stop).
- Test target: `AMLPTests` (JUCE UnitTest framework)

## Project Structure
```
source/
  Main.cpp, MainComponent.h/.cpp     — App entry point and root UI
  core/                               — Audio engine, loop management
  ui/                                 — JUCE UI components
  effects/                            — Effects chain management
  midi/                               — MIDI mapping, foot pedal
  drum/                               — Drum machine (future)
tests/                                — Unit tests (JUCE UnitTest)
scripts/                              — Build/test/CI scripts
logs/                                 — ALL build/test output (gitignored)
resources/                            — Audio samples, presets
```
