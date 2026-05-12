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
        title: "Home"
        detail: "A neutral command surface for balance, recent activity, and high-confidence wallet actions."
    }

    NuPanel {
        Layout.fillWidth: true
        implicitHeight: 180

        ColumnLayout {
            anchors.fill: parent
            spacing: NuTokens.spaceMd

            Label {
                text: "Total balance"
                color: NuTokens.textSecondary
                font.pixelSize: 14
            }
            Label {
                text: NuFakeService.totalBalance
                color: NuTokens.textPrimary
                font.pixelSize: 42
                font.bold: true
            }
            RowLayout {
                spacing: NuTokens.spaceXl
                NuMetricRow { label: "Available"; value: NuFakeService.availableBalance }
                NuMetricRow { label: "Pending"; value: NuFakeService.pendingBalance }
                NuMetricRow { label: "Immature"; value: NuFakeService.immatureBalance }
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: NuTokens.spaceLg

        NuActionButton {
            Layout.preferredWidth: 160
            text: "Receive"
            primary: true
        }
        NuActionButton {
            Layout.preferredWidth: 160
            text: "Send"
        }
        NuActionButton {
            Layout.preferredWidth: 180
            text: "Mask balances"
        }
        Item { Layout.fillWidth: true }
        NuStatusDot {
            label: "Connected: 4 peers and blockchain up to date"
            stateColor: NuTokens.stateConnected
        }
    }

    NuDataTable {
        Layout.fillWidth: true
        Layout.fillHeight: true
        columns: ["Date", "Type", "Label", "Amount"]
        rows: NuFakeService.recentTransactions
        emptyText: "Recent transactions will appear here."
    }
}
