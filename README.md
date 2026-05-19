Defcoin Core Nu
===============

Defcoin Core Nu is a full-node desktop wallet for the Defcoin network. The
26.3.0 release, codename `Core Memories`, is derived from Litecoin Core
v0.21.5.5 and keeps the Defcoin chain rules, wallet data directory, and
Litecoin-derived backend while adding a Qt Quick desktop interface inspired by
Nothing Company product design and the Bitcoin Design Community.

Key network facts
-----------------

- Proof of work: Scrypt
- Target block time: 2 minutes
- Difficulty retarget interval: 720 blocks
- Subsidy halving interval: 840,000 blocks
- Initial block reward: 50 DFC
- Mainnet P2P/RPC ports: 1337 / 9332
- Mainnet legacy P2P magic: `fb c0 b6 db`
- Mainnet Defcoin P2P magic: `de fc 01 4e`
- Mainnet seed hosts: `seed.defcoin.io`, `seed.defcoin.mikej.tech`,
  `seed.defcoin.dc903.org`, `seed.defcoincore.org`
- Testnet P2P/RPC ports: 31337 / 19332
- Regtest P2P/RPC ports: 19444 / 19443
- macOS data directory: `~/Library/Application Support/Defcoin/`
- Config file: `defcoin.conf`
- Display units: `DFC`, `Packet`, `Tock`, `Mote`

Compatibility Notes
-------------------

This port follows the Defcoin compatibility guidance from
`packetloss404/DefCoinCore`: preserve the historical chain rules first, and do
not blindly enable newer Litecoin mainnet consensus features.

- Mainnet should use legacy Base58 addresses.
- SegWit, CSV, CLTV, BIP66, Taproot, and MWEB are not treated as active Defcoin
  mainnet consensus features in this branch.
- Berkeley DB wallet support is enabled for legacy `wallet.dat` compatibility.
- Back up old wallets before testing them with this software.
- Signet is inherited from Litecoin Core internals but is not a supported
  Defcoin network target yet.

Nu also includes a migration path away from the legacy Litecoin-family message
start bytes. In compatibility mode it can accept both legacy `fbc0b6db` and
Defcoin-specific `defc014e` peers, prefers `defc014e` for upgraded outbound
connections, and records the actual per-peer magic in diagnostics. This is a
network-efficiency and peer-pollution fix, not a consensus hard fork.

Differences From Defcoin 1.0.0 And 1.0.1
----------------------------------------

- Defcoin Core Nu uses a Qt Quick shell organized around Home, Send, Receive,
  Activity, Diagnostics, and Settings.
- Nu runs a bundled `defcoind` backend as a managed child process and connects
  to it through local RPC, so launch diagnostics, backend readiness, and logs
  are visible in the app.
- Nu adds peer user-agent filtering, dual-magic migration controls, and a peer
  table that reports actual observed network magic.
- Nu restores current Litecoin-derived wallet flows that are still appropriate
  for Defcoin, including receive request history, transaction details, PSBT
  handling, message signing, wallet backup, encryption, and passphrase changes.
- Nu keeps the existing Defcoin wallet directory and chain data; it does not
  require users to redownload the chain or migrate wallet files for this UI
  change.

Differences From Litecoin Core v0.21.5.5
----------------------------------------

- Product identity, app names, URI scheme, units, artwork, and packaging are
  Defcoin-specific.
- Mainnet ports are `1337` for P2P and `9332` for RPC.
- Defcoin chain parameters, genesis blocks, checkpoints, address prefixes,
  seed hosts, and historical consensus compatibility are used.
- Litecoin mainnet deployments that are not active for Defcoin are disabled or
  documented as inactive for Defcoin mainnet.
- Defcoin-specific P2P hygiene controls are added to reduce Litecoin-family
  peer pollution on a small network.

Build
-----

See [doc/defcoin-core-nu-github-notes.md](doc/defcoin-core-nu-github-notes.md),
[doc/build-osx.md](doc/build-osx.md), [doc/build-windows.md](doc/build-windows.md),
and [doc/defcoin-modern-build-guide.md](doc/defcoin-modern-build-guide.md).

For Defcoin-specific consensus, wallet, GUI, and porting notes, see:

- [doc/defcoin-porting-notes.md](doc/defcoin-porting-notes.md)
- [doc/defcoin-core-vs-litecoin-core.md](doc/defcoin-core-vs-litecoin-core.md)
- [doc/defcoin-build-deviation-matrix.md](doc/defcoin-build-deviation-matrix.md)
- [doc/defcoin-litecoin-conversion-notes.md](doc/defcoin-litecoin-conversion-notes.md)
- [doc/defcoin-modern-build-guide.md](doc/defcoin-modern-build-guide.md)
- [doc/litecoin-upstream-comparison-standard.md](doc/litecoin-upstream-comparison-standard.md)
- [doc/github-publication-checklist.md](doc/github-publication-checklist.md)

Native Apple Silicon backend outputs from this branch are expected to be:

- `src/defcoind`
- `src/defcoin-cli`
- `src/defcoin-tx`
- `src/defcoin-wallet`
- `src/qt/defcoin-qt` for the inherited Qt Widgets wallet, where enabled
- `src/qt/nu/app` for the Nu Qt Quick shell

Run the focused smoke test after building:

```sh
./contrib/defcoin-smoke-test.sh
```

To create a local Apple Silicon DMG after `make appbundle`:

```sh
./contrib/defcoin-macos-package-dmg.sh
```

The generated DMG is an ad-hoc signed developer distribution image, not a
Developer ID signed or notarized public release.

The Apple Silicon Nu release DMG is named:

```text
Defcoin-Core-Nu-v26.3.0-macOS-AppleSilicon.dmg
```

Release artifacts are attached to GitHub Releases rather than committed to
source history.

For the current design and porting notes, see:

- [doc/defcoin-core-help-manual.md](doc/defcoin-core-help-manual.md)
- [doc/defcoin-third-party-notices-standard.txt](doc/defcoin-third-party-notices-standard.txt)

Runtime Defcoin artwork and generated Qt/macOS assets needed by the standard
build are tracked in the repository under `src/qt/res/icons/`.

License
-------

Defcoin Core Nu code is released under the terms of the MIT license inherited
from Litecoin Core and Bitcoin Core. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

Trademark rights and the non-commercial coin artwork permission are separate
from the software license and do not grant rights to use Def Con marks outside
the permission already documented for this project.

Copyright (C) 2014-2026 The Defcoin Core developers.

Defcoin Core is derived from Litecoin Core and Bitcoin Core. Portions remain
credited to The Litecoin Core developers and The Bitcoin Core developers.
