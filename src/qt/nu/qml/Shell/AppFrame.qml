pragma ComponentBehavior: Bound
import QtQuick 2.15
import QtQuick.Layouts 1.15

import "../Theme"
import "../Views"

Item {
    id: root

    property string currentRoute: "home"

    RowLayout {
        anchors.fill: parent
        spacing: 0

        NavigationRail {
            id: nav
            Layout.preferredWidth: 184
            Layout.fillHeight: true
            currentRoute: root.currentRoute
            onRouteRequested: (route) => root.currentRoute = route
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: NuTokens.backgroundBase

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: NuTokens.spaceXl
                spacing: NuTokens.spaceLg

                StatusStrip {
                    Layout.fillWidth: true
                }

                Loader {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    sourceComponent: {
                        switch (root.currentRoute) {
                        case "send": return sendView
                        case "receive": return receiveView
                        case "activity": return activityView
                        case "node": return nodeView
                        case "settings": return settingsView
                        default: return homeView
                        }
                    }
                }
            }
        }
    }

    Component { id: homeView; HomeView {} }
    Component { id: sendView; SendView {} }
    Component { id: receiveView; ReceiveView {} }
    Component { id: activityView; ActivityView {} }
    Component { id: nodeView; NodeView {} }
    Component { id: settingsView; SettingsView {} }
}
