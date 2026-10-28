# ATS Mini Development Handoff

Last updated: 2026-10-28 (VHF keypad pass committed; pre-build review still pending)
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
- Existing scan engine is in `Scan.cpp` and currently exposes `scanRunRange(uint16_t,uint16_t,uint16_t)`, `scanGetCount()`, and `scanGetPoint(uint16_t,uint16_t*,uint8_t*,uint8_t*)`. Inspect full implementation before relying on its memory/state/tuning side effects.
- `Network.cpp` should not be changed unnecessarily; existing web-spectrum functionality must remain intact.
- Scanner code must explicitly prevent the `ALL Band Spectrum` menu item from invoking the VHF scan until all-band support is implemented.

## Current repository state observed on 2026-10-28
- Branch: `development`.
- `ats-mini/SpectrumScanner.cpp/.h` now contain scanner-owned input state and a dedicated numeric keypad for start/end frequencies; the existing `FreqKeypad.cpp/.h` is not modified.
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
1. Inspect exact declarations and all `currentCmd` input/draw dispatch points.
2. Add a scanner-specific frequency-entry UI/state, separate from `FreqKeypad` and Dual RX.
3. Validate both frequencies before touching radio state or starting the sweep.
4. Add an explicit visible scan-start/status stage.
5. Ensure `ALL Band Spectrum` is clearly unavailable/placeholder or does not launch VHF scan.
6. Review cancellation, tuning side effects, and web scanner interference.
7. Update this handoff in the same change set as each meaningful implementation step.
8. Only then tell the user the code is ready for the manual build. Report exact files/commit and explicitly state that compilation/device testing remain pending until performed.

## Validation checklist (must be checked before asking user to build)
- [ ] VHF start and end are entered in separate scanner-owned stages.
- [ ] Decimal point and/or documented frequency format works correctly.
- [ ] Range and ordering validation reject invalid values.
- [ ] Cancel/error does not inadvertently retune the radio.
- [ ] Encoder navigation and short-click selection work.
- [ ] Long press behavior is defined and does not leak into Dual RX behavior.
- [ ] ALL Band submenu item cannot accidentally start VHF scanning.
- [ ] Existing Dual RX source/behavior is unchanged.
- [ ] Web Spectrum remains functional; shared scan buffer behavior understood.
- [ ] German and English menu labels are appropriate.
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
