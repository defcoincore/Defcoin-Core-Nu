# Nu Tools

This folder is reserved for future non-consensus development tools for the
QML Nu interface.

Current suggested tools:

- QML visual smoke runner.
- Screenshot comparison harness.
- IPC fake service for UI development.
- Defcoin engine delta verifier.

## First Tool To Add

The first practical tool should launch `src/qt/nu/qml/Main.qml` against the
fake service and capture screenshots for default, narrow, and large-text
layouts. That keeps Nu UI iteration independent from the current wallet build.
