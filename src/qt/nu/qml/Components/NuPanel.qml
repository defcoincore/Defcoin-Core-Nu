import QtQuick 2.15

import "../Theme"

Rectangle {
    id: root
    radius: NuTokens.radiusLarge
    color: NuTokens.panelBase
    border.color: NuTokens.lineSubtle

    default property alias content: body.data
    property int padding: NuTokens.spaceLg

    Item {
        id: body
        anchors.fill: parent
        anchors.margins: root.padding
    }
}
