import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Defcoin.Nu 1.0

import "../Theme"
import "../Components"

ColumnLayout {
    signal navigateRequested(string route)

    spacing: NuTokens.spaceLg

    NuPageHeader {
        Layout.fillWidth: true
        title: "Home"
        detail: ""
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
                font.pixelSize: NuTokens.fontBody
            }
            Label {
                text: NuService.totalBalance
                color: NuTokens.textPrimary
                font.pixelSize: NuTokens.fontHero
                font.weight: Font.DemiBold
            }
            RowLayout {
                spacing: NuTokens.spaceXl
                NuMetricRow { label: "Available"; value: NuService.availableBalance }
                NuMetricRow { label: "Pending"; value: NuService.pendingBalance }
                NuMetricRow { label: "Immature"; value: NuService.immatureBalance }
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
            helpText: "Create a payment request or copy a receiving address."
            onClicked: navigateRequested("receive")
        }
        NuActionButton {
            Layout.preferredWidth: 160
            text: "Send"
            helpText: "Compose a payment and review it before signing."
            onClicked: navigateRequested("send")
        }
        NuActionButton {
            Layout.preferredWidth: 180
            text: "Mask balances"
            helpText: "Hide or show balance values on this screen."
            onClicked: NuService.maskBalances = !NuService.maskBalances
        }
        Item { Layout.fillWidth: true }
    }

    NuDataTable {
        Layout.fillWidth: true
        Layout.fillHeight: true
        tableId: "homeRecentTransactions"
        columns: ["", "Date", "Type", "Label", "Amount"]
        columnTypes: ["action", "date", "text", "text", "amount"]
        columnWeights: [0.1, 1.1, 0.65, 2.8, 1.25]
        columnMinimums: [44, 150, 70, 180, 130]
        columnMaximums: [44, 180, 110, 520, 180]
        rows: NuService.recentTransactions
        emptyText: "Recent transactions will appear here."
        onRowActivated: (row) => NuService.requestTransactionDetails(row && row.meta ? row.meta.txid : "")
    }
}
