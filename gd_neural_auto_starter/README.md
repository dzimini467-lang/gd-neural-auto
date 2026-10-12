# GD Neural Auto (starter)

A minimal Windows Geode mod for Geometry Dash 2.2081 / Geode 5.11.1.

This is stage 1 of the project: it adds a menu button that displays a test message.
**There is no AI/autoplay in this build yet.**

## Build without local programming tools

1. Create a new empty GitHub repository with its default branch named `main`.
2. Extract the ZIP and upload **the contents** (not the ZIP itself) via GitHub's `Add file > Upload files`.
3. Commit the files. GitHub Actions should run `Build GD Neural Auto`.
4. Open the repository's Actions tab, choose the successful run, and download the artifact `gd-neural-auto-windows`.
5. Extract the artifact ZIP, then copy `neural.auto.geode` to:
   `C:\Program Files (x86)\Steam\steamapps\common\Geometry Dash\geode\mods\`
6. Restart Geometry Dash, look for the button near the top-right of the main menu, and click it.

The build workflow uses the official `geode-sdk/build-geode-mod` action and pins the target SDK to `v5.11.1` via `mod.json`.

If the Actions build fails, inspect the logs and adjust the code or SDK version accordingly. The project has not been compiled in this environment.
