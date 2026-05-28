# Legacy OS X 10.7 Build Notes

Date: 2026-05-28

Host:

- `vnc://192.168.2.19`
- Mac OS X Server 10.7.5, Darwin 11.4.2, i386 kernel
- Xcode 4.6.3
- Apple clang 4.2 / LLVM 3.2
- Git 2.6.3
- 4 GB RAM

GitHub source checkout:

- Repo: `https://github.com/defcoincore/Defcoin-Core-Nu.git`
- Latest published `26.3.x` tag found from GitHub refs: `v26.3.1`
- Remote checkout path: `/Users/david/defcoin-core-nu-build-20260528_034346/src`
- Commit: `bf8bb8c8f841941171c771b4ff0c78f5977d949d`

Observed blockers:

- Homebrew is absent.
- MacPorts is installed at `/opt/local/bin/port`, version 2.12.1.
- The host has no CMake.
- CMake 3.24.4 source bootstrap fails with Apple clang 4.2 because the compiler
  does not support inherited constructors used by CMake 3.24.
- The current Nu Qt 6.10.1 macOS toolchain has Mach-O `minos 13.0`, so it cannot
  run on OS X 10.7.5.
- The current Nu QML imports use QtQuick 2.15 / QtQuick.Controls 2.15. That is
  not portable to Qt 5.5.1.

MacPorts was updated from 2.12.1 to 2.12.5 on 2026-05-28. The ports tree was
refreshed and `port upgrade outdated` was run successfully. `port outdated`
then reported no outdated installed ports, and `port uninstall inactive`
removed inactive duplicate versions.

SSL/TLS notes:

- A local OpenSSL 1.1.1w build under
  `/Users/david/defcoin-core-nu-build-20260528_034346/tools/openssl-1.1.1w`
  completed successfully as an early bootstrap test, but is not the selected
  durable dependency path.
- MacPorts is the preferred durable SSL provider because its `openssl3` port has
  current Lion support.
- A normal `sudo port install openssl3` found a prebuilt Darwin 11 x86_64
  archive, but failed archive signature verification.
- Retried with `sudo port -s install openssl3`; this built and activated
  `clang-11-bootstrap @11.1.0_6+universal`, then built and activated
  `openssl3 @3.6.1_0`.
- After `port selfupdate`, MacPorts upgraded the active OpenSSL3 backport to
  `openssl3 @3.6.2_0`.
- `openssl3` installs its binary at `/opt/local/libexec/openssl3/bin/openssl`
  and its linkable libraries under `/opt/local/libexec/openssl3/lib`.
- `openssl3` expects `/opt/local/share/curl/curl-ca-bundle.crt` via
  `/opt/local/libexec/openssl3/etc/openssl/cert.pem`; install the CA bundle with
  `sudo port -s install curl-ca-bundle` if certificate verification fails.
- Verified modern HTTPS from Lion:
  `/opt/local/libexec/openssl3/bin/openssl s_client -brief -connect github.com:443 -servername github.com`
  negotiated TLS 1.3 and reported `Verification: OK`.
- Qt 5.5.1 predates OpenSSL 1.1/OpenSSL 3 opaque structs and some removed
  macros. The compatibility patch for this branch is
  `depends/patches/qt5.5.1/openssl3-lion-compat.patch`.
- After applying that patch to Qt 5.5.1 `qtbase`, `./configure
  -openssl-linked -I /opt/local/libexec/openssl3/include -L
  /opt/local/libexec/openssl3/lib` configured successfully, `make -j2` returned
  `0`, and `make install` returned `0`.
- `otool -L` on the installed `libQt5Network.5.dylib` confirms linkage to
  `/opt/local/libexec/openssl3/lib/libssl.3.dylib` and
  `/opt/local/libexec/openssl3/lib/libcrypto.3.dylib`.

Current active MacPorts package audit after selfupdate:

