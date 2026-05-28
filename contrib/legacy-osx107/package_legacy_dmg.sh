#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage:
  package_legacy_dmg.sh --app PATH --qt-prefix PATH --output-dir PATH [options]

Options:
  --defcoind PATH       Copy defcoind into Contents/Resources/nu/bin/defcoind
  --openssl-prefix PATH Prefix containing lib/libssl.3.dylib and lib/libcrypto.3.dylib
                        Defaults to /opt/local/libexec/openssl3.
  --macports-prefix PATH
                        Prefix containing MacPorts lib dependencies such as
                        libevent, sqlite, and db48. Defaults to /opt/local.
  --ca-bundle PATH      Copy CA bundle to Contents/Resources/nu/ssl/cert.pem.
                        Defaults to /opt/local/share/curl/curl-ca-bundle.crt.
  --version VERSION     Version label for the DMG filename. Defaults to 26.3.1-lion.
  --background PATH     PNG image to use as a DMG background. If omitted, one is generated.
  --no-dmg              Stage the app only; do not create a DMG.
  --no-strip            Do not strip local Mach-O symbols from bundled binaries.

The script is intended for the legacy OS X 10.7 Qt 5.5.1 build. It can run on
the Lion Mac or on a newer Mac after rsyncing the built app and Qt prefix.
EOF
}

APP_IN=
QT_PREFIX=
DEFCOIND=
OPENSSL_PREFIX=/opt/local/libexec/openssl3
MACPORTS_PREFIX=/opt/local
CA_BUNDLE=/opt/local/share/curl/curl-ca-bundle.crt
OUTPUT_DIR=
VERSION=26.3.1-lion
BACKGROUND=
MAKE_DMG=1
STRIP_BINARIES=1

while [[ $# -gt 0 ]]; do
  case "$1" in
    --app) APP_IN=$2; shift 2 ;;
    --qt-prefix) QT_PREFIX=$2; shift 2 ;;
    --defcoind) DEFCOIND=$2; shift 2 ;;
    --openssl-prefix) OPENSSL_PREFIX=$2; shift 2 ;;
    --macports-prefix) MACPORTS_PREFIX=$2; shift 2 ;;
    --ca-bundle) CA_BUNDLE=$2; shift 2 ;;
    --output-dir) OUTPUT_DIR=$2; shift 2 ;;
    --version) VERSION=$2; shift 2 ;;
    --background) BACKGROUND=$2; shift 2 ;;
    --no-dmg) MAKE_DMG=0; shift ;;
    --no-strip) STRIP_BINARIES=0; shift ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Unknown option: $1" >&2; usage >&2; exit 2 ;;
  esac
done

if [[ -z "$APP_IN" || -z "$QT_PREFIX" || -z "$OUTPUT_DIR" ]]; then
  usage >&2
  exit 2
fi

if [[ ! -d "$APP_IN" ]]; then
  echo "App bundle not found: $APP_IN" >&2
  exit 1
fi

if [[ ! -d "$QT_PREFIX" ]]; then
  echo "Qt prefix not found: $QT_PREFIX" >&2
  exit 1
fi

ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
PRODUCT_NAME="Defcoin Core Nu"
STAGE="$OUTPUT_DIR/stage"
APP="$STAGE/$PRODUCT_NAME.app"
FRAMEWORKS="$APP/Contents/Frameworks"
RESOURCES="$APP/Contents/Resources"
NU_RESOURCES="$RESOURCES/nu"
DMG_ROOT="$OUTPUT_DIR/dmg-root"
VOLNAME="$PRODUCT_NAME"
DMG_RW="$OUTPUT_DIR/${PRODUCT_NAME// /-}-$VERSION-rw.dmg"
DMG_FINAL="$OUTPUT_DIR/${PRODUCT_NAME// /-}-$VERSION-osx107.dmg"

rm -rf "$STAGE" "$DMG_ROOT"
mkdir -p "$OUTPUT_DIR" "$STAGE"
cp -R "$APP_IN" "$APP"
mkdir -p "$FRAMEWORKS" "$NU_RESOURCES/bin" "$NU_RESOURCES/ssl"

