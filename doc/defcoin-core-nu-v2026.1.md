# Defcoin Core Nu v2026.1

This branch starts from Litecoin Core `v0.21.5.5` and applies the standard
Defcoin Core `2026.1` port as the engine baseline.

The Nu work is intentionally staged:

1. Preserve Defcoin node and wallet behavior from the standard build.
2. Add a QML / Qt Quick scaffold beside the current Qt Widgets UI.
3. Keep the scaffold inactive until the client/service bridge is implemented.
4. Later split the GUI and node into separate processes following the Bitcoin
   Core App architecture direction.

## Current State

The source tree now contains:

- Defcoin chain parameters, ports, seeds, address prefixes, user agent, display
  units, and packaging identity from the standard `2026.1` build.
- Current Qt Widgets UI from the standard build.
- Non-active Nu QML scaffold under `src/qt/nu/`.

The Nu scaffold is not wired into the current build system yet. This avoids
breaking the working wallet while the presentation model is designed.

## Design Direction

Nu follows three constraints:

- Bitcoin Core App architecture: QML / Qt Quick, modular UI, eventual separate
  GUI process.
- Bitcoin Core App principles: neutral visuals, safety first, transparent node
  activity.
- Defcoin standard behavior: core wallet, node, chain, and signing behavior stay
  unchanged unless explicitly changed in a later engine task.

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

The Nu UI will target a service/client boundary shaped from the existing
`interfaces::Node` and `interfaces::Wallet` APIs.

## Scaffold Entry Points

```text
src/qt/nu/qml/Main.qml
src/qt/nu/qml/Shell/AppFrame.qml
src/qt/nu/qml/Theme/Tokens.qml
src/qt/nu/qml/Components/
src/qt/nu/qml/Views/
src/qt/nu/assets/
src/qt/nu/docs/
```

## Safety Rule

Until the Nu bridge exists, QML files must remain inert design scaffolding.
They should not duplicate wallet logic, signing logic, validation logic, or
networking logic.
