#!/usr/bin/env bash

set -e

APP_NAME="SubtitleEditFree"
APP_ID="subtitleeditfree"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

BUILD_BIN="${ROOT_DIR}/build/${APP_NAME}"
ICON_SRC="${ROOT_DIR}/assets/icon.png"

INSTALL_DIR="/opt/${APP_ID}"
BIN_TARGET="${INSTALL_DIR}/${APP_NAME}"

ICON_DIR="/usr/share/icons/hicolor/256x256/apps"
ICON_TARGET="${ICON_DIR}/${APP_ID}.png"

DESKTOP_FILE="/usr/share/applications/${APP_ID}.desktop"

echo "Installing ${APP_NAME}..."

if [ ! -f "${BUILD_BIN}" ]; then
    echo "ERROR: executable not found: ${BUILD_BIN}"
    exit 1
fi

if [ ! -f "${ICON_SRC}" ]; then
    echo "ERROR: icon not found: ${ICON_SRC}"
    exit 1
fi

sudo mkdir -p "${INSTALL_DIR}"
sudo mkdir -p "${ICON_DIR}"

echo "Copy executable..."
sudo cp "${BUILD_BIN}" "${BIN_TARGET}"
sudo chmod +x "${BIN_TARGET}"

echo "Copy icon..."
sudo cp "${ICON_SRC}" "${ICON_TARGET}"

echo "Create desktop entry..."

sudo tee "${DESKTOP_FILE}" > /dev/null <<EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=${APP_NAME}
Comment=Subtitle editor for SRT and VTT files
Exec=${BIN_TARGET}
Icon=${APP_ID}
Terminal=false
Categories=Utility;AudioVideo;
StartupNotify=true
EOF

sudo chmod +x "${DESKTOP_FILE}"

echo "Update desktop database..."
sudo update-desktop-database || true

echo "Update icon cache..."
sudo gtk-update-icon-cache /usr/share/icons/hicolor || true

echo "Installation completed."