PLIST="$APP/Contents/Info.plist"
if [[ -x /usr/libexec/PlistBuddy && -f "$PLIST" ]]; then
  /usr/libexec/PlistBuddy -c "Set :CFBundleName $PRODUCT_NAME" "$PLIST" 2>/dev/null || true
  /usr/libexec/PlistBuddy -c "Set :CFBundleDisplayName $PRODUCT_NAME" "$PLIST" 2>/dev/null || true
  /usr/libexec/PlistBuddy -c "Set :CFBundleShortVersionString $VERSION" "$PLIST" 2>/dev/null || true
  /usr/libexec/PlistBuddy -c "Set :CFBundleVersion $VERSION" "$PLIST" 2>/dev/null || true
  /usr/libexec/PlistBuddy -c "Set :LSMinimumSystemVersion 10.7.0" "$PLIST" 2>/dev/null || \
    /usr/libexec/PlistBuddy -c "Add :LSMinimumSystemVersion string 10.7.0" "$PLIST" 2>/dev/null || true
fi

cat > "$RESOURCES/qt.conf" <<'EOF'
[Paths]
Plugins = PlugIns
EOF

if [[ -n "$DEFCOIND" ]]; then
  if [[ ! -x "$DEFCOIND" ]]; then
    echo "defcoind is not executable: $DEFCOIND" >&2
    exit 1
  fi
  cp "$DEFCOIND" "$NU_RESOURCES/bin/defcoind"
fi

if [[ -f "$CA_BUNDLE" ]]; then
  cp "$CA_BUNDLE" "$NU_RESOURCES/ssl/cert.pem"
fi

copy_dylib() {
  local src=$1
  local dst="$FRAMEWORKS/$(basename "$src")"
  if [[ ! -f "$src" ]]; then
    echo "Required dylib not found: $src" >&2
    exit 1
  fi
  cp -L "$src" "$dst"
  chmod u+w "$dst"
}

for lib in libQt5Core.5.dylib libQt5Gui.5.dylib libQt5Widgets.5.dylib; do
  copy_dylib "$QT_PREFIX/lib/$lib"
done

copy_dylib "$OPENSSL_PREFIX/lib/libssl.3.dylib"
copy_dylib "$OPENSSL_PREFIX/lib/libcrypto.3.dylib"

mkdir -p "$APP/Contents/PlugIns/platforms"
cp "$QT_PREFIX/plugins/platforms/libqcocoa.dylib" "$APP/Contents/PlugIns/platforms/"
chmod u+w "$APP/Contents/PlugIns/platforms/libqcocoa.dylib"

