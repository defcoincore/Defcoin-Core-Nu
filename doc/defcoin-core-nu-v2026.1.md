# Defcoin Core Nu v2026.1c

This branch starts from Litecoin Core `v0.21.5.5` and applies the standard
Defcoin Core `2026.1` port as the engine baseline.

The Nu work is staged:

1. Preserve Defcoin node and wallet behavior from the standard build.
2. Add a QML / Qt Quick app beside the current Qt Widgets UI.
3. Keep the Nu app behind a local service/RPC boundary while the interface is
   reviewed.
4. Later harden the GUI/node split into fully separate processes following the Bitcoin
   Core App architecture direction.

## Current State

The source tree now contains:

- Defcoin chain parameters, ports, seeds, address prefixes, user agent, display
  units, and packaging identity from the standard `2026.1` build.
- Current Qt Widgets UI from the standard build.
- Nu QML implementation under `src/qt/nu/`, plus a standalone review app under
  `src/qt/nu/app/`.

The Nu app is not wired into the current Qt Widgets wallet build. It uses
`NuRpcService` as a local JSON-RPC client to a running Defcoin backend, and the
standalone app can bundle/start `defcoind` for local development. This keeps
key handling, signing, validation, and peer networking outside QML while
allowing the new interface to be reviewed as a functional client.

## 2026.1c Release Boundary

Nu 2026.1c is the no-help-manual release track. It keeps the app small while
the fuller 2026.2 help manual is parked for review and likely folded into a
later 2026.3 help-enabled release.

The app still includes an About summary and a Build Notes action. Build Notes
cover what went into the Nu interface and briefly explain how Nu differs from
Defcoin 1.0.0 and 1.0.1:

- Nu uses a Qt Quick shell instead of the older Qt Widgets wallet UI.
- Nu can autostart and monitor the bundled `defcoind` backend.
- Nu keeps wallet keys, signing, validation, and peer logic in the backend.
- Nu adds clearer startup diagnostics, peer inspection, and network-migration
  controls.
- Nu remains on the same Defcoin chain, wallet format, data directory, and
  backend lineage as the 2026.1 engine baseline.

The deferred help-manual work is tracked in
`src/qt/nu/docs/roadmap-2026.3-help-manual.md`.

## Design Direction

Nu follows three constraints:

- Bitcoin Core App architecture: QML / Qt Quick, modular UI, eventual separate
  GUI process.
- Bitcoin Core App principles: neutral visuals, safety first, transparent node
  activity.
- Nothing-inspired composition: restrained monochrome structure, larger legible
  typography, data-first hierarchy, and minimal decorative chrome.
- Bitcoin Design Guide fallbacks: wallet flows must stay understandable,
  accessible, safe around irreversible actions, and organized around real user
  tasks.
- Defcoin standard behavior: core wallet, node, chain, and signing behavior stay
  unchanged unless explicitly changed in a later engine task.

The current Nu font target is still system-native (`Helvetica Neue`) so the
standalone app has no web-font dependency. If the project later adopts the full
Nothing typography stack, declare and bundle the chosen families first; do not
assume remote font loading is available inside the wallet.

## Engine Boundary

The Defcoin engine remains authoritative for:

- private keys;
- wallet database;
- transaction creation and signing;
- validation and chain sync;
- peer networking;
- fee policy;
- mempool and relay;
- RPC execution.

The Nu UI currently targets a local JSON-RPC service boundary. A later in-tree
service can be shaped from the existing `interfaces::Node` and
`interfaces::Wallet` APIs after the UI behavior stabilizes.

## Scaffold Entry Points

```text
src/qt/nu/qml/Main.qml
src/qt/nu/qml/Shell/AppFrame.qml
src/qt/nu/qml/Theme/Tokens.qml
src/qt/nu/qml/Components/
src/qt/nu/qml/Views/
src/qt/nu/assets/
src/qt/nu/docs/
src/qt/nu/app/
```

## Safety Rule

QML files must not duplicate wallet logic, signing logic, validation logic, or
networking logic. Visible controls should call backend service methods, be
disabled, or be omitted.
