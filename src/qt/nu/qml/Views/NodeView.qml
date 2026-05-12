import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../Theme"
import "../Components"
import "../Bridge"

ColumnLayout {
    spacing: NuTokens.spaceLg

    NuPageHeader {
        Layout.fillWidth: true
        title: "Node"
        detail: "Transparent node status, peers, traffic, and logs. Heavy peer and graph work should hydrate after this shell renders."
    }

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

        NuDataTable {
            columns: ["Metric", "Value"]
            rows: NuFakeService.nodeMetrics
            emptyText: "Node status hydrates here."
        }

        ColumnLayout {
            spacing: NuTokens.spaceMd

            NuDataTable {
                Layout.fillWidth: true
                Layout.fillHeight: true
                compact: true
                columns: ["Node ID", "Direction", "IP Address:Port", "Ping", "Sent", "Rec'd", "User Agent"]
                columnWeights: [0.65, 0.8, 5.6, 0.7, 0.65, 0.65, 1.3]
                rows: NuFakeService.peers
                emptyText: "Peers hydrate here after the tab renders."
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: NuTokens.spaceMd

                Label {
                    Layout.fillWidth: true
                    text: NuFakeService.peerCount + " peers"
                    color: NuTokens.textSecondary
                    font.pixelSize: 13
                }

                NuActionButton { text: "Copy Row"; Layout.preferredWidth: 112 }
                NuActionButton { text: "Ping"; Layout.preferredWidth: 96 }
                NuActionButton { text: "Trace Route"; Layout.preferredWidth: 128 }
            }
        }

        ColumnLayout {
            spacing: NuTokens.spaceMd

            NuTimelineGraph {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: NuTokens.spaceMd

                Label { text: "Window"; color: NuTokens.textSecondary }
                Slider { Layout.fillWidth: true; from: 1; to: 100; value: 25 }
                Label { text: "15m"; color: NuTokens.textPrimary }
                NuActionButton { text: "Pause"; Layout.preferredWidth: 92 }
                NuActionButton { text: "Export CSV"; Layout.preferredWidth: 120 }
            }
        }

        NuDataTable {
            columns: ["Current launch debug log"]
            rows: NuFakeService.logLines.map(function(line) { return [line] })
            emptyText: "Current-launch debug log appears here."
        }

        NuPanel {
            ColumnLayout {
                anchors.fill: parent
                spacing: NuTokens.spaceMd

                Label {
                    text: "Console"
                    color: NuTokens.textPrimary
                    font.pixelSize: 18
                    font.bold: true
                }

                TextArea {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    readOnly: true
                    text: "RPC console bridge is intentionally disabled in this fake service build."
                    color: NuTokens.textPrimary
                    background: Rectangle { color: NuTokens.backgroundBase; border.color: NuTokens.lineSubtle }
                }
            }
        }
    }
}
