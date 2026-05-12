import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../Theme"

Button {
    id: root
    implicitHeight: 48
    font.pixelSize: 15
    font.bold: selected

    property string iconSource: ""
    property bool selected: false

    contentItem: RowLayout {
        spacing: NuTokens.spaceSm

        Image {
            Layout.preferredWidth: 20
            Layout.preferredHeight: 20
            source: root.iconSource
            visible: root.iconSource.length > 0
            fillMode: Image.PreserveAspectFit
        }

        Text {
            Layout.fillWidth: true
            text: root.text
            color: root.selected ? NuTokens.inverseBase : NuTokens.textInverse
            font: root.font
            verticalAlignment: Text.AlignVCenter
        }
    }

    background: Rectangle {
        radius: NuTokens.radiusMedium
        color: root.selected ? NuTokens.panelBase : "transparent"
        border.color: root.selected ? NuTokens.panelBase : NuTokens.lineStrong
    }
}
