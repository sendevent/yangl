#!/bin/bash
set -e

BUILD_DIR=./appimagebuild
QT_DIR=""

# Tool channel can be pinned by setting LINUXDEPLOY_CHANNEL to a release tag.
# Default "continuous" preserves existing behavior.
LINUXDEPLOY_CHANNEL="${LINUXDEPLOY_CHANNEL:-continuous}"
LINUXDEPLOY_PLUGIN_QT_CHANNEL="${LINUXDEPLOY_PLUGIN_QT_CHANNEL:-$LINUXDEPLOY_CHANNEL}"

LINUXDEPLOY_URL="https://github.com/linuxdeploy/linuxdeploy/releases/download/${LINUXDEPLOY_CHANNEL}/linuxdeploy-x86_64.AppImage"
LINUXDEPLOY_PLUGIN_QT_URL="https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/${LINUXDEPLOY_PLUGIN_QT_CHANNEL}/linuxdeploy-plugin-qt-x86_64.AppImage"

# Allow linuxdeploy AppImages to run without FUSE (required for CI environments)
export APPIMAGE_EXTRACT_AND_RUN=1

STARTED_AT=$(date +"%I:%M:%S.%N")

# --- Parse arguments ---
while [[ $# -gt 0 ]]; do
    case "$1" in
        --qt-dir)
            QT_DIR="$2"
            shift 2
            ;;
        --qt-dir=*)
            QT_DIR="${1#*=}"
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--qt-dir <path>]"
            exit 1
            ;;
    esac
done

# --- Download linuxdeploy tools if missing ---
download_tool() {
    local url="$1"
    local dest="$2"

    if [ -f "$dest" ]; then
        echo "Found:      $(basename "$dest")"
        return
    fi

    echo "Downloading $(basename "$dest")..."
    curl -fSL --retry 3 "$url" -o "$dest"
    chmod +x "$dest"
}

TOOLS_DIR="$BUILD_DIR/tools"
mkdir -p "$TOOLS_DIR"
download_tool "$LINUXDEPLOY_URL"            "$TOOLS_DIR/linuxdeploy-x86_64.AppImage"
download_tool "$LINUXDEPLOY_PLUGIN_QT_URL"  "$TOOLS_DIR/linuxdeploy-plugin-qt-x86_64.AppImage"

# --- Configure ---
echo ""
echo "Build directory: $BUILD_DIR"

CMAKE_ARGS=(
    -DCMAKE_BUILD_TYPE=Release
    -DCMAKE_INSTALL_PREFIX=/usr
)

if [ -n "$QT_DIR" ]; then
    echo "Qt directory:    $QT_DIR"
    CMAKE_ARGS+=(-DCMAKE_PREFIX_PATH="$QT_DIR")
    export QMAKE="$QT_DIR/bin/qmake"
else
    echo "Qt directory:    (system default)"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# From here on, all paths are relative to BUILD_DIR
rm -f CMakeCache.txt
rm -rf CMakeFiles
cmake .. "${CMAKE_ARGS[@]}" || { echo "Could not execute cmake"; exit 1; }

# --- Build ---
make -j"$(nproc)" || { echo "Could not execute make"; exit 2; }

# --- Install into AppDir ---
rm -rf AppDir
make install DESTDIR="$(pwd)/AppDir"

# --- Create AppImage ---
export PATH="$(pwd)/tools:$PATH"

# Ensure QML scanning finds runtime-loaded modules used by the map view.
readonly QML_SRC_GEO="$(realpath ../src/geo/qml)"
readonly QML_SRC_ROOT="$(realpath ../src)"
export QML_SOURCES_PATHS="${QML_SOURCES_PATHS:-${QML_SRC_GEO}:${QML_SRC_ROOT}}"

# Force-deploy plugin categories often missed in runtime-loaded map stacks.
# The map view relies on QtLocation providers such as "osm" from geoservices.
export EXTRA_QT_PLUGINS="${EXTRA_QT_PLUGINS:-geoservices,position,imageformats,iconengines,platformthemes,tls}"

echo ""
echo "Deploy diagnostics:"
echo "  linuxdeploy channel:          $LINUXDEPLOY_CHANNEL"
echo "  linuxdeploy-qt channel:       $LINUXDEPLOY_PLUGIN_QT_CHANNEL"
echo "  QML_SOURCES_PATHS:            $QML_SOURCES_PATHS"
echo "  EXTRA_QT_PLUGINS:             $EXTRA_QT_PLUGINS"

linuxdeploy-x86_64.AppImage \
    --appdir AppDir \
    --plugin qt \
    --output appimage \
    || { echo "Could not create AppImage"; exit 3; }

FINISHED_AT=$(date +"%I:%M:%S.%N")

echo ""
echo -e "Started:\t$STARTED_AT"
echo -e "Finished:\t$FINISHED_AT"
echo ""
echo "AppImage created:"
ls -lh *.AppImage
