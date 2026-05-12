import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../Theme"
import "../Components"

ColumnLayout {
    spacing: NuTokens.spaceLg

    Label { text: "Send"; color: NuTokens.textPrimary; font.pixelSize: 34; font.bold: true }

    Rectangle {
        Layout.fillWidth: true
        radius: NuTokens.radiusLarge
        color: NuTokens.panelBase
        border.color: NuTokens.lineSubtle
        implicitHeight: 280

        GridLayout {
            anchors.fill: parent
            anchors.margins: NuTokens.spaceXl
            columns: 2
            rowSpacing: NuTokens.spaceLg
            columnSpacing: NuTokens.spaceLg

            Label { text: "Recipient"; color: NuTokens.textSecondary }
            TextField { Layout.fillWidth: true; placeholderText: "Defcoin address or URI" }

            Label { text: "Amount"; color: NuTokens.textSecondary }
            TextField { Layout.fillWidth: true; placeholderText: "0.00000000 DFC" }

            Label { text: "Fee"; color: NuTokens.textSecondary }
            ComboBox { Layout.fillWidth: true; model: ["Recommended", "Custom"] }
        }
    }

    RowLayout {
        NuActionButton { text: "Advanced"; Layout.preferredWidth: 140 }
        Item { Layout.fillWidth: true }
        NuActionButton { text: "Review transaction"; primary: true; Layout.preferredWidth: 220 }
    }
}
