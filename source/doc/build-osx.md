# macOS Build Notes

For the current Defcoin Core Nu macOS build and packaging process, see
[`defcoin-core-nu-technical-guide.md`](defcoin-core-nu-technical-guide.md).

The short public flow is:

```sh
git clone https://github.com/DefcoinCore/Defcoin-Core-Nu.git
cd Defcoin-Core-Nu

brew install automake libtool pkg-config python libevent qrencode fmt sqlite berkeley-db@4 qt@5 boost@1.85

./autogen.sh
./configure --with-gui=qt5 --enable-wallet --with-sqlite=yes
make -j"$(sysctl -n hw.ncpu)"
make check
make appbundle
```

Nu release builds use the Qt Quick CMake app under `src/qt/nu/app` and stage
DMG artifacts outside the source tree. Public release packages should be signed
and notarized with the maintainer's Apple Developer ID credentials before broad
distribution.

The general dependency notes in `dependencies.md` still apply. Berkeley DB 4.8
is required for legacy wallet compatibility unless building without wallet
support.
