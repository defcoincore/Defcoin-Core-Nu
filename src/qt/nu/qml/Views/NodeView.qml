import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../Theme"
import "../Components"

ColumnLayout {
    spacing: NuTokens.spaceLg

    Label { text: "Node"; color: NuTokens.textPrimary; font.pixelSize: 34; font.bold: true }

    TabBar {
        id: tabs
        Layout.fillWidth: true
        TabButton { text: "Status" }
        TabButton { text: "Peers" }
        TabButton { text: "Traffic" }
        TabButton { text: "Log" }
        TabButton { text: "Console" }
    }

    StackLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        currentIndex: tabs.currentIndex

        NuDataTable { emptyText: "Node status hydrates here." }
        NuDataTable { emptyText: "Peers hydrate here after the tab renders." }
        NuTimelineGraph {}
        NuDataTable { emptyText: "Current-launch debug log appears here." }
        NuDataTable { emptyText: "RPC console appears here." }
    }
}