- `MacPorts @2.12.5`
- `openssl3 @3.6.2_0`
- `curl-ca-bundle @8.19.0_0`
- `sqlite3 @3.53.1_0`
- `zlib @1.3.2_0`
- `openssl @3_30`
- `autoconf @2.73_0`
- `automake @1.18.1_0`
- `libtool @2.5.4_0`
- `pkgconfig @0.29.2_0`
- `db48 @4.8.30_5`
- `libevent @2.1.12_2`
- `bzip2 @1.0.8_0`
- `clang-11-bootstrap @11.1.0_6+universal`

Backport notes:

- `qt56-qtbase @5.6.3_17` is present in the current MacPorts tree and appears
  to be the newest Qt 5 branch with credible OS X 10.7 viability. Qt 5.7
  supported-platform references list macOS 10.8+ for supported configurations.
- The MacPorts `qt56-qtbase` and `qt55-qtbase` ports still depend on
  `openssl10` and `clang-16`, which is a poor fit for this 4 GB Lion build host
  and for the OpenSSL3 runtime requirement.
- The shipped legacy shell therefore stays on the already-built patched Qt
  5.5.1 + OpenSSL3 prefix until a separate Qt 5.6.3 + OpenSSL3 source build can
  be attempted and tested without destabilizing the working package.

Compatibility decision:

- Keep the current Apple Silicon and Windows 11 build path on the existing Qt 6
  branch.
- Keep Lion work on a separate branch: `legacy/osx-10.7-qt5.5`.
- Use Qt 5.5.1 `qtbase`, qmake, and the classic Qt Widgets wallet for the
  legacy app.
- Treat this as a compatibility product. Do not down-port the current Qt Quick
  Controls 2 interface into the Lion branch.

Remote build workspace:

- `/Users/david/defcoin-core-nu-build-20260528_034346`
- Progress log: `/Users/david/defcoin-core-nu-build-20260528_034346/progress.log`
- CMake source attempt: `/Users/david/defcoin-core-nu-build-20260528_034346/cmake-3.24.4`
- Qt 5.5.1 source attempt: `/Users/david/defcoin-core-nu-build-20260528_034346/qtbase-opensource-src-5.5.1`
- Working Qt 5.5.1 + OpenSSL3 source:
  `/Users/david/defcoin-core-nu-build-20260528_034346/qtbase-opensource-src-5.5.1-openssl3test`
- Installed Qt 5.5.1 + OpenSSL3 prefix:
  `/Users/david/defcoin-core-nu-build-20260528_034346/tools/qt-5.5.1-openssl3test`

Initial source change:

- Added `src/qt/nu/legacy-osx107/`.
- Added a qmake-based Qt Widgets launcher/status app.
- The launcher can start a bundled `Resources/nu/bin/defcoind` when present and
  show the Defcoin debug log.
- Built successfully on Lion with:
  `/Users/david/defcoin-core-nu-build-20260528_034346/tools/qt-5.5.1-openssl3test/bin/qmake`
- Built app:
  `/Users/david/defcoin-core-nu-build-20260528_034346/src/src/qt/nu/legacy-osx107/DefcoinCoreNuLegacy.app`
- Built executable is a 64-bit x86_64 Mach-O and links against Qt 5.5.1 from
  the isolated Qt prefix via `@rpath`.
- This launcher-only app was useful as an early packaging proof, but it is
  superseded by the full `src/qt/defcoin-qt` wallet build below.

Daemon build notes:

- Avoided MacPorts `boost176 +clang11` and `libfmt8` because they tried to pull
  a large modern compiler/CMake/Python stack onto the 4 GB Lion host.
- Built a staged static Boost 1.76.0 subset with MacPorts
  `clang-11-bootstrap` and `b2` built directly in debug mode to avoid an old
  `/usr/bin/ld` internal error during stripped B2 bootstrap.
- Built staged static fmt 8.1.1 directly with `clang++`, without CMake.
- Configured the daemon with the staged Boost/fmt, MacPorts BDB 4.8, sqlite,
  libevent, and MacPorts `openssl3`.
- Added a Lion fallback for `std::shared_timed_mutex` in
  `src/libmw/include/mw/common/Lock.h`; the fallback maps shared locking to a
  plain mutex only when the macOS deployment target is older than 10.12.
