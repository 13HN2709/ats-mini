# ATS Mini Development Handoff

Last updated: 2026-10-29 (VHF scanner UI/peak-navigation changes committed; build pending)
Repository: https://github.com/13HN2709/ats-mini
Active work branch: `development` (never modify `main` unless explicitly requested)

## Working agreement
- Keep this file current after every meaningful feature or code change. Use it as the handoff reference when starting a new chat.
- New features must be isolated from existing features wherever practical. If implementing a feature would require changing shared feature-specific logic, duplicate that logic into a new module/state/handler for the new feature rather than refactoring the established feature.
- Preserve Dual RX, existing Web Spectrum behavior, Clean Layout and especially Layout-TEF6686 unless a change is explicitly required.
- Shared low-level radio/scanning primitives may be reused only after checking their side effects and compatibility.
- Build workflow is manual (`workflow_dispatch`). Do not claim a build/device test passed unless verified. User launches build and tests on ATS Mini.
- Keep branch hygiene simple; development first. User manages copying approved README/release changes to main unless explicitly asked.

## Current task: native VHF Spectrum Scanner
### Intended user experience
1. Main menu has XXL `SPECTRUM` category.
2. Spectrum submenu has `VHF Spectrum` and `ALL Band Spectrum`. The all-band item is a future implementation and must not accidentally start the VHF scan.
3. VHF scanner asks for start frequency and end frequency sequentially with its own keypad/input flow.
4. Valid range: 88.00–108.00 MHz, end frequency strictly greater than start.
5. Invalid input shows an error and does not start scanning or change the receiver.
6. Show a visible “SCANNING SPECTRUM” status/range before the sweep.
7. Show graph; encoder rotation selects a point, short click accepts the selected frequency and exits.
8. Cancel exits cleanly and should not tune/change the receiver as a side effect.
9. Match the dark-blue/cyan/green TEF6686 visual style.

### Isolation requirements
- Do not alter Dual RX's `dualRxSetupStage`, keypad functions, or click behavior to implement the scanner.
- Do not reuse `CMD_FREQ_KEYPAD` for the scanner. Add a dedicated scanner-input command/state and separate keypad handling/drawing, ideally in scanner-specific module(s).
- Existing `FreqKeypad.cpp/.h` is used by radio tuning and Dual RX. Treat it as protected.
- Existing web scan engine is in `Scan.cpp`; the native VHF scanner now uses a separate private sample buffer and sweep implementation in `SpectrumScanner.cpp` to avoid overwriting web scan results.
- `Network.cpp` should not be changed unnecessarily; existing web-spectrum functionality must remain intact.
- Scanner code must explicitly prevent the `ALL Band Spectrum` menu item from invoking the VHF scan until all-band support is implemented.


### New UI requirements confirmed on 2026-10-28
- VHF Spectrum keypad must visually match the existing native frequency keypad: same compact black panel, blue double frame, frequency field, fixed-position FM indicator, 4x4 key geometry, cyan selection state, and font treatment.
- VHF keypad is FM-only and MHz-only. No selectable modulation control and no kHz toggle. The FM indicator and MHz key are fixed informational/entry labels, not mode/unit switches.
- Future ALL Band Spectrum keypad must be a separate mode: offer only USB, LSB, AM (no FM) and allow kHz entry only (no MHz). Do not implement or expose these options on the VHF keypad.
- The native VHF keypad drawing function was updated in commit `4414b3ccb32d93a313916a2ec1237cfa47232afe` to use the existing keypad's geometry/colors/fonts and fixed FM/MHz labels. This is source-only; no build or device test has been run.
- The VHF keypad close button label was aligned with the existing keypad (`CLOSE`) in commit `9a4d9c2fbda1796ed48e7e301c9c2a9b843aafd5`.
- Native scanner tuning finds an FM band that covers the requested range, configures FM before the sweep, and restores the previously selected band/frequency/mode/BFO after scanning. Accepting a graph point explicitly switches to the FM band covering that frequency. Graph long-press exits without tuning. Commits: `827091d16d4f46625f5ad989e7a3ebb3bccc5305`, `42f34a90788a0c6f5d971ad76174f29323873381`, `0cae2833afec6acf20bb44e3740996af079a11b1`, `d7b79986eeab993828456faf633750e4769cf07b`.
- Native VHF Spectrum now has its own 201-point RSSI/SNR sample buffer and isolated sweep loop in `SpectrumScanner.cpp`; it no longer uses or overwrites the Web Spectrum's `Scan.cpp` sample buffer. The native loop keeps its own data and tunes the shared physical receiver only while scanning. Commits: `ff0ed62319b51e75ccf2e509808491abd9216a2a`, `5e55a3ae099c768afdd9b153ab547a0d9c6a3fe3`, `dca1574dd612a8f95cd402ade25287dad77bb0a4`, `bc350911571369583d39e77ef5d3e67bf9f2bd8`, `07bac5f4b21d4ca70bdb6e62a74071758360e425`.

