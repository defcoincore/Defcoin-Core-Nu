# Nu UI Scaffold

This directory contains non-active QML and design assets for the future
Defcoin Core Nu interface.

It is intentionally not connected to the current Qt Widgets build yet.

## Target

- Qt Quick / QML presentation.
- GUI/client process separated from node/wallet service process.
- Neutral professional visual style.
- Wallet actions routed through typed service/client APIs.
- Inert graphical scaffold that can be run against fake data before touching
  consensus, wallet, or networking code.

## Do Not Put Here

- consensus changes;
- private-key handling;
- wallet database access;
- transaction signing;
- direct peer-manager access;
- blocking network or filesystem work on the QML scene thread.

## Files

```text
qml/Main.qml              Nu shell entry point
qml/Bridge/               fake service now; typed IPC client later
qml/Components/           reusable QML controls
qml/Shell/                app frame, nav, and status strip
qml/Theme/Tokens.qml      neutral visual tokens
qml/Views/                wallet and node views
assets/icons/             Nu SVG icon system
resources/nu.qrc          inactive resource manifest
```
