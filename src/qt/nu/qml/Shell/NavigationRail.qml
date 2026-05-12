pragma ComponentBehavior: Bound
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../Theme"
import "../Components"

Rectangle {
    id: root
    color: NuTokens.inverseBase

    property string currentRoute: "home"
    signal routeRequested(string route)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: NuTokens.spaceLg
        spacing: NuTokens.spaceMd

        Label {
            text: "DEFCOIN\nCORE NU"
            color: NuTokens.textInverse
            font.pixelSize: 20
            font.bold: true
            lineHeight: 0.9
            Layout.bottomMargin: NuTokens.spaceLg
        }

        Repeater {
            model: [
                { route: "home", label: "Home", icon: "../../assets/icons/home.svg" },
                { route: "send", label: "Send", icon: "../../assets/icons/send.svg" },
                { route: "receive", label: "Receive", icon: "../../assets/icons/receive.svg" },
                { route: "activity", label: "Activity", icon: "../../assets/icons/activity.svg" },
                { route: "node", label: "Node", icon: "../../assets/icons/node.svg" },
                { route: "settings", label: "Settings", icon: "../../assets/icons/settings.svg" }
            ]

            NuNavButton {
                required property var modelData
                Layout.fillWidth: true
                text: modelData.label
                iconSource: modelData.icon
                selected: root.currentRoute === modelData.route
                onClicked: root.routeRequested(modelData.route)
            }
        }

        Item { Layout.fillHeight: true }

        Label {
            text: "CONNECTED\n4 PEERS"
            color: NuTokens.stateConnected
            font.pixelSize: 12
            font.letterSpacing: 1
        }
    }
}