## Current repository state observed on 2026-10-28
- Branch: `development`.
- `ats-mini/SpectrumScanner.cpp/.h` now contain scanner-owned input state and a dedicated numeric keypad for start/end frequencies; the existing `FreqKeypad.cpp/.h` is not modified.
- Native scan sample storage/sweep is now isolated from Web Spectrum; only the physical SI4735 receiver is necessarily shared.
- `CMD_SPECTRUM_INPUT` is a separate command. `Draw.cpp` renders its own input screen and `ats-mini.ino` dispatches its own encoder input, outside the Dual RX block.
- The VHF input accepts 88.00–108.00 MHz, accepts dot or comma decimals up to two decimal places, and rejects an end frequency that is not greater than the start. The scan graph no longer tunes on encoder rotation; a short click tunes only the accepted point, while long press exits without tuning.
- `Menu_german.cpp.src` localizes the Spectrum submenu. The all-band placeholder is guarded so it does not start VHF scan.
- `Network.cpp` ignores pending web spectrum commands while native spectrum input is active.
- Source review completed on 2026-10-28: delimiter-balance check passed, no calls remain to the shared web-scan APIs in `SpectrumScanner.cpp`, and the workflow remains `workflow_dispatch` only. The first manual build (run 40781905177, job `build ospi-de`) failed because `SpectrumScanner.cpp` called `muteOn(MUTE_TEMP, ...)` without including `Utils.h`, where both are declared. Fixed by adding `#include "Utils.h"` and clarified the keypad input condition's operator parentheses in commit `db8c1f7fe7ab23911d7a65ea53b779ebb893f8c3`. A new build is still required; no successful compile or hardware test is confirmed.
- `Menu.h` defines `CMD_SPECTRUM` and `CMD_SPECTRUM_MENU`.
- English/German menu templates include Spectrum and submenu entries; the click handler currently starts the VHF scanner only for submenu index 0, so index 1 must be handled safely.
- `Draw.cpp` routes `CMD_SPECTRUM_MENU` to the full-screen menu and `CMD_SPECTRUM` to `drawSpectrumScanner()`.
- `ats-mini.ino` has dedicated encoder/click handling for `CMD_SPECTRUM_INPUT` and `CMD_SPECTRUM`; it stays outside the Dual RX block. Short click accepts a graph point; long press exits without tuning.
- `FreqKeypad.cpp/.h` implements the existing radio/Dual RX keypad and must remain unchanged for this feature.
- Native VHF scanning no longer calls `Scan.cpp`; the web scanner's existing global result buffer is untouched.
- GitHub Actions workflow was changed earlier to `ubuntu-24.04` for both jobs after an Ubuntu runner/Python issue. Do not revert without a reason.

## Immediate next steps
1. User launches the manual GitHub Actions workflow on `development` and checks all four OSPI/QSPI × German/English builds.
2. After the fix commit `db8c1f7fe7ab23911d7a65ea53b779ebb893f8c3`, rerun the manual workflow on `development`. If another build fails, use the exact compiler output to fix only the affected code and update this handoff.
3. After a successful build, flash/test the ATS Mini V4: start/end entry, invalid range, cancel without retune, full sweep/status, graph navigation, short-click tune, long-press exit, and confirm Web Spectrum + Dual RX still work.
4. Do not merge/copy to `main` unless the user explicitly asks.

