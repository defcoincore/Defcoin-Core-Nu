# Defcoin Core Nu Build And Installer Runbook

Last updated: 2026-05-19

This runbook records the build paths and commands that have worked recently.
Use it as the canonical local procedure and revise it when the build system
changes.

## Canonical Paths

Use these variables in shell sessions:

```sh
NU_ROOT="/Volumes/TB5_4TB/d/litecoincore/Defcoin Core Nu"
SRC="$NU_ROOT/source/defcoin-core-nu-litecoin-fork"
OUT="$NU_ROOT/Distribution_Versions"
TOOLS_OUT="/Volumes/Tools/Defcoin-Windows-Builds"
```

Finished deliverables belong only under versioned release folders:

```text
$OUT/Nu-26.3.0/apple-silicon/
$OUT/Nu-26.3.0/mac-intel/
$OUT/Nu-26.3.0/windows11-x86_64/
```

Source-local build directories are transient CMake or backend build products.
They should not be treated as release artifacts.

Each QML resource build writes `BUILD_INFO.txt` and
`BUILD_INFO.properties` into the bundled `nu` resource directory. The visible
release remains `26.3.0`; the Build ID records the UTC build time and source
commit, for example `26.3.0+20260518-153000-14cb07953-dirty`. On macOS the
same metadata is also written to custom `DefcoinNu...` keys in the app
`Info.plist`.

Current transient build directories:

```text
$SRC/build/nu-qml-arm64
$SRC/build/nu-qml-intel
$SRC/build/nu-qml-win64-local
$NU_ROOT/source/defcoin-core-nu-win-backend-build-src
```

Avoid keeping older duplicate CMake directories unless they are actively being
used to debug a platform-specific build.

## Cleanup Before Building

Safe generated folders to remove when not actively debugging:

```sh
cd "$SRC"
rm -rf "Defcoin Core Nu.app"
rm -rf build/nu-qml-win64 build/nu-qml-x86_64 build/nu-qml-x86_64-local
rm -rf build-win64-backend-nu
```

Do not remove:

```text
$SRC/build-aux
$SRC/build_msvc
$SRC/depends/builders
```

Those are source-tree build support directories.

## macOS Apple Silicon App

Build the QML shell. Set `DEFCOIN_NU_RELEASE_NAME` and
`DEFCOIN_NU_ENABLE_HELP` for the target release. For example, `26.3.0` uses
`DEFCOIN_NU_ENABLE_HELP=OFF`; the future help-manual release should use its own
release name and `DEFCOIN_NU_ENABLE_HELP=ON`.

```sh
cd "$SRC"
cmake -S src/qt/nu/app -B build/nu-qml-arm64 \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_PREFIX_PATH="$NU_ROOT/toolchains/qt/6.10.1/macos" \
  -DQt6_DIR="$NU_ROOT/toolchains/qt/6.10.1/macos/lib/cmake/Qt6" \
  -DDEFCOIN_NU_BACKEND_BINARY="$SRC/src/defcoind" \
  -DDEFCOIN_NU_RELEASE_NAME="26.3.0" \
  -DDEFCOIN_NU_ENABLE_HELP=OFF
cmake --build build/nu-qml-arm64 --target DefcoinCoreNuResources -- -j1
```

The CMake app target runs Qt's `macdeployqt` for macOS bundles. Do not manually
copy Qt frameworks into `Contents/Frameworks`; manual directory copies can
dereference framework symlinks and produce ambiguous framework bundles that
cannot pass strict `codesign`.

Stage the app and DMG. This helper removes the previous staged app and DMG,
copies the newly built bundle, signs it for local testing, creates the DMG, and
sets Finder creation and modification dates on the staged platform folder, app,
DMG, and `BUILD_STAGED_AT.txt` stamp file. Replacing the bundle instead of
overwriting it prevents stale Finder creation dates from making an old build
look current.

```sh
src/qt/nu/app/stage_macos_distribution.sh \
  "$SRC/build/nu-qml-arm64/DefcoinCoreNu.app" \
  "$OUT/Nu-26.3.0/apple-silicon" \
  "26.3.0" \
  "macOS-AppleSilicon"
```

If strict verification reports an ambiguous Qt framework bundle, the release
bundle is not acceptable. Rebuild the app target so `macdeployqt` recreates the
framework layout, restage the app, clear extended attributes, and sign again.

For Developer ID or Mac App Store distribution, install a valid Apple signing
identity first. This machine must show a valid result from:

```sh
security find-identity -v -p codesigning
```

Ad-hoc signing is enough to verify the local bundle layout, but do not add
`--options runtime` when signing ad-hoc. Hardened runtime library validation
requires a real Apple Team ID or matching entitlements; with ad-hoc signatures
it can reject bundled Qt frameworks at launch. Ad-hoc signing is not a
substitute for Developer ID signing, notarization, or App Store signing.

## macOS Intel App

