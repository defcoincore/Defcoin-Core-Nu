# Defcoin Core Nu 26.3.1 Release Notes

Codename: `Core Memories`

Defcoin Core Nu `26.3.1` is a cleanup release for the current Nu desktop
wallet line. It keeps the `26.3.0` network, wallet, peer-magic, and
User-Agent filtering behavior while reducing packaged Nu assets to the files
the Qt Quick shell actually uses.

Notable release changes:

- Runtime asset staging now copies an explicit allowlist of Nu icons and brand
  assets instead of the whole asset directory.
- Unused Nu image drafts, duplicate icon outputs, and Finder metadata were
  removed from source and packages.
- Build notes and release-facing documentation were tightened to describe the
  current release only.
- An optional mainnet bootstrap pack is available from the GitHub release
  assets for users who want to speed up first sync.

## Optional Bootstrap Pack

The `Defcoin-bootstrap-mainnet-2332283.zip` release asset contains
`bootstrap.dat`, macOS and Windows import scripts, instructions, and checksums.
It snapshots mainnet through block `2,332,283`. Defcoin Core Nu still verifies
imported blocks and then syncs newer blocks from the network normally.

Technical details are maintained in
`doc/defcoin-core-nu-technical-guide.md`.