## Validation checklist (must be checked before asking user to build)
- [x] VHF start and end are entered in separate scanner-owned stages (source reviewed).
- [x] Dot/comma decimal parsing works up to two decimal places (source reviewed).
- [x] Range and ordering validation reject invalid values before radio changes (source reviewed).
- [x] Input cancel/validation errors do not retune; scan restores prior state before graph navigation (source reviewed).
- [x] Encoder navigation and short-click selection are wired to scanner-specific handlers (source reviewed; hardware test pending).
- [x] Long press cancels keypad or exits graph without tuning; Dual RX handling block remains unchanged (source reviewed).
- [x] ALL Band submenu item cannot start VHF scanning (source reviewed).
- [x] Existing Dual RX-specific keypad and state logic were not modified for the scanner (source review; hardware regression test pending).
- [x] Native scanner no longer shares Web Spectrum sample buffer (source reviewed; hardware regression test pending).
- [ ] Native sweep tune-complete polling and timeout behavior reviewed against existing scan loop.
- [x] First build failure fixed at source level: `Utils.h` is now included for `muteOn` and `MUTE_TEMP`; rebuild pending.
- [x] Native sweep configures a covering FM band and restores previous band/frequency/mode/BFO after scanning (source reviewed; hardware test pending).
- [x] German and English menu templates have VHF/All Band labels and guarded click handling (source reviewed).
- [x] VHF keypad uses matching geometry/colors/fonts and `CLOSE` label (visual confirmation requires device test).
- [x] VHF keypad exposes only fixed FM and MHz; no mode or kHz switching (source reviewed).
- [x] Future ALL Band requirement is documented; implementation is intentionally not present yet.
- [x] Build workflow uses `workflow_dispatch`; no workflow was triggered by the assistant.
- [x] Handoff updated with actual changes and remaining build/device validation.

## Changes made in this session
- Added `DEVELOPMENT_HANDOFF.md` on `development` (initial handoff commit `bee78177301f4d2be6759f40f5de46ed0c55522f`).
- Added `CMD_SPECTRUM_INPUT` and scanner-specific keypad API/state.
- Added independent VHF start/end frequency entry and basic range validation in `SpectrumScanner.cpp`.
- Added keypad drawing dispatch in `Draw.cpp`; added encoder input dispatch in `ats-mini.ino` without changing the Dual RX block itself.
- Updated English/German menu click handling so only VHF item starts scanner; German submenu localized.
- Updated `Network.cpp` to ignore web-spectrum command processing during native scanner input.
- Additional commits: `b1b657d3355526920328069a31735eeab1babfa3` (corrected label line breaks), `1c6861a7427736b7670afbe82a870d7892e174f1` (web-spectrum command isolation), `7d5d86e1a54f7db04ad28709a180c4ec0fac5c5d` (consume menu-opening click so it cannot enter a keypad digit). The handoff itself was updated in `90effb849c89d8b4c867808c5aaf6e13a572ebbd` before these final edits.
- Still required before user build: finish the source-level review of the isolated native sweep and all encoder/UI state transitions. No compile or hardware test has been run.


## Build readiness
- First manual build attempt failed in `build ospi-de` on undeclared `muteOn`/`MUTE_TEMP`; the fix is committed as `db8c1f7fe7ab23911d7a65ea53b779ebb893f8c3` (added `Utils.h`, clarified a warning-prone condition). Rerun the manual workflow on `development` before flashing.
- Ready for the user to click **Run workflow** on the `development` branch.
- The workflow matrix builds OSPI/QSPI firmware in both German and English (four builds).
- This is a source-level review only. Do not describe the build as passing until GitHub Actions completes successfully.
- Current relevant source commit: `07bac5f4b21d4ca70bdb6e62a74071758360e425`; latest handoff updates continue after it.