- Adjusted `src/crypto/scrypt.h` and `src/crypto/scrypt.cpp` so macOS uses the
  local endian helpers instead of including missing Lion-era endian headers.
- Built daemon:
  `/Users/david/defcoin-core-nu-build-20260528_034346/src/src/defcoind`
- Built daemon is a 64-bit x86_64 Mach-O. Before packaging it links to
  MacPorts BDB, OpenSSL3, libevent, and sqlite.

Packaging:

- Added `contrib/legacy-osx107/package_legacy_dmg.sh`.
- The package script can run on the Lion Mac or on a newer Mac after rsyncing
  the Lion-built app, daemon, Qt prefix, OpenSSL3 prefix, CA bundle, and needed
  MacPorts dylibs.
- The script stages the app as `Defcoin Core Nu.app`, adds
  `Contents/Resources/nu/bin/defcoind`, adds
  `Contents/Resources/nu/ssl/cert.pem`, writes `Contents/Resources/qt.conf`,
  bundles Qt 5.5.1, Qt Cocoa plugin, OpenSSL3, BDB, libevent, sqlite, and
  MacPorts zlib, then rewrites install names for app, plugin, framework, and
  backend locations.
- The script reads `CFBundleExecutable`, so it can package either the temporary
  launcher or the real `Defcoin-Qt` app bundle.
- The script forces `LSMinimumSystemVersion` to `10.7.0`.
- The script copies `src/qt/nu/assets` into `Contents/Resources/nu/assets` when
  those assets exist, so the Lion Nu shell has the same Nu visual resources.
- The script strips stale build-prefix `LC_RPATH` values so the Cocoa plugin
  cannot load Qt from the original Qt build prefix.
- The DMG must be HFS+, not APFS. The final script forces `-fs HFS+` so
  Mac OS X 10.7.5 can mount the image.
- The DMG background/layout now matches the Apple Silicon build when passed the
  Apple Silicon background PNG: 640x420 window, 72px icons, app at `{150, 285}`,
  and Applications at `{450, 285}`.

Classic Qt wallet reference build:

- Built the Qt translation release tool from Qt Tools 5.5.1 because `lrelease`
  was missing from the Qt 5.5.1 `qtbase` install.
- Configured the main source tree with `--with-gui=qt5 --enable-wallet
  --with-sqlite=yes --without-miniupnpc --without-qrencode --disable-zmq
  --disable-tests --disable-gui-tests --disable-bench` against the patched Qt
  5.5.1 prefix, staged Boost 1.76.0, staged fmt 8.1.1, MacPorts BDB 4.8,
  MacPorts OpenSSL3, MacPorts libevent, and MacPorts sqlite.
- Added Qt 5.5 compatibility fixes for newer Qt APIs used by the current UI:
  `setWindowFlag`, `QOverload`, `QDateTime::currentSecsSinceEpoch`,
  `QList::constFirst/constLast`, `QVector::crbegin/crend`, and
  `Qt::ISODateWithMs`.
- Gated `NSUserNotification` and App Nap code for Lion-era SDK support.
- Built `src/qt/defcoin-qt`, a real 64-bit x86_64 wallet executable.
- Hid Qt `libQt5*.la` files during the final link because GNU libtool on Lion
  tried to parse framework arguments from Qt's `.la` dependency metadata and
  failed with `unhandled argument '-framework'`.
- Built the native app bundle with `make appbundle`, then packaged
  `Defcoin-Qt.app` as `Defcoin Core Nu.app`.
- This build was later identified as the classic Defcoin Core / standard Qt
  wallet UI lineage. Keep it as a working Lion reference build. Do not treat it
  as the Nu interface.

Nu UI Lion build:

- The current Nu UI in `src/qt/nu` is a standalone Qt 6 / QML / Qt Quick
  Controls 2.15 app. It cannot run directly on OS X 10.7.5.
- The Lion-compatible Nu artifact therefore uses `src/qt/nu/legacy-osx107`,
  a Qt 5.5.1 Widgets shell that mirrors the Nu product shell, iconography,
  route model, status cards, and backend boundary without pulling in the classic
  `Defcoin-Qt` wallet UI.
