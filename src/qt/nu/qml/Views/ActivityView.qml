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
        title: "Activity"
        detail: "Search, inspect, and export wallet history without changing wallet state."
    }

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
        columns: ["Date", "Type", "Label", "Amount"]
        rows: NuFakeService.recentTransactions
        emptyText: "Transactions will appear here."
    }
}
