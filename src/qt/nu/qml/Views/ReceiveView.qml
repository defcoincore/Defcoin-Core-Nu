import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../Theme"
import "../Components"
import "../Bridge"

ColumnLayout {
    spacing: NuTokens.spaceLg

    Label { text: "Receive"; color: NuTokens.textPrimary; font.pixelSize: 34; font.bold: true }

    Rectangle {
        Layout.fillWidth: true
        implicitHeight: 300
        radius: NuTokens.radiusLarge
        color: NuTokens.panelBase
        border.color: NuTokens.lineSubtle

        RowLayout {
            anchors.fill: parent
            anchors.margins: NuTokens.spaceXl
            spacing: NuTokens.spaceXl

            Rectangle {
                Layout.preferredWidth: 180
                Layout.preferredHeight: 180
                color: "#ffffff"
                border.color: NuTokens.lineStrong
                Image {
                    anchors.centerIn: parent
                    width: 96
                    height: 96
                    source: "../../assets/icons/qr.svg"
                    fillMode: Image.PreserveAspectFit
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Label { text: "Payment address"; color: NuTokens.textSecondary }
                NuCopyField { Layout.fillWidth: true; value: NuFakeService.receiveAddress }
                NuActionButton { text: "Generate new request"; primary: true; Layout.preferredWidth: 220 }
            }
        }
    }

    NuDataTable {
        Layout.fillWidth: true
        Layout.fillHeight: true
        emptyText: "Requested payments will appear here."
    }
}