dep_source_path() {
  local dep=$1
  case "$dep" in
    "$OPENSSL_PREFIX"/*) printf '%s\n' "$dep" ;;
    /opt/local/libexec/openssl3/*) printf '%s\n' "$OPENSSL_PREFIX${dep#/opt/local/libexec/openssl3}" ;;
    /opt/local/*) printf '%s\n' "$MACPORTS_PREFIX${dep#/opt/local}" ;;
    *libQt5Core.5.dylib|*libQt5Gui.5.dylib|*libQt5Widgets.5.dylib|*libQt5Network.5.dylib|*libQt5PrintSupport.5.dylib)
      printf '%s\n' "$QT_PREFIX/lib/$(basename "$dep")"
      ;;
    *) printf '%s\n' "$dep" ;;
  esac
}

is_bundle_dependency() {
  local dep=$1
  case "$dep" in
    /opt/local/*|"$OPENSSL_PREFIX"/*|*libQt5Core.5.dylib|*libQt5Gui.5.dylib|*libQt5Widgets.5.dylib|*libQt5Network.5.dylib|*libQt5PrintSupport.5.dylib)
      return 0
      ;;
    *)
      return 1
      ;;
  esac
}

copy_legacy_nu_assets() {
  local src="$ROOT_DIR/src/qt/nu/assets"
  local dst="$NU_RESOURCES/assets"
  [[ -d "$src" ]] || return 0

  rm -rf "$dst"
  mkdir -p "$dst/brand" "$dst/icons"
  cp "$src/brand/defcoin-nu-icon-1024.png" "$dst/brand/"
  for icon in activity home node receive send wallet; do
    cp "$src/icons/$icon.svg" "$dst/icons/"
  done
}

copy_dependency_closure() {
  local bin=$1
  local dep src dst
  [[ -e "$bin" ]] || return 0

  while IFS= read -r dep; do
    is_bundle_dependency "$dep" || continue
    src=$(dep_source_path "$dep")
    dst="$FRAMEWORKS/$(basename "$dep")"
    [[ -f "$dst" ]] && continue
    [[ -f "$src" ]] || continue
    cp -L "$src" "$dst"
    chmod u+w "$dst"
    copy_dependency_closure "$dst"
  done < <(otool -L "$bin" | awk 'NR > 1 {print $1}')
}

strip_macho_binary() {
  local bin=$1
  [[ "$STRIP_BINARIES" -eq 1 && -f "$bin" ]] || return 0
  case "$(file -b "$bin" 2>/dev/null || true)" in
    Mach-O*)
      chmod u+w "$bin" 2>/dev/null || true
      /usr/bin/strip -x "$bin" 2>/dev/null || true
      ;;
  esac
}

rewrite_binary() {
  local bin=$1
  local dep_target=${2:-@executable_path/../Frameworks}
  local rpath
  [[ -e "$bin" ]] || return 0

  install_name_tool -id "@loader_path/$(basename "$bin")" "$bin" 2>/dev/null || true
  while IFS= read -r rpath; do
    case "$rpath" in
      "$QT_PREFIX"/lib|"$OPENSSL_PREFIX"/lib|"$MACPORTS_PREFIX"/lib|/opt/local/*|*qt-5.5.1*|*/qt-5.5.1-openssl3test/lib)
        install_name_tool -delete_rpath "$rpath" "$bin" 2>/dev/null || true
        ;;
    esac
  done < <(otool -l "$bin" | awk '/LC_RPATH/{in_rpath=1; next} in_rpath && /path /{print $2; in_rpath=0}')

  while IFS= read -r dep; do
    is_bundle_dependency "$dep" || continue
    install_name_tool -change "$dep" "$dep_target/$(basename "$dep")" "$bin" 2>/dev/null || true
  done < <(otool -L "$bin" | awk 'NR > 1 {print $1}')
}

if [[ -x /usr/libexec/PlistBuddy && -f "$PLIST" ]]; then
  APP_EXE_NAME=$(/usr/libexec/PlistBuddy -c "Print :CFBundleExecutable" "$PLIST" 2>/dev/null || true)
fi
APP_EXE_NAME=${APP_EXE_NAME:-DefcoinCoreNuLegacy}
APP_EXE="$APP/Contents/MacOS/$APP_EXE_NAME"
if [[ ! -x "$APP_EXE" ]]; then
  echo "App executable not found or not executable: $APP_EXE" >&2
  exit 1
fi
if [[ "$APP_EXE_NAME" == "DefcoinCoreNuLegacy" ]]; then
  copy_legacy_nu_assets
fi
copy_dependency_closure "$APP_EXE"
if [[ -n "$DEFCOIND" && -x "$NU_RESOURCES/bin/defcoind" ]]; then
  copy_dependency_closure "$NU_RESOURCES/bin/defcoind"
fi

rewrite_binary "$APP_EXE" "@executable_path/../Frameworks"
find "$FRAMEWORKS" -type f \( -name "*.dylib" -o -name "*.so" \) -print0 | while IFS= read -r -d '' file; do
  rewrite_binary "$file" "@loader_path"
done
find "$APP/Contents/PlugIns" -type f \( -name "*.dylib" -o -name "*.so" \) -print0 | while IFS= read -r -d '' file; do
  copy_dependency_closure "$file"
  rewrite_binary "$file" "@loader_path/../../Frameworks"
done

if [[ -n "$DEFCOIND" && -x "$NU_RESOURCES/bin/defcoind" ]]; then
  rewrite_binary "$NU_RESOURCES/bin/defcoind" "@executable_path/../../../Frameworks"
fi