The local Qt 6.10.1 macOS toolchain under `$NU_ROOT/toolchains/qt/6.10.1/macos`
is universal and contains both `arm64` and `x86_64` slices. Use that Qt tree for
Intel builds so CMake does not accidentally link against Homebrew's arm64-only
Qt frameworks.

```sh
QT_MAC="$NU_ROOT/toolchains/qt/6.10.1/macos"
cd "$SRC"
cmake -S src/qt/nu/app -B build/nu-qml-intel \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES=x86_64 \
  -DCMAKE_PREFIX_PATH="$QT_MAC" \
  -DQt6_DIR="$QT_MAC/lib/cmake/Qt6" \
  -DCMAKE_TOOLCHAIN_FILE="$QT_MAC/lib/cmake/Qt6/qt.toolchain.cmake" \
  -DDEFCOIN_NU_BACKEND_BINARY="/tmp/defcoin-core-nu-mac-intel-src/src/defcoind" \
  -DDEFCOIN_NU_RELEASE_NAME="26.3.0" \
  -DDEFCOIN_NU_ENABLE_HELP=OFF \
  -DQRENCODE_LIBRARY="$NU_ROOT/toolchains/qrencode/macos-x86_64/lib/libqrencode.a" \
  -DQRENCODE_INCLUDE_DIR="$NU_ROOT/toolchains/qrencode/macos-x86_64/include"
cmake --build build/nu-qml-intel --target DefcoinCoreNuResources -j8
rm -rf "$OUT/Nu-26.3.0/mac-intel/Defcoin Core Nu.app"
ditto "$SRC/build/nu-qml-intel/DefcoinCoreNu.app" \
  "$OUT/Nu-26.3.0/mac-intel/Defcoin Core Nu.app"
touch -ch "$OUT/Nu-26.3.0/mac-intel/Defcoin Core Nu.app"
xattr -cr "$OUT/Nu-26.3.0/mac-intel/Defcoin Core Nu.app"
codesign --force --deep --sign - "$OUT/Nu-26.3.0/mac-intel/Defcoin Core Nu.app"
codesign --verify --deep --strict --verbose=4 "$OUT/Nu-26.3.0/mac-intel/Defcoin Core Nu.app"
```

## macOS DMG

Use the app staged in the matching `$OUT/Nu-<version>/...` platform folder.

Create or refresh the DMG background before packaging, then build:

```sh
hdiutil create -volname "Defcoin Core Nu" \
  -srcfolder "$OUT/Nu-26.3.0/apple-silicon/Defcoin Core Nu.app" \
  -ov -format UDZO \
  "$OUT/Nu-26.3.0/apple-silicon/Defcoin-Core-Nu-v26.3.0-macOS-AppleSilicon.dmg"
hdiutil verify "$OUT/Nu-26.3.0/apple-silicon/Defcoin-Core-Nu-v26.3.0-macOS-AppleSilicon.dmg"
touch -ch "$OUT/Nu-26.3.0/apple-silicon/Defcoin-Core-Nu-v26.3.0-macOS-AppleSilicon.dmg"
```

If a custom Finder background is required, use the project DMG script or create
a temporary writable DMG, copy the background into a hidden `.background`
folder, set Finder view options through AppleScript, then convert to `UDZO`.

## Windows Backend

Use a clean backend source copy so Apple Silicon object files cannot contaminate
the Windows backend.

```sh
WIN_SRC="$NU_ROOT/source/defcoin-core-nu-win-backend-build-src"
rm -rf "$WIN_SRC"
rsync -a --delete \
  --exclude '.git' \
  --exclude '/build/' \
  --exclude '/build-win64-backend-nu/' \
  --exclude '/Defcoin Core Nu.app/' \
  --exclude '*.o' \
  --exclude '*.a' \
  --exclude '*.la' \
  --exclude '*.lo' \
  --exclude '.libs' \
  "$SRC/" "$WIN_SRC/"
```

Do not exclude `build-aux`; the autotools macros are required by `autogen.sh`.
If a copied source tree already contains stale UniValue libtool artifacts, clear
them before the final link:

```sh
rm -f src/univalue/libunivalue.la src/univalue/lib/*.lo src/univalue/lib/*.o
rm -rf src/univalue/.libs
```

Configure and build:

```sh
cd "$WIN_SRC"
./autogen.sh
CONFIG_SITE="/Volumes/TB5_4TB/d/litecoincore/defcoin-core-legacy/depends/x86_64-w64-mingw32/share/config.site" \
  ./configure --prefix=/ --host=x86_64-w64-mingw32 --without-gui --enable-wallet --with-sqlite=yes \
  --with-miniupnpc --disable-zmq --disable-tests --disable-bench \
  --disable-shared --with-pic
make -j1 src/defcoind.exe
```

The current Nu source tree does not include a populated Windows depends prefix;
the recent successful build used the existing legacy Defcoin Windows depends
prefix above.

If Homebrew MinGW emits duplicate `__stack_chk_fail` symbols, prefer the
project toolchain setting that disables duplicate SSP injection rather than
blindly adding both static and dynamic SSP libraries.

