import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../Theme"
import "../Components"

ColumnLayout {
    spacing: NuTokens.spaceLg

    Label { text: "Activity"; color: NuTokens.textPrimary; font.pixelSize: 34; font.bold: true }

    RowLayout {
        Layout.fillWidth: true
        ComboBox { Layout.preferredWidth: 180; model: ["All dates", "Today", "This week", "This month"] }
        ComboBox { Layout.preferredWidth: 180; model: ["All types", "Sent", "Received", "Mined"] }
        TextField { Layout.fillWidth: true; placeholderText: "Search address, label, or transaction ID" }
        NuActionButton { text: "Export"; Layout.preferredWidth: 120 }
    }

    NuDataTable {
        Layout.fillWidth: true
        Layout.fillHeight: true
        emptyText: "Transactions will appear here."
    }
}
