import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../Theme"
import "../Components"

ColumnLayout {
    spacing: NuTokens.spaceLg

    Label { text: "Settings"; color: NuTokens.textPrimary; font.pixelSize: 34; font.bold: true }

    TabBar {
        id: tabs
        Layout.fillWidth: true
        TabButton { text: "Wallet" }
        TabButton { text: "Network" }
        TabButton { text: "Display" }
        TabButton { text: "Advanced" }
        TabButton { text: "About" }
    }

    NuDataTable {
        Layout.fillWidth: true
        Layout.fillHeight: true
        emptyText: "Settings controls will appear here."
    }
}
