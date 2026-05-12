# Defcoin Core Nu Port Delta

Defcoin Core Nu starts from Litecoin Core `v0.21.5.5` and applies the
standard Defcoin Core `2026.1` port as the engine baseline. The current Nu UI
scaffold is intentionally inactive; the node and wallet behavior is inherited
from the standard build.

## Read-Only Reference

The standard Defcoin reference used for this fork is:

```text
/Volumes/TB5_4TB/d/litecoincore/defcoin-core
tag: v2026.1
commit: b794dd795ff83f2374f1cac9b042a8f5c2b43beb
```

That tree was used as a read-only source of truth. The Nu fork was created from
Litecoin Core `v0.21.5.5` and then received the standard Defcoin port commit.

## Engine Changes That Must Stay Defcoin-Specific

These categories are required for the program to function as a Defcoin node:

- chain parameters: message starts, ports, subsidy and maturity behavior,
  genesis information, DNS seeds, base58/bech32 prefixes, and checkpoint-like
  policy choices;
- network identity: command-line help, user agent, process names, service names,
  bundle identifiers, and Windows/macOS resources;
- wallet display identity: Defcoin units, address labels, QR/URI scheme, splash,
  about text, icons, and installer names;
- disabled Litecoin features: inherited upstream features that are not active in
  this Defcoin build must remain disabled until explicitly ported and tested;
- packaging/build compatibility: macOS Apple Silicon, macOS Intel, and Windows
  build adjustments inherited from the standard build.

## UI Boundary

The Nu interface must not reimplement consensus, wallet persistence, key
management, transaction signing, or peer networking. It should call a typed
client API backed by existing `interfaces::Node` and `interfaces::Wallet`
objects, then later move that API across a separate process boundary.

The QML scaffold lives under:

```text
src/qt/nu/
```

It is not linked into the Qt Widgets build yet.

## Current Fork Baseline

```text
Litecoin upstream: v0.21.5.5
Nu branch: defcoin-core-nu-v2026.1
Nu base commit: d8c8adc0b46cf96cc713455ba4aeb135efabac86
Defcoin port commit applied: b794dd795ff83f2374f1cac9b042a8f5c2b43beb
```

## Next Integration Steps

1. Add a feature flag such as `--enable-defcoin-nu-ui`.
2. Build a small fake service so QML can be visually tested without loading a
   wallet.
3. Add a bridge layer that exposes read-only node and wallet state first.
4. Move send/sign flows behind review-first commands with explicit confirmation.
5. Only after the bridge is stable, wire QML into the build and package it.
