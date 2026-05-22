#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -lt 3 ]; then
  echo "usage: $0 <built-app> <destination-platform-dir> <release-version> [dmg-suffix]" >&2
  exit 2
fi

BUILT_APP="$1"
DEST_PLATFORM_DIR="$2"
RELEASE_VERSION="$3"
DMG_SUFFIX="${4:-macOS-AppleSilicon}"
PRODUCT_NAME="Defcoin Core Nu"
DEST_APP="$DEST_PLATFORM_DIR/${PRODUCT_NAME}.app"
DEST_DMG="$DEST_PLATFORM_DIR/Defcoin-Core-Nu-v${RELEASE_VERSION}-${DMG_SUFFIX}.dmg"
RELEASE_DIR="$(dirname "$DEST_PLATFORM_DIR")"

if [ ! -d "$BUILT_APP" ]; then
  echo "built app not found: $BUILT_APP" >&2
  exit 1
fi

mkdir -p "$DEST_PLATFORM_DIR"
rm -rf "$DEST_APP" "$DEST_DMG"

ditto "$BUILT_APP" "$DEST_APP"
rm -f "$DEST_APP/Contents/PlugIns/sqldrivers/libqsqlmimer.dylib"
xattr -cr "$DEST_APP"

codesign --force --deep --sign - "$DEST_APP" >/dev/null
codesign --verify --deep --strict --verbose=4 "$DEST_APP"

hdiutil create -volname "$PRODUCT_NAME" -srcfolder "$DEST_APP" -ov -format UDZO "$DEST_DMG" >/dev/null
hdiutil verify "$DEST_DMG"

touch -ch "$RELEASE_DIR" "$DEST_PLATFORM_DIR" "$DEST_APP" "$DEST_DMG"

STAMP_FILE="$DEST_PLATFORM_DIR/BUILD_STAGED_AT.txt"
{
  echo "Defcoin Core Nu staged distribution"
  echo "Release: $RELEASE_VERSION"
  echo "Staged at: $(date '+%Y-%m-%d %H:%M:%S %Z')"
  echo "Built app: $BUILT_APP"
  echo "App: $DEST_APP"
  echo "DMG: $DEST_DMG"
} > "$STAMP_FILE"
touch -ch "$STAMP_FILE"

if command -v SetFile >/dev/null 2>&1; then
  FINDER_DATE="$(date '+%m/%d/%Y %H:%M:%S')"
  for path in "$RELEASE_DIR" "$DEST_PLATFORM_DIR" "$DEST_APP" "$DEST_DMG" "$STAMP_FILE"; do
    SetFile -d "$FINDER_DATE" "$path" >/dev/null 2>&1 || true
    SetFile -m "$FINDER_DATE" "$path" >/dev/null 2>&1 || true
  done
fi

echo "staged app: $DEST_APP"
echo "staged dmg: $DEST_DMG"
