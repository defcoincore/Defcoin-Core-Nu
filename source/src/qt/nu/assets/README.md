# Nu Runtime Assets

Nu runtime assets are intentionally limited to the icons and brand images used
by the Qt Quick shell. The build copies an explicit allowlist into app bundles
so stale drafts, unrelated theme packs, and generated icon intermediates do not
ship with Nu packages.

## Palette

- background: `#0B0D0F`
- foreground: `#F6F6F2`
- action sky: `#5DA9F6`
- connected green: `#19C37D`
- warning yellow: `#E5A400`
- isolated red: `#D93025`

The SVG files use literal colors for now so they can be reviewed in isolation.
