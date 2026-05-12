import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../Theme"
import "../Components"
import "../Bridge"

ColumnLayout {
    spacing: NuTokens.spaceLg

    Label {
        text: "Home"
        color: NuTokens.textPrimary
        font.pixelSize: 34
        font.bold: true
    }

    Rectangle {
        Layout.fillWidth: true
        implicitHeight: 180
        radius: NuTokens.radiusLarge
        color: NuTokens.panelBase
        border.color: NuTokens.lineSubtle

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: NuTokens.spaceXl
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
    }

    NuDataTable {
        Layout.fillWidth: true
        Layout.fillHeight: true
        emptyText: "Recent transactions will appear here."
    }
}
