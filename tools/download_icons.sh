#!/bin/bash

# Create icons directory if it doesn't exist
mkdir -p resources/icons

# Download icons from Qt's standard icon set
ICONS=(
    "document-new:new"
    "document-open:open"
    "document-save:save"
    "document-save-as:save-as"
    "edit-undo:undo"
    "edit-redo:redo"
    "edit-cut:cut"
    "edit-copy:copy"
    "edit-paste:paste"
    "edit-find:find"
    "edit-find-replace:replace"
    "preferences-system:settings"
    "zoom-in:zoom-in"
    "zoom-out:zoom-out"
    "zoom-original:zoom-reset"
    "view-split-left-right:split-horizontal"
    "view-split-top-bottom:split-vertical"
)

for ICON in "${ICONS[@]}"; do
    QT_ICON="${ICON%%:*}"
    LOCAL_NAME="${ICON#*:}"
    echo "Downloading $QT_ICON as $LOCAL_NAME.png"
    wget -q "https://raw.githubusercontent.com/KDE/oxygen-icons/master/32x32/actions/$QT_ICON.png" -O "resources/icons/$LOCAL_NAME.png"
done 