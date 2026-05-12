import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../Theme"
import "../Components"

ColumnLayout {
    spacing: NuTokens.spaceLg

    NuPageHeader {
        Layout.fillWidth: true
        title: "Settings"
        detail: "Preferences stay grouped by intent. Risky changes should explain impact before they apply."
    }

    TabBar {
        id: tabs
        Layout.fillWidth: true
        TabButton { text: "Wallet" }
        TabButton { text: "Network" }
        TabButton { text: "Display" }
        TabButton { text: "Advanced" }
        TabButton { text: "About" }
    }

    StackLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        currentIndex: tabs.currentIndex

        NuDataTable {
            columns: ["Wallet setting", "Current value"]
            rows: [["Default currency unit", "DFC"], ["Coin control", "Disabled"], ["Mask balances", "Off"]]
        }

        NuDataTable {
            columns: ["Network setting", "Current value"]
            rows: [["Map port using UPnP", "Unavailable"], ["Proxy", "None"], ["Network state", "Connected"]]
        }

        NuDataTable {
            columns: ["Display setting", "Current value"]
            rows: [["Theme", "Neutral"], ["Language", "English"], ["Text resizing controls", "Shown"]]
        }

        NuDataTable {
            columns: ["Advanced setting", "Current value"]
            rows: [["Config file overrides", "None"], ["Prune block storage", "Off"], ["Script threads", "Automatic"]]
        }

        NuPanel {
            ColumnLayout {
                anchors.fill: parent
                spacing: NuTokens.spaceMd

                Label {
                    text: "Defcoin Core Nu v2026.1"
                    color: NuTokens.textPrimary
                    font.pixelSize: 22
                    font.bold: true
                }
                Label {
                    Layout.fillWidth: true
                    text: "A QML / Qt Quick shell for a future process-separated Defcoin Core wallet. Current scaffold uses fake data and leaves engine behavior unchanged."
                    color: NuTokens.textSecondary
                    wrapMode: Text.WordWrap
                }
            }
        }
    }
}
