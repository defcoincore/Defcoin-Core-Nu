import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../Theme"

RowLayout {
    id: root
    spacing: NuTokens.spaceLg

    property string title: ""
    property string detail: ""

    ColumnLayout {
        Layout.fillWidth: true
        spacing: NuTokens.spaceXs

        Label {
            text: root.title
            color: NuTokens.textPrimary
            font.pixelSize: 34
            font.bold: true
        }

        Label {
            text: root.detail
            color: NuTokens.textSecondary
            font.pixelSize: 14
            visible: root.detail.length > 0
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }
}