## Future layout reference: GNBD miniradio (requested 2026-10-28)
- User wants to recreate the screenshot's receiver layout as close to 1:1 as possible in the ATS Mini V4 firmware, using the reference source at https://github.com/GNBD/ats-mini-dualboot/tree/main/GNBD%20miniradio.
- The supplied screenshot is the visual target: dark near-black background, compact top status bar with time/signal/battery, VHF FM heading, large frequency digits, RSSI/SNR horizontal meters, right-side compact receiver settings, large RDS panel at bottom-left, and small SIG graph panel at bottom-right.
- Specific adjustment requested: raise the “MHz” unit slightly higher relative to the frequency digits.
- This is a future layout task, not part of the current VHF Spectrum Scanner build fix. Before implementing, inspect the reference repository's actual source files and map the target drawing to the user's current layout architecture. Keep Layout-TEF6686.cpp and Clean Layout protected unless the user chooses the destination layout.


## Layout default change (requested 2026-10-28)
- User wants Layout-TEF6686 to be the standard/default layout, including on a fresh installation.
- In both English and German menu source templates, `uiLayoutIdx` now initializes to `5` (TEF6686), and the layout formerly labeled `Default` is now labeled `Original`.
- In `Draw.cpp`, the invalid/unrecognized layout-index fallback now draws `Layout-TEF6686` rather than `Layout-Default`.
- Commits: `8eca92ce3358c38ebf60950a06836a1de3a746e8` (English default/label), `0adefb7ac7154f7bd627de7d6c562d472e45dc80` (German default/label), `ffb2b1cc284564090e306f0419591db3afabc195` (render fallback).
- Existing devices with a saved `UILayout` preference will keep that saved choice; fresh installs/reset preferences should initialize to TEF6686. Build and hardware confirmation pending.


## VHF scanner keypad hang fix (2026-10-29)
- User reported the ATS Mini V4 hung while rotating the native VHF Spectrum keypad from the MHz key toward CLOSE (photo showed the input screen stuck at `--- MHz`).
- Root cause found in `spectrumScannerInputRotate()`: when the next key index was the intentionally blank slot 14, a `do/while(next == 14)` loop never changed `next`, causing an infinite loop and a hard UI freeze.
- Fixed on `development` in commit `e35a2081c69861e71b284a4193f7ef7201ced944`: advance one additional step when landing on slot 14, with wrap-around handling. This directly addresses the freeze path without touching Web Spectrum, Dual RX, or existing layouts.
- Not yet compiled or device-tested. Rerun the manual workflow on `development`; then test rotating in both directions across `.`, MHz, blank slot and CLOSE, plus OK/BS and cancel behavior.


## Future layout reference: lsongdev/tinyradio (requested 2026-10-29)
- User wants the additional display layout shown near the bottom of https://github.com/lsongdev/tinyradio added to the project's layout TODO list. User jokingly notes they are a layout fan.
- The repository is an AM/FM/SSB SI4732 receiver firmware project for ATS Mini and GeekNest V5A. Inspect the specific screenshot/image near the bottom of the linked repository page and its corresponding implementation before planning a port.
- Treat this as a separate future layout option, not a replacement for the TEF6686 default layout or the previously requested GNBD miniradio-inspired layout. Preserve existing layouts and isolate the implementation where possible.
- Priority: after current VHF Spectrum scanner build and menu/input bugs are resolved. Confirm which exact bottom-of-page layout the user means if the repository page contains multiple candidates.


