import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../Theme"

RowLayout {
    id: root
    spacing: NuTokens.spaceSm

    property string label: ""
    property color stateColor: NuTokens.stateInactive

    Rectangle {
        implicitWidth: 10
        implicitHeight: 10
        radius: 5
        color: root.stateColor
        Layout.alignment: Qt.AlignVCenter
    }

    Label {
        text: root.label
        color: NuTokens.textPrimary
        font.pixelSize: NuTokens.fontBody
    }
}
