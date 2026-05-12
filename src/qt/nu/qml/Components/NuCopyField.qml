import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../Theme"

Rectangle {
    id: root
    radius: NuTokens.radiusMedium
    color: NuTokens.panelBase
    border.color: NuTokens.lineSubtle
    implicitHeight: 44

    property string value: ""
    signal copyRequested(string value)

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: NuTokens.spaceMd
        spacing: NuTokens.spaceSm

        Label {
            Layout.fillWidth: true
            text: root.value
            elide: Text.ElideMiddle
            color: NuTokens.textPrimary
            font.family: "monospace"
            font.pixelSize: 14
        }

        Button {
            text: "Copy"
            onClicked: root.copyRequested(root.value)
        }
    }
}
