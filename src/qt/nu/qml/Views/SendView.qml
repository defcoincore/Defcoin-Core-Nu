import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../Theme"
import "../Components"

ColumnLayout {
    spacing: NuTokens.spaceLg

    NuPageHeader {
        Layout.fillWidth: true
        title: "Send"
        detail: "Compose first, review second, sign last. Irreversible actions stay behind explicit confirmation."
    }

    NuPanel {
        Layout.fillWidth: true
        implicitHeight: 280

        GridLayout {
            anchors.fill: parent
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
