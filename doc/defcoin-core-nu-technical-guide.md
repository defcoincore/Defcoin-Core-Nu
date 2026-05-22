# Defcoin Core Nu Technical Guide

Last updated: 2026-05-20

This is the canonical public technical guide for Defcoin Core Nu. It replaces
the older Defcoin-specific build, porting, comparison, checklist, and
publication notes that previously repeated the same information in several
places.

## Overview

Defcoin Core Nu `26.3.0`, codename `Core Memories`, is a full-node desktop
wallet for the Defcoin network. It is derived from Litecoin Core `v0.21.5.5`
and keeps the inherited Litecoin Core engine where that behavior is still
correct for Defcoin. The Nu release adds a Qt Quick desktop shell, bundled
backend launch management, clearer diagnostics, and Defcoin-specific peer
hygiene controls for a smaller network.

Public source repository:

```text
https://github.com/DefcoinCore/Defcoin-Core-Nu
```

Release binaries, installers, disk images, checksums, and detached signatures
belong on GitHub Releases or another release distribution service. They should
not be committed to source history.

## Release Identity

- Release: `26.3.0`
- Codename: `Core Memories`
- Backend baseline: Litecoin Core `v0.21.5.5`
- Proof of work: Scrypt
- Default data directory: the existing Defcoin data directory
- Config file: `defcoin.conf`
- Display units: `DFC`, `Packet`, `Tock`, `Mote`
- macOS bundle namespace: `org.defcoincore`

The inherited numeric `CLIENT_VERSION` remains available where the upstream
code expects it, but the public Defcoin Core Nu release identity is `26.3.0`.
The peer User-Agent for this release should report a Defcoin prefix and the
Nu release version, for example `/DefcoinCoreNu:26.3.0/`.

## Architecture

Nu has two main process and responsibility boundaries:

- the backend owns consensus, block validation, peer networking, wallet
  storage, private keys, fee policy, transaction funding, signing,
  broadcasting, debug logging, and RPC execution;
- the Qt Quick frontend owns navigation, layout, display preferences, local
  clipboard/export actions, QR presentation, charts, diagnostics presentation,
  and safety-first review flows.

The bundled desktop app may start a packaged `defcoind` backend when no
compatible local RPC backend is already available. The frontend then connects
to that backend through local RPC, using normal cookie authentication. The app
surfaces backend startup state, readiness, debug logs, peer status, traffic,
and wallet state in the Diagnostics and Settings areas.

The QML layer must not hold private-key material, write wallet databases,
construct consensus-critical transactions by hand, or duplicate validation
logic. Wallet and node actions should remain behind backend RPC wrappers such
as the Nu RPC service layer.

## Defcoin And Litecoin Core Differences

Defcoin Core Nu inherits the parts of Litecoin Core that are still useful for a
Defcoin full node:

- block validation, mempool, transaction relay, wallet storage, Berkeley DB
  legacy wallet compatibility, pruning, reindex, rescan, proxy, RPC, REST, and
  Scrypt proof-of-work implementation paths;
- the upstream build and dependency model, with platform-specific adjustments
  for the Nu shell and packaged releases;
- the security and maintenance fixes from the Litecoin Core `v0.21.5.5`
  baseline, including hardening that lives in inherited code paths.

Defcoin Core Nu changes the areas that are Defcoin-specific:

- product names, executable names, app metadata, URI scheme, displayed units,
  artwork, and packaging;
- chain parameters, genesis data, checkpoints, minimum chain work, assumed
  chain size, address prefixes, default ports, and seed hosts;
- peer message-start migration support and peer User-Agent filtering;
- Qt Quick Nu desktop interface and release packaging;
- disabled inherited Litecoin features that are not active Defcoin mainnet
  consensus features.

Defcoin Core Nu also preserves compatibility with the historical Defcoin chain
and wallet directory. Users should not need to redownload the chain or move
wallet files solely because they switch from an older Defcoin desktop wallet
to Nu. Existing wallet files should still be backed up before any software
upgrade.

## Historical Compatibility Boundary

This branch follows the Defcoin compatibility policy from the historical
Defcoin source line: preserve the existing chain first, and do not enable newer
Litecoin mainnet deployments unless they are explicitly valid for Defcoin.

