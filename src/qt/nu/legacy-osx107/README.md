# Defcoin Core Nu Legacy OS X 10.7 Build

This directory is intentionally separate from the Qt 6 / QML release app.

Target:

- Mac OS X 10.7.5
- Xcode 4.6.3
- Qt 5.5.1 `qtbase`, patched for MacPorts OpenSSL3
- qmake, not the CMake 3.24+ Qt 6 build

Reason:

- The current Nu app uses Qt 6 and Qt Quick Controls 2.
- Qt 6.10.1 binaries used by the current release have a macOS 13.0 minimum.
- Qt 5.5 is the last Qt line that still fits OS X 10.7.

Build sketch on the legacy Mac:

```sh
QMAKE="$HOME/defcoin-core-nu-build-YYYYMMDD_HHMMSS/tools/qt-5.5.1-openssl3test/bin/qmake"
cd "$HOME/defcoin-core-nu-build-YYYYMMDD_HHMMSS/src/src/qt/nu/legacy-osx107"
"$QMAKE" DefcoinCoreNuLegacy.pro -spec macx-clang CONFIG+=release
make -j2
```

The Qt 5.5.1 source must be patched with
`depends/patches/qt5.5.1/openssl3-lion-compat.patch` before building against
MacPorts `openssl3`.

The initial legacy shell is a Qt Widgets launcher/status surface. It is not a
port of the full Qt Quick Controls 2 application.

Package the app and daemon into a redistributable HFS+ DMG after copying the
Lion-built outputs and runtime dylibs to a staging directory on the packaging
Mac:

```sh
./contrib/legacy-osx107/package_legacy_dmg.sh \
  --app "/path/to/osx107-stage/DefcoinCoreNuLegacy.app" \
  --qt-prefix "/path/to/osx107-stage/qt-5.5.1-openssl3test" \
  --defcoind "/path/to/osx107-stage/defcoind" \
  --openssl-prefix "/path/to/osx107-stage/openssl3" \
  --macports-prefix "/path/to/osx107-stage/macports" \
  --ca-bundle "/path/to/osx107-stage/curl-ca-bundle.crt" \
  --output-dir "/path/to/osx107-dmg" \
  --version "26.3.1-lion"
```

The package script writes `Contents/Resources/qt.conf` and the launcher pins
`QT_PLUGIN_PATH` to the bundled `Contents/PlugIns` directory. Both are needed on
the Lion host because Qt 5.5 otherwise falls back to the original Qt build
prefix and can load a second copy of Qt when resolving the Cocoa platform
plugin.