- The Lion Nu shell bundles the Nu assets into
  `Contents/Resources/nu/assets`, starts the bundled `defcoind` with
  `listen=1`, `discover=1`, and `allowlannodediscovery=1`, and exposes live
  node/log status in Home, Wallet, Node, and Activity views.
- Built on Lion with the patched Qt 5.5.1 prefix through qmake:
  `src/qt/nu/legacy-osx107/DefcoinCoreNuLegacy.pro`.
- Staged on Lion with `contrib/legacy-osx107/package_legacy_dmg.sh --no-dmg`
  so dependency rewriting still happens on the 10.7 machine.
- The final Nu UI DMG was created on the Tahoe Mac with Python `dmgbuild`
  instead of Finder AppleScript. This avoids intermittent Finder AppleEvent
  timeouts on Lion and applies the same 640x420 Apple Silicon DMG background
  and icon layout.
- The Tahoe Mac was used only to assemble DMG metadata; the Intel/Lion app was
  launched and validated on the Mac OS X 10.7.5 machine.

Apple Silicon packaging command used:

```sh
./contrib/legacy-osx107/package_legacy_dmg.sh \
  --app "/Volumes/TB5_4TB/d/litecoincore/Defcoin Core Nu/build/osx107-stage/DefcoinCoreNuLegacy.app" \
  --qt-prefix "/Volumes/TB5_4TB/d/litecoincore/Defcoin Core Nu/build/osx107-stage/qt-5.5.1-openssl3test" \
  --defcoind "/Volumes/TB5_4TB/d/litecoincore/Defcoin Core Nu/build/osx107-stage/defcoind" \
  --openssl-prefix "/Volumes/TB5_4TB/d/litecoincore/Defcoin Core Nu/build/osx107-stage/openssl3" \
  --macports-prefix "/Volumes/TB5_4TB/d/litecoincore/Defcoin Core Nu/build/osx107-stage/macports" \
  --ca-bundle "/Volumes/TB5_4TB/d/litecoincore/Defcoin Core Nu/build/osx107-stage/curl-ca-bundle.crt" \
  --output-dir "/Volumes/TB5_4TB/d/litecoincore/Defcoin Core Nu/build/osx107-dmg" \
  --version "26.3.1-lion"
```

Superseded launcher-only artifact:

- `/Volumes/TB5_4TB/d/litecoincore/Defcoin Core Nu/build/osx107-dmg/Defcoin-Core-Nu-26.3.1-lion-osx107.dmg`
- Size: about 22 MB.
- SHA256:
  `ffb4197f3f71c69228a1a2255f0236ac4525daa32cec1c3400b6632a68ada956`
- Bundled upgraded backports verified locally before Lion smoke testing:
  OpenSSL `3.6.2`, SQLite `3.53.1`, zlib `1.3.2`, CA bundle `8.19.0`.
- This artifact mounted and launched, but it only contained the temporary
  launcher shell and did not provide the full wallet UI.

Final full Qt wallet artifact:

- `/Volumes/TB5_4TB/d/litecoincore/Defcoin Core Nu/build/osx107-dmg/Defcoin-Core-Nu-26.3.1-lion-fullqt-osx107.dmg`
- Remote source artifact:
  `/Users/david/defcoin-core-nu-build-20260528_034346/full-qt-package/Defcoin-Core-Nu-26.3.1-lion-fullqt-osx107.dmg`
- SHA256:
  `fca4846d1d4f0376a1bf34cdf5df9d5571cd118d623a738ee7e62e4fb7b5307e`
- `hdiutil verify` on Mac OS X 10.7.5 reports the checksum is valid.
- The image mounts as Apple_HFS at `/Volumes/Defcoin Core Nu`.
- The mounted DMG contains `Defcoin Core Nu.app` plus an `/Applications`
  symlink and the Apple Silicon background art/layout.
