#!/bin/bash
# Builds Heresy-<version>-mac.pkg from the Release artefacts.
# Signs and notarizes when Developer ID identities are available:
#   APP_IDENTITY        e.g. "Developer ID Application: Name (TEAMID)"
#   INSTALLER_IDENTITY  e.g. "Developer ID Installer: Name (TEAMID)"
#   APPLE_ID, APPLE_APP_PASSWORD, APPLE_TEAM_ID  (for notarization)
# Without them the bundles are ad-hoc signed and the .pkg is unsigned.
set -euo pipefail

VERSION="$1"
ART="$2"                      # build/Heresy_artefacts/Release
OUT="${3:-dist}"
HERE="$(cd "$(dirname "$0")" && pwd)"
WORK="$(mktemp -d)"
mkdir -p "$OUT"

stage() {   # stage <bundle> <name>
  mkdir -p "$WORK/$2"
  ditto "$1" "$WORK/$2/$(basename "$1")"
}
stage "$ART/AU/Heresy.component" au
stage "$ART/VST3/Heresy.vst3" vst3
stage "$ART/Standalone/Heresy.app" app

sign() {
  if [[ -n "${APP_IDENTITY:-}" ]]; then
    codesign --force --deep --options runtime --timestamp -s "$APP_IDENTITY" "$1"
  else
    codesign --force --deep -s - "$1"
  fi
}
sign "$WORK/au/Heresy.component"
sign "$WORK/vst3/Heresy.vst3"
sign "$WORK/app/Heresy.app"

component() {   # component <dir> <id> <install-location> [scripts]
  local plist="$WORK/$1.plist"
  pkgbuild --analyze --root "$WORK/$1" "$plist"
  # never let Installer "relocate" into an older copy found elsewhere on disk
  local n=0
  while /usr/libexec/PlistBuddy -c "Print :$n" "$plist" >/dev/null 2>&1; do
    /usr/libexec/PlistBuddy -c "Set :$n:BundleIsRelocatable false" "$plist" 2>/dev/null \
      || /usr/libexec/PlistBuddy -c "Add :$n:BundleIsRelocatable bool false" "$plist"
    n=$((n + 1))
  done
  local args=(--root "$WORK/$1" --component-plist "$plist" --identifier "$2" --version "$VERSION" --install-location "$3")
  [[ -n "${4:-}" ]] && args+=(--scripts "$4")
  pkgbuild "${args[@]}" "$WORK/heresy-$1.pkg"
}
component au   com.castledangermusic.heresy.au   "/Library/Audio/Plug-Ins/Components" "$HERE/scripts-au"
component vst3 com.castledangermusic.heresy.vst3 "/Library/Audio/Plug-Ins/VST3"
component app  com.castledangermusic.heresy.app  "/Applications"

sed "s/@VERSION@/$VERSION/g" "$HERE/distribution.xml" > "$WORK/distribution.xml"
PKG="$OUT/Heresy-$VERSION-mac.pkg"
args=(--distribution "$WORK/distribution.xml" --package-path "$WORK" --resources "$HERE/resources")
[[ -n "${INSTALLER_IDENTITY:-}" ]] && args+=(--sign "$INSTALLER_IDENTITY" --timestamp)
productbuild "${args[@]}" "$PKG"

if [[ -n "${INSTALLER_IDENTITY:-}" && -n "${APPLE_ID:-}" && -n "${APPLE_APP_PASSWORD:-}" && -n "${APPLE_TEAM_ID:-}" ]]; then
  xcrun notarytool submit "$PKG" --apple-id "$APPLE_ID" --password "$APPLE_APP_PASSWORD" --team-id "$APPLE_TEAM_ID" --wait
  xcrun stapler staple "$PKG"
  echo "Signed and notarized: $PKG"
else
  echo "Built unsigned installer: $PKG"
fi
rm -rf "$WORK"
