# GD Neural Auto 0.3 (Geode v5.11.1 / GD 2.2081 / Windows)

> Experimental Geode mod source code. NOT a prebuilt `.geode`. The GitHub Actions build has not been tested against Geode headers in this environment; compile failures may need a follow-up fix.

## Features
- Pause menu: **Train AI**, **Record Demo**, **Forget AI**.
- Main menu: AI help button.
- In-game HUD with current mode, trial count and best horizontal distance.
- Per-level persistent best input timeline using Geode save values.
- Evolutionary training: retain the furthest-reaching run and mutate button holds near the last frontier.
- Manual demonstration capture to seed a better policy.
- Never activates automatically when opening a new level; no auto-clicking if training is off.
- Native unit tests for the learning core in CI.

## Known limitations
- **Not a neural network**. Simple stochastic search, useful for learning experiments.
- Uses player X position buckets of 10 units; moving hazards, gravity changes, portals and variable timing may be unreliable.
- Requires repeated deaths and level restarts; does not guarantee completion.
- Does not support practice checkpoints, two-player mode or reliable platformer solving.
- Training is not intended for recording legitimate leaderboard completions.
- No actual Windows / Geode binary test performed here. GitHub Actions will validate the build.

## Installation without installing a compiler locally
1. Extract this ZIP so that `src`, `.github`, `mod.json`, `CMakeLists.txt`, `README.md` are **at the top level**. Do not upload the ZIP itself.
2. Upload all of those entries to the **root** of a new GitHub repo. Be careful not to create a `gd_neural_auto_v0.3/` folder in the repo. Remove old starter files first or use a fresh repository.
3. Confirm that `.github/workflows/build.yml` exists at that exact path (otherwise Actions will not start).
4. Commit, open **Actions** and wait for **Build GD Neural Auto (Windows)** to finish.
5. Download its `gd-neural-auto-v0.3-win64` artifact, unzip it, and place the `.geode` file at:
   `C:\Program Files (x86)\Steam\steamapps\common\Geometry Dash\geode\mods\`
6. Restart Geometry Dash.

## How to use
1. Open a normal non-dual level.
2. Pause and choose **Record Demo** to play and teach by example (recommended first). Restart the attempt, play, then let the run finish or die.
3. Pause and choose **Train AI**. **Restart** the attempt to enable new search runs.
4. The HUD will show its attempts and best progress. Turn off training by using **Stop AI**, then restart.
5. Use **Forget AI** to erase learned inputs only for this level.

## Structure
- `src/Brain.hpp`, `src/Brain.cpp`: standalone evolutionary search.
- `src/main.cpp`: Geode hooks, in-game UI, persistence and input injection.
- `tests/brain_tests.cpp`: portable algorithm tests.
- `.github/workflows/build.yml`: unit tests + Win64 Geode build.
