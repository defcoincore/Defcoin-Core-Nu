# Defcoin Core Nu Legacy OS X 10.7 Build

This directory is intentionally separate from the Qt 6 / QML release app.

Target:

- Mac OS X 10.7.5
- Xcode 4.6.3
- Qt 5.5.1 `qtbase`
- qmake, not the CMake 3.24+ Qt 6 build

Reason:

- The current Nu app uses Qt 6 and Qt Quick Controls 2.
- Qt 6.10.1 binaries used by the current release have a macOS 13.0 minimum.
- Qt 5.5 is the last Qt line that still fits OS X 10.7.

Build sketch on the legacy Mac:

```sh
QMAKE="$HOME/defcoin-core-nu-build-YYYYMMDD_HHMMSS/tools/qt-5.5.1/bin/qmake"
cd "$HOME/defcoin-core-nu-build-YYYYMMDD_HHMMSS/src/src/qt/nu/legacy-osx107"
"$QMAKE" DefcoinCoreNuLegacy.pro -spec macx-clang CONFIG+=release
make -j2
```

The initial legacy shell is a Qt Widgets launcher/status surface. It is not a
port of the full Qt Quick Controls 2 application.
