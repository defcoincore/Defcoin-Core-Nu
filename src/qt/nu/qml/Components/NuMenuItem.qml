import QtQuick 2.15
import QtQuick.Controls 2.15

import "../Theme"

MenuItem {
    id: control
    implicitWidth: Math.max(label.implicitWidth + 34, 236)
    implicitHeight: Math.max(label.implicitHeight + 14, 34)

    contentItem: Text {
        id: label
        text: control.text
        color: !control.enabled ? NuTokens.textMuted : (control.highlighted ? NuTokens.textInverse : NuTokens.textPrimary)
        font.family: NuTokens.bodyFont
        font.pixelSize: NuTokens.fontBody
        elide: Text.ElideNone
        verticalAlignment: Text.AlignVCenter
        leftPadding: 10
        rightPadding: 10
    }

    background: Rectangle {
        color: control.highlighted && control.enabled ? NuTokens.lineStrong : NuTokens.panelBase
    }
}
