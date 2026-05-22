Defcoin Core
=============

Setup
---------------------
Defcoin Core is the original Defcoin client and it builds the backbone of the network. It downloads and, by default, stores the entire history of Defcoin transactions, which requires approximately 22 gigabytes of disk space. Depending on the speed of your computer and network connection, the synchronization process can take anywhere from a few hours to a day or more.

The current Nu release line is Defcoin Core Nu `26.3.1`, codename
`Core Memories`.

Running
---------------------
The following are some helpful notes on how to run Defcoin Core on your native platform.

### Unix

Unpack the files into a directory and run:

- `bin/defcoin-qt` (GUI) or
- `bin/defcoind` (headless)

### Windows

Unpack the files into a directory, and then run defcoin-qt.exe.

### macOS

Drag Defcoin Core to your applications folder, and then run Defcoin Core.

### Need Help?

For wallet and node basics, start with the app's About and Build Notes, then
use the developer documents below for implementation details.

Building
---------------------
The following are developer notes on how to build Defcoin Core on your native platform. They are not complete guides, but include notes on the necessary libraries, compile flags, etc.

- [Dependencies](dependencies.md)
- [macOS Build Notes](build-osx.md)
- [Unix Build Notes](build-unix.md)
- [Windows Build Notes](build-windows.md)
- [FreeBSD Build Notes](build-freebsd.md)
- [OpenBSD Build Notes](build-openbsd.md)
- [NetBSD Build Notes](build-netbsd.md)
- [Gitian Building Guide (External Link)](https://github.com/bitcoin-core/docs/blob/master/gitian-building.md)

Defcoin-specific release documentation:

- [Defcoin Core Nu Technical Guide](defcoin-core-nu-technical-guide.md)
- [Defcoin Core Nu 26.3.1 Release Notes](release-notes/release-notes-26.3.1.md)
- [License And Attribution Notices](license-and-attribution-notices.md)

Development
---------------------
The Defcoin repo's [root README](/README.md) contains relevant information on the development process and automated testing.

- [Developer Notes](developer-notes.md)
- [Productivity Notes](productivity.md)
- [Release Notes](release-notes.md)
- [Release Process](release-process.md)
- [Source Code Documentation (External Link)](https://doxygen.bitcoincore.org/)
- [Translation Process](translation_process.md)
- [Translation Strings Policy](translation_strings_policy.md)
- [JSON-RPC Interface](JSON-RPC-interface.md)
- [Unauthenticated REST Interface](REST-interface.md)
- [Shared Libraries](shared-libraries.md)
- [BIPS](bips.md)
- [Dnsseed Policy](dnsseed-policy.md)
- [Benchmarking](benchmarking.md)

### Resources

- Source repository: [DefcoinCore/Defcoin-Core-Nu](https://github.com/DefcoinCore/Defcoin-Core-Nu)
- Community forum: [DefcoinTalk](https://defcointalk.io/)

### Miscellaneous
- [Assets Attribution](assets-attribution.md)
- [bitcoin.conf Configuration File](bitcoin-conf.md)
- [Files](files.md)
- [Fuzz-testing](fuzzing.md)
- [Reduce Memory](reduce-memory.md)
- [Reduce Traffic](reduce-traffic.md)
- [Tor Support](tor.md)
- [Init Scripts (systemd/upstart/openrc)](init.md)
- [ZMQ](zmq.md)
- [PSBT support](psbt.md)

License
---------------------
Distributed under the [MIT software license](/COPYING).
