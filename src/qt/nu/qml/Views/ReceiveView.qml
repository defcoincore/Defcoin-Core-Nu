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
        title: "Receive"
        detail: "Generate requests, copy addresses, and keep payment details readable before sharing."
    }

    NuPanel {
        Layout.fillWidth: true
        implicitHeight: 300

        RowLayout {
            anchors.fill: parent
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
        columns: ["Date", "Label", "Address", "Amount"]
        rows: [["05/12/26", "test", NuFakeService.receiveAddress, "1.00000000 DFC"]]
        emptyText: "Requested payments will appear here."
    }
}
