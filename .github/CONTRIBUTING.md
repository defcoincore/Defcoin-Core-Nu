# Contributing To Defcoin Core Nu

Defcoin Core Nu is a Defcoin full-node wallet derived from Litecoin Core and
Bitcoin Core. Contributions should preserve Defcoin chain compatibility first.

## Before Opening A Pull Request

- Read the [technical guide](../source/doc/defcoin-core-nu-technical-guide.md).
- Build and test on the platform affected by your change.
- Keep changes scoped and explain the user-facing or network-facing reason.
- Do not change consensus behavior unless the change is explicitly reviewed as
  a Defcoin consensus change.
- Do not include private machine paths, local credentials, wallet files, RPC
  cookies, or API tokens in commits or logs.

## Useful Test Commands

The inherited Litecoin Core test layout remains in place. Depending on the
change, useful checks may include:

```sh
cd source
make check
test/functional/test_runner.py
```

For desktop packaging and Nu-specific runtime checks, use the platform build
notes linked from the root [README](../README.md).

## Pull Request Expectations

Pull requests should include:

- A concise description of the problem and solution.
- The Defcoin Core Nu version or branch tested.
- The platform tested.
- Screenshots for visible UI changes.
- Notes about any migration, wallet, networking, or compatibility impact.

## Inherited Upstream Code

Large parts of this repository come from Litecoin Core and Bitcoin Core.
Preserve upstream style and structure where practical, but update user-facing
Defcoin documentation and product copy when inherited text is no longer true
for this project.
