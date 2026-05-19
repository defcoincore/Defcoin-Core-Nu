# Roadmap TODO: Help Manual For Next Help-Enabled Nu Release

Target release: a later `26.x` Nu release.

Status: Nu 26.3.0 ships without the bundled Help manual while keeping a compact About build-notes dialog.

## Held Manual Inventory

The held help bundle currently contains:

- `index.html`: main Defcoin Core Nu Help Manual.
- `details.html`: detailed About, architecture, history, copyright, and reference page.
- `DefcoinCoreNu.hhp`, `DefcoinCoreNu.hhc`, `DefcoinCoreNu.hhk`: Windows help project, contents, and index metadata.
- macOS Apple Help bundle metadata under `DefcoinCoreNu.help/Contents/Info.plist`.

## Manual Sections To Review Before Releasing

- Home (Overview)
- Cryptocurrency basics
- Status and sync indicators
- Send
- Partially signed transactions
- Wallet safety
- Receive
- Activity (Transactions)
- Diagnostics
- Settings
- Unsupported or compatibility-limited features
- Glossary
- Known issues and review notes
- Sources
- Detailed About
- Copyright
- Architecture
- Litecoin Core features not active in Defcoin
- Frontend direction
- Defcoin history
- Current Defcoin sites and communities as of May 2026
- References

## Required Cleanup Before Re-Enabling Help

- Rewrite remaining manual sections as polished user documentation, not implementation prompts.
- Keep Windows and macOS help content identical where platform differences do not matter.
- Verify all table-of-contents links and internal anchors.
- Keep links visible with full URLs or clear destination hover text.
- Re-test Windows in-app help routing so it opens bundled HTML, never QML source.
- Re-test macOS native Apple Help registration and search index generation.
- Confirm PSBT content appears in both Windows and macOS help.
- Keep About details in one place; avoid duplicating the same long-form text in both About and Help.

## 26.3.0 Boundary

Nu 26.3.0 intentionally does not include the full Help manual. It should include:

- A minimal About summary.
- A Build Notes / Details action.
- Short Nu build notes.
- A limited bullet list explaining how Nu differs from Defcoin 1.0.0 and 1.0.1.
- No empty Help menu item.
- No Help Manual entry in the menu until the manual is ready.
