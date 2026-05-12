import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../Theme"

RowLayout {
    id: root
    spacing: NuTokens.spaceSm

    property string label: ""
    property string value: ""

    Label {
        text: root.label
        color: NuTokens.textSecondary
        font.pixelSize: 13
    }

    Label {
        text: root.value
        color: NuTokens.textPrimary
        font.pixelSize: 14
        font.bold: true
    }
}
