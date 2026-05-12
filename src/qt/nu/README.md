# Defcoin Core Nu UI

This folder contains the inactive QML / Qt Quick scaffold for Defcoin Core Nu
v2026.1.

The files here are design and bridge scaffolding only. They are deliberately
not connected to the current Qt Widgets application, so the working standard
wallet behavior is unchanged.

## Intended Runtime Shape

```text
defcoind / wallet service process
    |
    | typed local IPC client
    v
Defcoin Core Nu QML process
```

The current fake service in `qml/Bridge/NuFakeService.qml` exists so the shell,
views, tables, status strips, and graph components can be built and reviewed
without loading a wallet or blocking the UI thread.

## Build Status

Not wired into `src/Makefile.qt.include` yet.

The resource manifest at `resources/nu.qrc` is present for the future feature
flag build, but should stay inert until the bridge is implemented.

## Design Rules

- keep irreversible wallet actions review-first;
- never put signing, key, chain, or peer logic in QML;
- use neutral visual tokens from `qml/Theme/Tokens.qml`;
- expose node activity clearly without making status indicators look like
  destructive controls;
- defer heavy tab data until after the first visible view renders.
