import QtQuick 2.15
import QtQuick.Layouts 1.15
import Defcoin.Nu 1.0

import "../Theme"
import "../Components"

Rectangle {
    id: root
    radius: NuTokens.radiusMedium
    color: NuTokens.panelBase
    border.color: NuTokens.lineSubtle
    implicitHeight: 62

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: NuTokens.spaceLg
        anchors.rightMargin: NuTokens.spaceLg
        spacing: NuTokens.spaceXl

        NuStatusDot {
            label: NuService.networkState === "connected" ? "Network connected" : "Network isolated"
            stateColor: NuService.networkState === "connected" ? NuTokens.stateConnected : NuTokens.stateError
        }

        NuMetricRow {
            label: "Peers"
            value: NuService.peerCount
        }

        NuMetricRow {
            label: "Block"
            value: NuService.blockHeight
        }

        NuMetricRow {
            label: "Sync"
            value: NuService.syncState
        }

        Item { Layout.fillWidth: true }

        NuStatusDot {
            label: NuService.walletLocked ? "Wallet locked" : "Wallet unlocked"
            stateColor: NuService.walletLocked ? NuTokens.stateInactive : NuTokens.stateConnected
        }
    }
}