The following inherited Litecoin features are not treated as active Defcoin
mainnet consensus features in this release:

- BIP65 / CLTV
- BIP66
- CSV
- SegWit
- Taproot
- MWEB
- Signet

Some inherited source paths and RPC fields may still exist because they are
part of the upstream Litecoin Core codebase. Their presence in the source tree
does not mean the corresponding feature is active on Defcoin mainnet.

## Network Parameters And Seeds

Mainnet:

| Parameter | Value |
| --- | --- |
| Network ID | `main` |
| Legacy message start | `fb c0 b6 db` |
| Defcoin message start | `de fc 01 4e` |
| Default P2P port | `1337` |
| Default RPC port | `9332` |
| Prune after height | `100000` |
| Target spacing | `120` seconds |
| Target timespan | `86400` seconds |
| Difficulty retarget interval | `720` blocks |
| Subsidy halving interval | `840000` blocks |
| Initial subsidy | `50 DFC` |
| Base58 pubkey prefix | `30` |
| Base58 script prefix | `5` |
| Base58 second script prefix | `50` |
| Base58 secret key prefix | `176` |
| Bech32 HRP | `dfc` |
| MWEB HRP | `dfcmweb` |

Other local networks:

| Network | P2P port | RPC port |
| --- | ---: | ---: |
| Testnet | `31337` | `19332` |
| Regtest | `19444` | `19443` |

Mainnet DNS seeds:

- `seed.defcoin.io`
- `seed.defcoin.mikej.tech`
- `seed.defcoin.dc903.org:10332`
- `seed.defcoincore.org`
- `seed.defcoin-ng.org`

`seed.defcoin.io` is listed only as a seed address operated by another party.
It is not used as a bundle identifier, reverse-DNS namespace, or ownership
signal for this project.

macOS bundle and helper identifiers use the `org.defcoincore` namespace, for
example:

- `org.defcoincore.DefcoinCoreNu`
- `org.defcoincore.DefcoinPayment`

## Peer Magic And User-Agent Filtering

Defcoin Core Nu supports two mainnet P2P message starts during the migration
window:

- legacy compatibility magic: `fbc0b6db`
- Defcoin-specific magic: `defc014e`

Compatibility mode accepts both values. Outbound connections should prefer
`defc014e` when the remote node supports it. After the first valid P2P header
selects a peer's magic value, replies to that peer must use the same magic
value. Peer tables and RPC diagnostics should report the actual selected magic
for each peer rather than inferring it from version or User-Agent text.

This is not a blockchain hard fork. It is a transport-level network isolation
improvement that reduces wasted sockets, handshakes, address pollution, and
CPU/network load from unrelated Litecoin-family peers.

Accepted Defcoin peer User-Agents must begin with `/Defcoin`.
`/DefcoinCore:1.0.0/` is valid because it starts with `/Defcoin`. Do not
document or accept abbreviated ticker-style prefixes unless a separate audit
finds a real Defcoin node family using one.

Legacy-magic peers that do not pass the Defcoin User-Agent prefix check should
be disconnected before their `addr` or `addrv2` gossip is accepted into
addrman or rebroadcast. Address filtering should be endpoint-specific, not
IP-wide. If one host runs a Litecoin service on one port and a Defcoin service
on another port, the valid Defcoin endpoint must remain eligible.

## Wallet And UI Capabilities

Nu organizes the wallet around these main views:

- Home: balances, sync state, wallet lock state, and core node health.
- Send: normal send flow, fee review, transaction details, and signing paths
  exposed through backend RPC.
- Receive: address generation, receive-request history, request details, QR
  display, and request removal.
- Activity: transaction history, details, copy/export actions, and explorer
  links where configured by the user.
- Diagnostics: backend status, logs, console, traffic, and simple or detailed
  peer tables.
- Settings: wallet safety actions, network preferences, display preferences,
  About, and build notes.

Wallet-sensitive operations such as backup, encryption, passphrase changes,
message signing, message verification, PSBT handling, transaction funding, and
broadcasting remain backend-owned. The frontend presents the workflow and
passes the request through RPC wrappers.

