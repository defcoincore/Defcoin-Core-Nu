# Defcoin Core Nu 26.3.0 GitHub Notes

Last updated: 2026-05-19

Defcoin Core Nu 26.3.0, codename `Core Memories`, is a Defcoin wallet and full
node release derived from Litecoin Core v0.21.5.5. It keeps the inherited
Litecoin Core backend architecture where it remains appropriate for Defcoin,
while adding a Qt Quick desktop shell, bundled backend launch management, and
Defcoin-specific peer hygiene controls.

## License

The source code is released under the MIT license inherited from Litecoin Core
and Bitcoin Core. Keep new Defcoin Core Nu code under the same MIT license
unless a file explicitly says otherwise.

The code license does not grant trademark rights or artwork rights. Defcoin Core
Nu includes Defcoin coin imagery under the project's non-commercial permission
history, and the Def Con marks remain owned by Def Con Communications, Inc.

## Technical Summary

Defcoin Core Nu differs from Litecoin Core v0.21.5.5 in these main areas:

- Defcoin chain parameters, genesis blocks, checkpoints, seed hosts, ports,
  Base58 prefixes, display units, and application names.
- Defcoin mainnet remains compatible with the historical Defcoin chain and does
  not activate Litecoin mainnet features that Defcoin did not adopt.
- The peer user agent is Defcoin-specific: `/DefcoinCoreNu:26.3.0/`.
- The backend supports Defcoin-specific P2P magic `defc014e` and legacy
  compatibility magic `fbc0b6db` during migration.
- User-agent filtering prevents non-Defcoin-prefixed peers from feeding
  addresses into addrman.
- Address-relay filtering rejects unvalidated relayed mainnet endpoints that do
  not use Defcoin service ports before those records are counted as processed or
  stored in addrman. This is endpoint-specific rather than IP-wide: a host that
  runs Litecoin Core on one port and Defcoin Core on another is not rejected just
  because both services share an IP address. Preferred Defcoin endpoints can
  replace older same-IP non-Defcoin ports in addrman so Defcoin nodes are not
  crowded out by stale Litecoin-family records. A non-standard-port Defcoin node
  can still communicate and be retained after it completes an actual Defcoin
  handshake; it is not accepted merely because another peer gossiped its address.
- The Nu UI is Qt Quick and talks to a bundled `defcoind` child process over
  local RPC.

Defcoin Core Nu differs from Defcoin 1.0.0 and 1.0.1 in these main areas:

- A new Home, Send, Receive, Activity, Diagnostics, and Settings shell.
- Current-launch diagnostics for backend path resolution, RPC readiness,
  `debug.log`, peers, network traffic, and startup failures.
- Receive request history, removable receive requests, transaction detail
  actions, PSBT review/sign/finalize/broadcast tools, message signing, wallet
  backup, encryption, passphrase change, and optional explorer links.
- Dual-magic migration controls to reduce Litecoin-family network pollution
  while preserving compatibility with old Defcoin nodes during the transition.
- Peer hygiene refinements that combine Defcoin user-agent checks, preferred
  Defcoin service-port checks, and same-IP port replacement for discovered
  Defcoin endpoints.

## Apple Silicon Build

The current local Nu build uses Qt 6.10.1 and stages release artifacts outside
the source tree.

```sh
NU_ROOT="/Volumes/TB5_4TB/d/litecoincore/Defcoin Core Nu"
SRC="$NU_ROOT/source/defcoin-core-nu-litecoin-fork"
QT_MAC="$NU_ROOT/toolchains/qt/6.10.1/macos"

cd "$SRC"
cmake -S src/qt/nu/app -B build/nu-qml-arm64 \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_PREFIX_PATH="$QT_MAC" \
  -DQt6_DIR="$QT_MAC/lib/cmake/Qt6" \
  -DDEFCOIN_NU_BACKEND_BINARY="$SRC/src/defcoind" \
  -DDEFCOIN_NU_RELEASE_NAME="26.3.0" \
  -DDEFCOIN_NU_ENABLE_HELP=OFF

cmake --build build/nu-qml-arm64 --target DefcoinCoreNuResources -- -j1

src/qt/nu/app/stage_macos_distribution.sh \
  "$SRC/build/nu-qml-arm64/DefcoinCoreNu.app" \
  "$NU_ROOT/Distribution_Versions/Nu-26.3.0/apple-silicon" \
  "26.3.0" \
  "macOS-AppleSilicon"
```

The staged app and DMG are local release artifacts. Do not commit `.app` or
`.dmg` files to source history.

## Windows Cross-Compile Build

The Windows backend is built from a clean source copy to avoid mixing macOS
object files into the MinGW build.

```sh
NU_ROOT="/Volumes/TB5_4TB/d/litecoincore/Defcoin Core Nu"
SRC="$NU_ROOT/source/defcoin-core-nu-litecoin-fork"
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

cd "$WIN_SRC"
./autogen.sh
CONFIG_SITE="/Volumes/TB5_4TB/d/litecoincore/defcoin-core-legacy/depends/x86_64-w64-mingw32/share/config.site" \
  ./configure --prefix=/ --host=x86_64-w64-mingw32 --without-gui --enable-wallet \
  --with-sqlite=yes --with-miniupnpc --disable-zmq --disable-tests --disable-bench \
  --disable-shared --with-pic
make -j1 src/defcoind.exe
```

Then build and stage the Windows Qt Quick shell:

```sh
cd "$SRC"
cmake -S src/qt/nu/app -B build/nu-qml-win64-local \
  -DCMAKE_TOOLCHAIN_FILE="$SRC/depends/cmake/mingw-w64-x86_64.cmake" \
  -DDEFCOIN_NU_BACKEND_BINARY="$WIN_SRC/src/defcoind.exe" \
  -DDEFCOIN_NU_RELEASE_NAME="26.3.0" \
  -DDEFCOIN_NU_ENABLE_HELP=OFF \
  -DQt6_DIR="$NU_ROOT/toolchains/qt/6.10.1/mingw_64/lib/cmake/Qt6" \
  -DQRENCODE_LIBRARY="$NU_ROOT/toolchains/qrencode/win64/lib/libqrencode.a" \
  -DQRENCODE_INCLUDE_DIR="$NU_ROOT/toolchains/qrencode/win64/include" \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build/nu-qml-win64-local --target DefcoinCoreNuResources -j1
```

Use the local release script to create the portable Windows app folder, ZIP,
and NSIS setup executable under `Distribution_Versions/Nu-26.3.0/`.

## Release Artifact Policy

Commit source, documentation, and build scripts. Upload platform installers,
DMGs, ZIPs, and checksums to a GitHub Release instead of committing them to the
repository.