## VHF Spectrum scanner device-test findings and patch (2026-10-29)
- User confirmed on ATS Mini V4 that native VHF scanning completes and displays a graph for 88.00–108.00 MHz, and selecting a point tunes the chosen frequency on the main layout correctly.
- Encoder keypad hang is no longer observed in the user's current build.
- User requested: (1) Spectrum submenu must appear on the first click, with VHF Spectrum and ALL Band Spectrum plus start-frequency guidance; (2) after entering start frequency, clearly explain that end frequency is next; (3) dark Web Control Spectrum style, subdued waveform and vivid red peak tips; (4) encoder moves from peak to peak rather than every 100 kHz.
- Patch adds start/end guidance to the scanner keypad, submenu rendering and a menu hint in both languages, and excludes CMD_SPECTRUM_MENU from the generic command timeout. It darkens the graph, marks local maxima in vivid red and changes encoder navigation to select local peaks, falling back to adjacent points if no peaks are detected.
- Source-only change; manual GitHub Actions build and device regression tests are required. Verify first-click submenu behavior, input-stage instructions, graph contrast, peak-to-peak selection, and existing Web Spectrum/Dual RX behavior.


## Follow-up fix after VHF scanner test (2026-10-29)
- User confirmed the scanner graph runs and clicking a frequency tunes the main layout correctly. The encoder keypad hang is gone.
- Native VHF scanner source already includes start/end-frequency instructions, dark graph styling, vivid red local-peak markers, and peak-to-peak encoder navigation with adjacent-point fallback.
- Found why the Spectrum submenu may disappear: the generic command timeout exempted the graph and keypad but not `CMD_SPECTRUM_MENU`. The timeout now also exempts the submenu so it stays open while the user reads/selects VHF Spectrum or ALL Band Spectrum.
- Menu hint is present in both English and German. The start/end keypad text is present.
- Next: manual build and device test. Verify first click on SPECTRUM opens and stays on the submenu; selecting VHF Spectrum opens the keypad; entering start frequency prompts for end frequency; graph colors/peak navigation; selecting a peak still tunes correctly. Source changes are not build-verified yet.


## VHF Spectrum UI polish before next manual build (2026-10-29)
- Commit `782a1f3985bd3ddcee04b1e262bcb7a3af7b63d2` adjusts the scanner keypad geometry so the instruction text is no longer drawn over the frequency field. The first stage explicitly says ENTER START FREQUENCY; the second stage shows the accepted start frequency and ENTER END FREQUENCY above the field.
- Native graph panel and grid were darkened toward the near-black navy Web Spectrum aesthetic; trace color is a very dark blue-gray and peak tips remain vivid red.
- Encoder peak navigation now uses a lower local-peak threshold (5% of the RSSI span) and, if the trace has no qualifying local peak, jumps to the strongest remaining sample in the requested direction instead of advancing by a single 100 kHz point.
- The earlier timeout exemption for `CMD_SPECTRUM_MENU` remains in `ats-mini.ino` (commit `29f2865d193b3ef3ffb2132607c069cc24964414`).
- Next step: manual build and device verification. These latest changes have not been built or tested on hardware yet.


## VHF Spectrum keypad click-dispatch regression (2026-10-29)
- User clarified that before the Spectrum submenu was introduced, selecting Spectrum opened the keypad immediately; confirming the start frequency opened the end-frequency keypad; confirming the end frequency ran the scan successfully. The current regression appeared after the submenu and follow-up UI changes.
- Root cause found in `ats-mini/ats-mini.ino`: a click while `currentCmd == CMD_SPECTRUM_INPUT` was first sent to the generic `clickHandler()`. Since that handler has no `CMD_SPECTRUM_INPUT` case, it returned false and the generic fallback changed `currentCmd` to `CMD_NONE` before the dedicated `spectrumScannerInputClick(true)` block could run. This breaks keypad confirmation/state progression and can make the keypad appear frozen or exit unexpectedly.
- Targeted fix committed on `development` as `b37791abdb8b28d9ab83d24df9493fac8f182e56`: exclude `CMD_SPECTRUM_INPUT` clicks from the generic click handler so the existing dedicated scanner input handler receives them. No Spectrum scanning algorithm, menu source, layout source, or Dual RX code was changed.
- Verification: source-level root cause confirmed and targeted edit committed. Manual build and hardware test are still required; do not claim compile/device success yet. Test menu -> VHF Spectrum -> enter/confirm start frequency -> enter/confirm end frequency -> scan; also verify Back/Close and that normal radio keypad and Dual RX still behave as before.
