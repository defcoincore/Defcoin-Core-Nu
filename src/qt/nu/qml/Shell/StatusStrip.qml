import QtQuick 2.15
import QtQuick.Layouts 1.15

import "../Theme"
import "../Components"
import "../Bridge"

Rectangle {
    id: root
    radius: NuTokens.radiusMedium
    color: NuTokens.panelBase
    border.color: NuTokens.lineSubtle
    implicitHeight: 56

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: NuTokens.spaceLg
        anchors.rightMargin: NuTokens.spaceLg
        spacing: NuTokens.spaceXl

        NuStatusDot {
            label: NuFakeService.networkState === "connected" ? "Network connected" : "Network isolated"
            stateColor: NuTokens.stateConnected
        }

        NuMetricRow {
            label: "Peers"
            value: NuFakeService.peerCount
        }

        NuMetricRow {
            label: "Block"
            value: NuFakeService.blockHeight
        }

        NuMetricRow {
            label: "Sync"
            value: NuFakeService.syncState
        }

        Item { Layout.fillWidth: true }

        NuStatusDot {
            label: NuFakeService.walletLocked ? "Wallet locked" : "Wallet unlocked"
            stateColor: NuFakeService.walletLocked ? NuTokens.stateInactive : NuTokens.stateConnected
        }
    }
}