find "$APP/Contents" -type f \( -perm +111 -o -name "*.dylib" -o -name "*.so" \) -print0 | while IFS= read -r -d '' file; do
  strip_macho_binary "$file"
done

generate_background() {
  local out=$1
  local logo="$ROOT_DIR/src/qt/nu/assets/brand/defcoin-nu-coin-stack-hires.png"
  if ! python3 -c 'import PIL' >/dev/null 2>&1; then
    cp "$logo" "$out"
    return 0
  fi
  python3 - "$logo" "$out" <<'PY'
import sys
from PIL import Image, ImageDraw, ImageFont

logo_path, out_path = sys.argv[1], sys.argv[2]
w, h = 640, 420
img = Image.new("RGBA", (w, h), "#f4f1ea")
draw = ImageDraw.Draw(img)
for y in range(h):
    shade = int(244 - y * 18 / h)
    draw.line([(0, y), (w, y)], fill=(shade, max(232, shade - 8), max(214, shade - 22)))

logo = Image.open(logo_path).convert("RGBA")
logo.thumbnail((210, 210), Image.LANCZOS)
img.alpha_composite(logo, (w // 2 - logo.width // 2, 74))

try:
    font_title = ImageFont.truetype("/System/Library/Fonts/Helvetica.dfont", 30)
    font_sub = ImageFont.truetype("/System/Library/Fonts/Helvetica.dfont", 15)
except Exception:
    font_title = font_sub = None

title = "Defcoin Core Nu"
sub = "Drag to Applications"
tw = draw.textlength(title, font=font_title) if hasattr(draw, "textlength") else draw.textsize(title, font=font_title)[0]
sw = draw.textlength(sub, font=font_sub) if hasattr(draw, "textlength") else draw.textsize(sub, font=font_sub)[0]
draw.text(((w - tw) / 2, 292), title, fill="#202124", font=font_title)
draw.text(((w - sw) / 2, 332), sub, fill="#5a5147", font=font_sub)
img.convert("RGB").save(out_path)
PY
}

if [[ "$MAKE_DMG" -eq 0 ]]; then
  echo "Staged app: $APP"
  exit 0
fi

mkdir -p "$DMG_ROOT/.background"
cp -R "$APP" "$DMG_ROOT/"
ln -s /Applications "$DMG_ROOT/Applications"

if [[ -n "$BACKGROUND" ]]; then
  cp "$BACKGROUND" "$DMG_ROOT/.background/background.png"
else
  generate_background "$DMG_ROOT/.background/background.png"
fi

rm -f "$DMG_RW" "$DMG_FINAL"
hdiutil create -volname "$VOLNAME" -srcfolder "$DMG_ROOT" -ov -fs HFS+ -format UDRW "$DMG_RW" >/dev/null

DEVICE=$(hdiutil attach -readwrite -noverify -noautoopen "$DMG_RW" | awk '/Apple_HFS/ {print $1; exit}')
MOUNT="/Volumes/$VOLNAME"
trap 'hdiutil detach "$MOUNT" >/dev/null 2>&1 || hdiutil detach "$DEVICE" >/dev/null 2>&1 || true' EXIT

osascript <<EOF
with timeout of 600 seconds
  tell application "Finder"
    tell disk "$VOLNAME"
      open
      set current view of container window to icon view
      set toolbar visible of container window to false
      set statusbar visible of container window to false
      set bounds of container window to {120, 120, 760, 540}
      set viewOptions to the icon view options of container window
      set arrangement of viewOptions to not arranged
      set icon size of viewOptions to 72
      set background picture of viewOptions to file ".background:background.png"
      set position of item "$PRODUCT_NAME.app" of container window to {150, 285}
      set position of item "Applications" of container window to {450, 285}
      close
      open
      update without registering applications
      delay 2
      close
    end tell
  end tell
end timeout
EOF

sync
hdiutil detach "$MOUNT" >/dev/null || hdiutil detach "$DEVICE" >/dev/null
trap - EXIT
hdiutil convert "$DMG_RW" -format UDZO -imagekey zlib-level=9 -o "$DMG_FINAL" >/dev/null
rm -f "$DMG_RW"

echo "Created DMG: $DMG_FINAL"
