# Legacy OS X 10.7 Build Notes

Date: 2026-05-28

Host:

- `vnc://192.168.2.19`
- Mac OS X Server 10.7.5, Darwin 11.4.2, i386 kernel
- Xcode 4.6.3
- Apple clang 4.2 / LLVM 3.2
- Git 2.6.3
- 4 GB RAM, about 22 GB free on `/`

GitHub source checkout:

- Repo: `https://github.com/defcoincore/Defcoin-Core-Nu.git`
- Latest published `26.3.x` tag found from GitHub refs: `v26.3.1`
- Remote checkout path: `/Users/david/defcoin-core-nu-build-20260528_034346/src`
- Commit: `bf8bb8c8f841941171c771b4ff0c78f5977d949d`

Observed blockers:

- The host has no package manager installed (`brew` and `port` are absent).
- The host has no CMake.
- CMake 3.24.4 source bootstrap fails with Apple clang 4.2 because the compiler
  does not support inherited constructors used by CMake 3.24.
- The current Nu Qt 6.10.1 macOS toolchain has Mach-O `minos 13.0`, so it cannot
  run on OS X 10.7.5.
- The current Nu QML imports use QtQuick 2.15 / QtQuick.Controls 2.15. That is
  not portable to Qt 5.5.1.

Correction: MacPorts is installed at `/opt/local/bin/port`; Homebrew is absent.
MacPorts 2.12.1 is present and its ports tree includes `openssl3 @3.6.1`.

SSL/TLS notes:

- A local OpenSSL 1.1.1w build under
  `/Users/david/defcoin-core-nu-build-20260528_034346/tools/openssl-1.1.1w`
  completed successfully and can be used as a temporary bootstrap.
- MacPorts is the preferred durable SSL provider because its `openssl3` port has
  current Lion support.
- A normal `sudo port install openssl3` found a prebuilt Darwin 11 x86_64
  archive, but failed archive signature verification.
- Retried with `sudo port -s install openssl3`; this moved to a source build
  and began building the required `clang-11-bootstrap` dependency.

Compatibility decision:

- Keep the current Apple Silicon and Windows 11 build path on the existing Qt 6
  branch.
- Keep Lion work on a separate branch: `legacy/osx-10.7-qt5.5`.
- Use Qt 5.5.1 `qtbase`, qmake, and a Qt Widgets shell for the legacy app.
- Treat this as a compatibility product, not a down-port of the full Qt Quick
  Controls 2 interface.

Remote build workspace:

- `/Users/david/defcoin-core-nu-build-20260528_034346`
- Progress log: `/Users/david/defcoin-core-nu-build-20260528_034346/progress.log`
- CMake source attempt: `/Users/david/defcoin-core-nu-build-20260528_034346/cmake-3.24.4`
- Qt 5.5.1 source attempt: `/Users/david/defcoin-core-nu-build-20260528_034346/qtbase-opensource-src-5.5.1`

Initial source change:

- Added `src/qt/nu/legacy-osx107/`.
- Added a qmake-based Qt Widgets launcher/status app.
- The launcher can start a bundled `Resources/nu/bin/defcoind` when present and
  show the Defcoin debug log.
