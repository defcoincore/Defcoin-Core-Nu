# Defcoin Core Nu 26.3.0 Release Notes

Codename: `Core Memories`

Defcoin Core Nu `26.3.0` is a full-node Defcoin desktop wallet derived from
Litecoin Core `v0.21.5.5`. It keeps the Litecoin-derived Defcoin backend and
adds a Qt Quick desktop interface, bundled backend launch management, startup
diagnostics, receive-request history, transaction detail actions, PSBT tools,
peer diagnostics, dual-magic migration controls, and stricter Defcoin
peer/address hygiene.

Notable release changes:

- Current release identity: `26.3.0 Core Memories`.
- Defcoin-specific mainnet magic support: preferred `defc014e` with temporary
  compatibility for legacy `fbc0b6db`.
- Peer User-Agent filtering: accepted Defcoin peers must begin with
  `/Defcoin`.
- Address-gossip filtering: non-Defcoin legacy peers are disconnected before
  their address gossip is accepted or rebroadcast.
- Diagnostics: simple and detailed peer views, actual per-peer magic, compact
  service flags, traffic charts, log viewing, and console access.
- Receive flow: generated request history and request management.
- Settings: Defcoin-only magic controls, launch/network options, wallet safety
  actions, About, and build notes.

Technical details are maintained in
`doc/defcoin-core-nu-technical-guide.md`.