Peer diagnostics expose transport and health details useful to Defcoin's
networking and security audience, including actual peer magic, direction,
address, port, ping, bytes sent and received, User-Agent, protocol version,
service flags, DNS name where resolved, and address-gossip counters where
available.

## Build And Package Overview

Use a clean clone of the public repository. Public documentation should use
generic paths such as:

```sh
REPO="$HOME/src/Defcoin-Core-Nu"
SRC="$REPO"
```

Do not publish local workstation paths, mounted drive names, user names,
private credentials, wallet files, RPC cookies, or machine-specific details.

### Backend

The backend follows the inherited Litecoin Core build model. Install the
dependencies for the target platform, configure with wallet support enabled
where legacy wallet compatibility is needed, then build the normal Defcoin
node tools:

- `src/defcoind`
- `src/defcoin-cli`
- `src/defcoin-tx`
- `src/defcoin-wallet`
- `src/qt/defcoin-qt` where the inherited Qt Widgets wallet is enabled

Run focused smoke tests after building:

```sh
./contrib/defcoin-smoke-test.sh
```

### Nu Qt Quick Shell

The Nu shell is built with CMake. A typical local macOS release build is:

```sh
cmake -S src/qt/nu/app -B build/nu-qml-macos \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DDEFCOIN_NU_BACKEND_BINARY="$SRC/src/defcoind" \
  -DDEFCOIN_NU_RELEASE_NAME="26.3.0" \
  -DDEFCOIN_NU_ENABLE_HELP=OFF

cmake --build build/nu-qml-macos --target DefcoinCoreNuResources -- -j1
```

Use `x86_64` for Intel macOS builds. Windows releases are built with the
Windows Qt runtime and the appropriate MinGW/CMake toolchain. Use
single-threaded backend builds on constrained machines.

Stage release artifacts outside the source tree. Do not commit app bundles,
installers, disk images, ZIP files, or generated signing material.

The Apple Silicon Nu release disk image is named:

```text
Defcoin-Core-Nu-v26.3.0-macOS-AppleSilicon.dmg
```

## Release And Publication Process

Before publishing:

- `git status --short` shows only intentional release edits;
- `git diff --check` passes;
- public docs do not contain private paths, credentials, old release labels,
  local machine details, personal handles, old reverse-DNS identifiers, or
  unshipped feature promises;
- seed lists include the five mainnet seed hosts above;
- bundle metadata uses `org.defcoincore`;
- `getnetworkinfo` reports a Defcoin User-Agent beginning with
  `/DefcoinCoreNu:26.3.0/`;
- platform packages are built from clean release inputs;
- checksums and signatures are generated for release artifacts;
- release notes describe only what ships in the release being published.

The source repository should contain the source, public documentation, license
material, and small assets required to build the software. Release binaries
and installers should be attached to releases or distributed through a
dedicated release host.

## AI-Assisted Development Note

Defcoin Core Nu was developed with AI assistance for documentation cleanup,
UI iteration, build scripting, code review, and repetitive source migration
work. Human review, local builds, runtime testing, and open-source
publication remain the controls that make the result auditable. AI assistance
does not change the inherited MIT license or the requirement that maintainers
review security-sensitive wallet and networking changes carefully.

## Security, License, And Trademark Boundaries

Defcoin Core Nu code is released under the MIT license inherited from
Litecoin Core and Bitcoin Core unless a file explicitly states otherwise. New
Nu source and documentation should stay under the same license model.

The software license does not grant trademark or artwork rights. Defcoin coin
imagery and Def Con-related marks have separate permission and ownership
boundaries documented in `doc/license-and-attribution-notices.md`.

Security-sensitive work should be reviewed with extra care, especially changes
to wallet encryption, signing, transaction construction, address relay,
network-message parsing, peer filtering, seed handling, and release signing.

## Public Documentation Map

- `README.md`: project overview and common build entry points.
- `doc/README.md`: index of inherited and Defcoin-specific documentation.
- `doc/release-notes/release-notes-26.3.0.md`: current release notes.
- `doc/license-and-attribution-notices.md`: license, dependency, artwork, and
  attribution notices.
- `src/qt/nu/docs/`: Nu frontend implementation notes for developers.
