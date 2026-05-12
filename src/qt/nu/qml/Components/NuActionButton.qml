import QtQuick 2.15
import QtQuick.Controls 2.15

import "../Theme"

Button {
    id: root
    implicitHeight: 42
    font.pixelSize: 15
    font.bold: true

    property bool primary: false
    property bool danger: false

    contentItem: Text {
        text: root.text
        color: root.primary || root.danger ? NuTokens.textInverse : NuTokens.textPrimary
        font: root.font
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        radius: NuTokens.radiusMedium
        color: root.danger ? NuTokens.stateError : (root.primary ? NuTokens.inverseBase : NuTokens.panelBase)
        border.color: root.danger ? NuTokens.stateError : NuTokens.lineStrong
    }
}
