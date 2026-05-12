pragma ComponentBehavior: Bound
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../Theme"

Rectangle {
    id: root
    radius: NuTokens.radiusMedium
    color: NuTokens.panelBase
    border.color: NuTokens.lineSubtle

    property string emptyText: "No rows."
    property var columns: []
    property var rows: []
    property var columnWeights: []
    property bool compact: false

    function columnWeight(index) {
        if (index >= 0 && index < columnWeights.length) {
            return columnWeights[index]
        }
        return 1
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.compact ? NuTokens.spaceSm : NuTokens.spaceLg
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            spacing: 0
            visible: root.columns.length > 0

            Repeater {
                model: root.columns

                Rectangle {
                    id: headerCell
                    required property int index
                    required property string modelData
                    Layout.fillWidth: true
                    Layout.preferredWidth: root.columnWeight(index) * 100
                    implicitHeight: root.compact ? 30 : 38
                    color: NuTokens.backgroundBase
                    border.color: NuTokens.lineSubtle

                    Label {
                        anchors.fill: parent
                        anchors.margins: NuTokens.spaceSm
                        text: headerCell.modelData
                        color: NuTokens.textPrimary
                        font.pixelSize: root.compact ? 12 : 13
                        font.bold: true
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }
                }
            }
        }

        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: root.rows
            visible: root.rows.length > 0

            delegate: RowLayout {
                id: row
                required property var modelData
                width: ListView.view.width
                height: root.compact ? 28 : 34
                spacing: 0

                Repeater {
                    model: row.modelData

                    Rectangle {
                        id: bodyCell
                        required property int index
                        required property string modelData
                        Layout.fillWidth: true
                        Layout.preferredWidth: root.columnWeight(index) * 100
                        implicitHeight: root.compact ? 28 : 34
                        color: "transparent"
                        border.color: NuTokens.lineSubtle

                        Label {
                            anchors.fill: parent
                            anchors.margins: NuTokens.spaceSm
                            text: bodyCell.modelData
                            color: NuTokens.textPrimary
                            font.pixelSize: root.compact ? 12 : 13
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }

        Label {
            text: root.emptyText
            color: NuTokens.textSecondary
            font.pixelSize: 14
            Layout.alignment: Qt.AlignCenter
            visible: root.rows.length === 0
        }
    }
}
