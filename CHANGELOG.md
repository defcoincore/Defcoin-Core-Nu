# Defcoin Core Changelog

## 26.3.0 Core Memories

Defcoin Core Nu `26.3.0`, codename `Core Memories`, is the current public Nu
release track.

### Added

- Qt Quick Nu shell organized around Home, Send, Receive, Activity,
  Diagnostics, and Settings.
- Managed backend startup for bundled `defcoind` builds, with launch
  diagnostics surfaced in the app.
- Dual Defcoin P2P magic migration support: legacy `fbc0b6db` and
  Defcoin-specific `defc014e`.
- Peer diagnostics that show actual observed peer magic, protocol version,
  services, User-Agent, traffic, and synchronization details.
- Defcoin User-Agent filtering using the `/Defcoin` prefix rule.
- Receive request history, transaction detail actions, PSBT handling, message
  signing, wallet backup, encryption, and passphrase flows.
- Public-safe technical documentation consolidated in
  `doc/defcoin-core-nu-technical-guide.md`.

### Changed

- Public release identity now uses `26.3.0` instead of the older year-style
  checkpoint labels.
- macOS bundle identifiers use the `org.defcoincore` namespace.
- Mainnet seed documentation and runtime lists include `seed.defcoin-ng.org`
  as a candidate DNS seed.
- Build and publication docs remove local workstation paths, personal handles,
  private machine details, and repeated stale instructions.

### Notes

- The inherited Litecoin-derived backend remains the consensus, wallet, and
  networking authority.
- Defcoin-only magic is a network-isolation goal, not a consensus hard fork.
- Release artifacts belong on GitHub Releases, not in source history.
