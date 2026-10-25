# ATS Mini V4 – Project Status

Repository: https://github.com/13HN2709/ats-mini

This file is the handover note for continuing work across ChatGPT conversations. Update it whenever a meaningful project milestone is reached. Verify branch heads and build/test status against GitHub before acting; this document can become stale.

## Branch strategy

- `main`: stable, approved release baseline and release backup source.
- `development`: the only ongoing development and test branch.
- `fullscreen-menu`: temporary branch for the current XXL full-screen menu work. After the menu is built and tested, promote the verified changes to `development`, then to `main`; verify both branches before deleting `fullscreen-menu`.
- Keep the number of branches low. Do not change `main` before the user approves the tested changes.

## Last repository state checked

Checked on 2026-10-25.

- `main`: `99b0cd32a5c498dbe9335eeba1b3a317f6f808d2`
  - Commit message: “Promote tested development firmware to main”
- `development`: `b2ddf13609c2035d64cf14e7590ad8a0f9e8104b`
- `fullscreen-menu`: `4dfbbbde53e69e2e7aee298b60cca9e0d544bddd`

### Current task: XXL full-screen menu

Work is in progress on `fullscreen-menu`.

Latest known menu commits:
- German menu: `5bc0c0d88a27332a4d7e38f7992f54a19cac2ca7`
- English menu: `4dfbbbde53e69e2e7aee298b60cca9e0d544bddd`

Files:
- [German menu source](https://github.com/13HN2709/ats-mini/blob/fullscreen-menu/ats-mini/Menu_german.cpp.src)
- [English menu source](https://github.com/13HN2709/ats-mini/blob/fullscreen-menu/ats-mini/Menu_english.cpp.src)

The recent work covers the XXL menu appearance in the TEF6686-inspired dark-blue style, menu icons, localized descriptions, and handling of German umlauts. The latest known umlaut approach draws dots above base letters because the selected font did not reliably provide Ä/Ö/Ü glyphs. This still needs confirmation on the physical display.

**Build status:** the latest menu changes have not yet been confirmed by a new build. Do not call them built or tested until a real build and the user's ATS Mini V4 hardware test have succeeded.

## Next steps

1. User starts a GitHub Actions build when back at work.
2. Review the build result and fix only issues supported by the logs.
3. User tests the menu on the ATS Mini V4:
   - Ä, Ö, Ü and ä, ö, ü render correctly.
   - Text size, spacing, icons and readability fit the 320×170 display.
   - Dark-blue TEF6686-style palette looks right.
   - German and English menu labels/descriptions are appropriate.
4. After approval, merge/promote the tested menu to `development`.
5. Promote the verified `development` state to `main`.
6. Create the release build and save a source-code ZIP backup from `main`.
7. Verify the changes are present in both destination branches and that the backup exists; then delete `fullscreen-menu`.
8. From then on, do all new development and tests in `development`; keep `main` stable.

## Permanent development rules

1. Protect existing working features. Do not refactor or rewrite stable code without a clear need.
2. For new features, prefer separate `.cpp` and `.h` modules where technically practical.
3. Reuse existing interfaces and avoid duplicate logic.
4. Do not unnecessarily modify `Layout-TEF6686.cpp`; the TEF6686 layout is the user's favorite. Keep the Clean layout separate for development.
5. Make small, focused changes and inspect the relevant current files before editing.
6. Report build and hardware-test status honestly. Never claim a build or device test passed unless it actually did.
7. Test on the user's hardware after the build. Only promote to `main` after the user approves.
8. Keep useful backups and verify each merge/promotion before deleting temporary branches.

## Design and project preferences

- Device: ATS Mini V4, 320×170 display.
- Preferred style: TEF6686-inspired dark navy panels, blue borders, cyan/neon accents, red vector icons where appropriate, and readable digital-style typography.
- Keep German text readable and handle Ä/Ö/Ü/ä/ö/ü correctly. Provide concise German explanations in the German menu and English explanations in the English menu where appropriate.
- Avoid unnecessary changes to the existing TEF6686 layout.
- The user prefers as few Git branches as practical.

## Related firmware features to protect

Existing areas that have previously been implemented or tested include the web control UI, Spectrum Scanner, Dual RX, memory selection/saving, receiver tuning and frequency entry, and multiple display layouts. Before changing any of these, check their current code and latest test status instead of assuming every older note still matches the repository.

## New-chat handover prompt

Paste this into a new ChatGPT conversation:

> We are continuing the ATS Mini V4 firmware project in https://github.com/13HN2709/ats-mini. First retrieve any available prior project context, then read PROJECT_STATUS.md on main and verify the current GitHub branch heads and relevant commits. Continue from the latest confirmed state, not from guesses.
>
> Follow the permanent development rules in PROJECT_STATUS.md: preserve working features, put new functionality in separate .cpp/.h modules where practical, make small focused changes, do not claim unperformed builds/tests, and do not promote to main without approval.
>
> Current focus is finishing and testing the XXL full-screen menu on fullscreen-menu. The next step is a GitHub Actions build and then a physical ATS Mini V4 test. Once approved, promote the verified state to development and then main, create a release build and source ZIP backup from main, verify everything, and delete fullscreen-menu. Future development and testing should happen only in development.

---
Last updated: 2026-10-25. Update this file when the branch, build, hardware test, or task status changes.
