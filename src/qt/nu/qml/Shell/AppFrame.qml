pragma ComponentBehavior: Bound
import QtQuick 2.15
import QtQuick.Layouts 1.15

import "../Theme"
import "../Views"

Item {
    id: root

    property string currentRoute: "home"

    function routeIndex(route) {
        switch (route) {
        case "send": return 1
        case "receive": return 2
        case "activity": return 3
        case "node": return 4
        case "settings": return 5
        default: return 0
        }
    }

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

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    currentIndex: root.routeIndex(root.currentRoute)

                    HomeView {}
                    SendView {}
                    ReceiveView {}
                    ActivityView {}
                    NodeView {}
                    SettingsView {}
                }
            }
        }
    }
}
