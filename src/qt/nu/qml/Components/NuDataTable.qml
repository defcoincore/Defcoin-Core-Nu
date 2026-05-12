import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../Theme"

Rectangle {
    id: root
    radius: NuTokens.radiusMedium
    color: NuTokens.panelBase
    border.color: NuTokens.lineSubtle

    property string emptyText: "No rows."

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: NuTokens.spaceLg

        Label {
            text: root.emptyText
            color: NuTokens.textSecondary
            font.pixelSize: 14
            Layout.alignment: Qt.AlignCenter
        }
    }
}