## Windows QML App

```sh
cd "$SRC"
cmake -S src/qt/nu/app -B build/nu-qml-win64-local \
  -DCMAKE_TOOLCHAIN_FILE="$SRC/depends/cmake/mingw-w64-x86_64.cmake" \
  -DDEFCOIN_NU_BACKEND_BINARY="$NU_ROOT/source/defcoin-core-nu-win-backend-build-src/src/defcoind.exe" \
  -DDEFCOIN_NU_RELEASE_NAME="26.3.0" \
  -DDEFCOIN_NU_ENABLE_HELP=OFF \
  -DQt6_DIR="$NU_ROOT/toolchains/qt/6.10.1/mingw_64/lib/cmake/Qt6" \
  -DQRENCODE_LIBRARY="$NU_ROOT/toolchains/qrencode/win64/lib/libqrencode.a" \
  -DQRENCODE_INCLUDE_DIR="$NU_ROOT/toolchains/qrencode/win64/include" \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build/nu-qml-win64-local --target DefcoinCoreNuResources -j8
```

The `DefcoinCoreNuResources` CMake target builds the app and deploys the current
QML, Qt runtime DLLs, Qt plugins, QML imports, bundled backend, help files, and a
`qt.conf` beside `DefcoinCoreNu.exe`. Verify these exist before packaging:

```sh
test -f "$SRC/build/nu-qml-win64-local/Qt6Core.dll"
test -f "$SRC/build/nu-qml-win64-local/plugins/platforms/qwindows.dll"
test -d "$SRC/build/nu-qml-win64-local/qml/QtQuick"
test -f "$SRC/build/nu-qml-win64-local/nu/bin/defcoind.exe"
```

Stage the Windows app folder under:

```text
$OUT/Nu-26.3.0/windows11-x86_64/Defcoin Core Nu/
```

Then run the NSIS script:

```sh
makensis "$OUT/Nu-26.3.0/windows11-x86_64/defcoin-core-nu-installer.nsi"
touch -ch "$OUT/Nu-26.3.0/windows11-x86_64/Defcoin-Core-Nu-v26.3.0-Windows11-x86_64-Setup.exe"
```

Copy each Windows build to Tools using this naming pattern:

```sh
STAMP="$(date +%Y%m%d_%H%M)"
DEST="$TOOLS_OUT/windows11-x86x64_nu-$STAMP"
mkdir -p "$DEST"
ditto "$OUT/Nu-26.3.0/windows11-x86_64" "$DEST"
```

2026-05-17 refreshed Windows copies before the 26.3.0 renumbering:

```text
/Volumes/Tools/Defcoin-Windows-Builds/windows11-x86x64_nu-20260517_0659
/Volumes/Tools/Defcoin-Windows-Builds/windows11-x86x64_nu-2026.1b-20260517_0705
```

Both copies were verified by matching the SHA-256 hash of the setup executable
against the local `Distribution_Versions` source.

## Current Magic Migration Notes

Dual magic mode is a startup setting. In dual mode the backend accepts both
legacy `fbc0b6db` and new `defc014e` packet headers. Most outbound handshakes
start with the new Defcoin bytes so upgraded nodes converge on `defc014e`, but
bounded legacy probes remain enabled so old-only Defcoin peers are still reachable. With
dual mode off, the backend uses only the new Defcoin magic. Nu-launched backends
also pass the curated Defcoin seed hosts as `-seednode` entries so peer
discovery starts immediately instead of waiting for stale `peers.dat` coverage.

## Peer Table Display Standard

Use the same peer vocabulary across Nu builds and the DC903 explorer where the
underlying data is available: `Node Id.`, `Dir.`, `IP Address`, `Port`, `Ping`,
`Sent`, `Rec'd`, `User Agent`, `Magic`, `Version`, and `Svcs`.

Display the actual peer packet magic reported by the backend. Do not infer magic
from user-agent text, because 2026-era nodes can speak either legacy
`fbc0b6db` or new `defc014e` depending on migration settings and peer direction.

IPv4 addresses should use fixed-width octet padding so the dots align. IPv6
addresses should remain in their compressed display form, sort as numeric IPv6,
and fit without dropping brackets, colons, or port information. When address and
port are separate fields, size the IP column for the address only and keep Port
as a separate right-aligned numeric column.

## Verification Checklist

- App icon appears in Finder, Dock, Command-Tab, Windows taskbar, and installer.
- Splash and About show the same summary copyright block.
- About Details opens a separate Help window.
- Tab focus moves through nav buttons and action buttons.
- Space and Enter activate focused buttons.
- Home, Send, Receive, Activity, Diagnostics, and Settings routes open.
- Diagnostics Log and Console text are readable and scrollable.
- Table columns sort and resize without visible first-open snapping.
- Windows installer lands in `/Volumes/Tools/Defcoin-Windows-Builds/` with the
  required date-time folder name.
