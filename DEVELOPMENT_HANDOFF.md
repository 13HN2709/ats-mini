# ATS Mini Development Handoff

Last updated: 2026-10-28 (native scan engine and sample buffer isolated; pre-build review still pending)
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
- Native scanner tuning now finds an FM band that covers the requested range, configures FM before the sweep, and restores the previously selected band/frequency/mode/BFO after the scan. Accepting a graph point explicitly switches to the FM band covering that frequency. Commits: `827091d16d4f46625f5ad989e7a3ebb3bccc5305`, `42f34a90788a0c6f5d971ad76174f29323873381`, `0cae2833afec6acf20bb44e3740996af079a11b1`, `d7b79986eeab993828456faf633750e4769cf07b`.
- Native VHF Spectrum now has its own 201-point RSSI/SNR sample buffer and isolated sweep loop in `SpectrumScanner.cpp`; it no longer uses or overwrites the Web Spectrum's `Scan.cpp` sample buffer. This avoids native/web spectrum data interference. Commits: `ff0ed62319b51e75ccf2e509808491abd9216a2a`, `5e55a3ae099c768afdd9b153ab547a0d9c6a3fe3`, `dca1574dd612a8f95cd402ade25287dad77bb0a4`.

## Current repository state observed on 2026-10-28
- Branch: `development`.
- `ats-mini/SpectrumScanner.cpp/.h` now contain scanner-owned input state and a dedicated numeric keypad for start/end frequencies; the existing `FreqKeypad.cpp/.h` is not modified.
- Native scan sample storage/sweep is now isolated from Web Spectrum; only the physical SI4735 receiver is necessarily shared.
- `CMD_SPECTRUM_INPUT` is a separate command. `Draw.cpp` renders its own input screen and `ats-mini.ino` dispatches its own encoder input, outside the Dual RX block.
- The VHF input accepts 88.00–108.00 MHz and rejects an end frequency that is not greater than the start. The scan graph no longer tunes on encoder rotation; selection tunes only when accepted.
- `Menu_german.cpp.src` localizes the Spectrum submenu. The all-band placeholder is guarded so it does not start VHF scan.
- `Network.cpp` ignores pending web spectrum commands while native spectrum input is active.
- This is still unbuilt/unverified. Review compile-sensitive changes and scanner/scan-engine side effects before telling the user to launch the build.
- `Menu.h` defines `CMD_SPECTRUM` and `CMD_SPECTRUM_MENU`.
- English/German menu templates include Spectrum and submenu entries; the click handler currently starts the VHF scanner only for submenu index 0, so index 1 must be handled safely.
- `Draw.cpp` routes `CMD_SPECTRUM_MENU` to the full-screen menu and `CMD_SPECTRUM` to `drawSpectrumScanner()`.
- `ats-mini.ino` has encoder handling for `CMD_SPECTRUM`; review it before adding a dedicated input command. Avoid modifying the Dual RX block.
- `FreqKeypad.cpp/.h` implements the existing radio/Dual RX keypad and must remain unchanged for this feature.
- `Scan.cpp` is the shared low-level sweep implementation. It stores scan results globally for the web spectrum too, so check the implications of starting a native scan before reuse.
- GitHub Actions workflow was changed earlier to `ubuntu-24.04` for both jobs after an Ubuntu runner/Python issue. Do not revert without a reason.

## Immediate next steps
1. Finish reviewing encoder/click dispatch, native sweep timing, restoration, and graph navigation.
2. Check the manual build workflow and generated source/template consistency.
3. Ensure `ALL Band Spectrum` cannot accidentally start VHF scan.
4. Update this handoff after each meaningful change.
5. Only then tell the user the code is ready for the manual build. Report exact files/commit and explicitly state that compilation/device testing remain pending until performed.

## Validation checklist (must be checked before asking user to build)
- [ ] VHF start and end are entered in separate scanner-owned stages.
- [ ] Decimal point and/or documented frequency format works correctly.
- [ ] Range and ordering validation reject invalid values.
- [ ] Cancel/error does not inadvertently retune the radio.
- [ ] Encoder navigation and short-click selection work.
- [ ] Long press behavior is defined and does not leak into Dual RX behavior.
- [ ] ALL Band submenu item cannot accidentally start VHF scanning.
- [ ] Existing Dual RX source/behavior is unchanged.
- [ ] Web Spectrum remains functional; native scanner no longer shares its sample buffer.
- [ ] Native sweep uses an FM-configured band and restores the previous radio state after scanning.
- [ ] German and English menu labels are appropriate.
- [ ] VHF keypad visually matches `drawFreqKeypad()` geometry/colors/fonts.
- [ ] VHF keypad exposes only fixed FM and MHz; no mode or kHz switching.
- [ ] Future ALL Band keypad is separate and exposes USB/LSB/AM only with kHz input.
- [ ] Build workflow remains manual.
- [ ] Handoff updated with actual changes, commit, and remaining risks.

## Changes made in this session
- Added `DEVELOPMENT_HANDOFF.md` on `development` (initial handoff commit `bee78177301f4d2be6759f40f5de46ed0c55522f`).
- Added `CMD_SPECTRUM_INPUT` and scanner-specific keypad API/state.
- Added independent VHF start/end frequency entry and basic range validation in `SpectrumScanner.cpp`.
- Added keypad drawing dispatch in `Draw.cpp`; added encoder input dispatch in `ats-mini.ino` without changing the Dual RX block itself.
- Updated English/German menu click handling so only VHF item starts scanner; German submenu localized.
- Updated `Network.cpp` to ignore web-spectrum command processing during native scanner input.
- Additional commits: `b1b657d3355526920328069a31735eeab1babfa3` (corrected label line breaks), `1c6861a7427736b7670afbe82a870d7892e174f1` (web-spectrum command isolation), `7d5d86e1a54f7db04ad28709a180c4ec0fac5c5d` (consume menu-opening click so it cannot enter a keypad digit). The handoff itself was updated in `90effb849c89d8b4c867808c5aaf6e13a572ebbd` before these final edits.
- Still required before user build: review current `ats-mini.ino` input click flow, verify the scan screen status/range display, and inspect shared scan buffer interactions. No compile or hardware test has been run.