- The packaged app launches from the mounted DMG on Mac OS X 10.7.5.
- Runtime log evidence from the mounted DMG:
  `Defcoin Core Nu version v26.3.1-17e38a4-dirty`, `Qt 5.5.1 (dynamic),
  plugin=cocoa (dynamic)`, and `System: OS X Lion (10.7), x86_64-little_endian-lp64`.
- Keep this artifact for reference as the classic Defcoin Core standard-style
  Lion build.

Final Nu UI Lion artifact:

- `/Volumes/TB5_4TB/d/litecoincore/Defcoin Core Nu/build/osx107-dmg/Defcoin-Core-Nu-26.3.1-lion-nuui-osx107.dmg`
- SHA256:
  `e3f6c796ed3a6983aa98c0755789287b9b97da71094a3856886711cb1d39595a`
- Built from the Lion-staged app archive:
  `/Users/david/defcoin-core-nu-build-20260528_034346/nu-ui-legacy-stage/Defcoin-Core-Nu-Lion-NuUI-stage.tar.gz`
- The app launch was tested from the mounted DMG on Mac OS X 10.7.5. Final
  screenshot:
  `/Users/david/defcoin-core-nu-build-20260528_034346/screenshot-nu-ui-lion-final.png`
- The visible UI is the Nu shell with a dark navigation rail, Nu icon, Home,
  Wallet, Send, Receive, Node, Activity routes, and node status cards.
- The final DMG was generated with Tahoe-side `dmgbuild` using the Apple Silicon
  background PNG and icon positions to avoid Lion Finder DMG metadata timeouts.

Final Lion validation:

- Copied the DMG to
  `/Users/david/defcoin-core-nu-build-20260528_034346/Defcoin-Core-Nu-26.3.1-lion-osx107.dmg`.
- `hdiutil verify` on Mac OS X 10.7.5 reports the checksum is valid.
- `hdiutil attach` mounts the image as Apple_HFS at
  `/Volumes/Defcoin Core Nu 26.3.1-lion`.
- The packaged daemon runs from the mounted DMG:
  `Defcoin Core Nu version v26.3.1-17e38a4`.
- The packaged Qt launcher runs from the mounted DMG and stayed alive in a
  5-second process smoke test.
- Repeated the DMG build and Lion smoke test after the MacPorts backport
  upgrades. `hdiutil verify`, HFS+ attach, packaged `defcoind -version`, and the
  5-second Qt launcher process smoke test all passed.
- Launched the full wallet GUI on the Lion desktop through Screen Sharing.
- Created a test wallet named `alegacy-test`; it loaded as Berkeley DB format,
  generated a keypool, and produced `wallet.dat` under the test datadir.
- Confirmed network and sync through RPC/logs: `networkactive: true`, 9
  outbound connections during the wallet test, headers and blocks syncing, and
  `UpdateTip` entries after block download started.
- Confirmed LAN discovery/listening defaults in the app config path:
  `listen=1`, `discover=1`, and `allowlannodediscovery=1`. Runtime peer evidence
  included LAN peers at `192.168.2.51:1337` and `192.168.3.69:1337`.
- SSH transfer benchmark with the persistent control socket: Tahoe to Lion
  moved 64 MB in about 29 seconds; Lion to Tahoe moved 64 MB in about 27
  seconds. SSH/rsync remains the simplest reliable transfer path, and SMB1 was
  not needed.

Published release artifact:

- Release: `v26.3.1`
- Asset:
  `https://github.com/defcoincore/Defcoin-Core-Nu/releases/download/v26.3.1/Defcoin-Core-Nu-v26.3.1-macOS-10.7-Lion-Intel.dmg`
- The release asset was replaced after full-wallet validation. Current GitHub
  asset size: `75910097` bytes.
- `SHA256SUMS.txt` on the release was updated with the full Qt wallet DMG hash:
  `fca4846d1d4f0376a1bf34cdf5df9d5571cd118d623a738ee7e62e4fb7b5307e`.

Branch boundary:

- Preserve all Lion work on `legacy/osx-10.7-qt5.5`.
- Do not merge these Qt 5.5 compatibility constraints into the current Qt 6
  Apple Silicon / Windows 11 build path.
