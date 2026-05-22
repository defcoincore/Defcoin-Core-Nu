import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../Theme"

Dialog {
    id: root
    modal: true
    padding: NuTokens.spaceLg
    parent: Overlay.overlay
    property int dialogWidth: 560
    property real dragStartX: 0
    property real dragStartY: 0
    property real dragStartDialogX: 0
    property real dragStartDialogY: 0
    width: Math.min(root.dialogWidth, Overlay.overlay ? Overlay.overlay.width - NuTokens.spaceXl * 2 : root.dialogWidth)

    default property alias content: body.data
    property string acceptText: "Close"
    property string cancelText: "Cancel"
    property bool showCancel: true

    function centeredX() {
        return Overlay.overlay ? Math.round((Overlay.overlay.width - width) / 2) : 0
    }

    function centeredY() {
        return Overlay.overlay ? Math.round((Overlay.overlay.height - height) / 2) : 0
    }

    function clampToOverlay(value, size, limit) {
        if (!Overlay.overlay) return value
        return Math.max(NuTokens.spaceSm, Math.min(value, limit - size - NuTokens.spaceSm))
    }

    onOpened: {
        x = centeredX()
        y = centeredY()
    }

    background: Rectangle {
        color: NuTokens.panelBase
        border.color: NuTokens.lineStrong
        border.width: 1
        radius: NuTokens.radiusLarge
    }

    header: Label {
        text: root.title
        visible: root.title.length > 0
        color: NuTokens.textPrimary
        font.pixelSize: NuTokens.fontBodyLarge
        font.weight: Font.DemiBold
        padding: NuTokens.spaceLg
        bottomPadding: NuTokens.spaceSm

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.SizeAllCursor
            onPressed: {
                const point = parent.mapToItem(Overlay.overlay, mouse.x, mouse.y)
                root.dragStartX = point.x
                root.dragStartY = point.y
                root.dragStartDialogX = root.x
                root.dragStartDialogY = root.y
            }
            onPositionChanged: {
                if (!pressed || !Overlay.overlay) return
                const point = parent.mapToItem(Overlay.overlay, mouse.x, mouse.y)
                root.x = root.clampToOverlay(root.dragStartDialogX + point.x - root.dragStartX, root.width, Overlay.overlay.width)
                root.y = root.clampToOverlay(root.dragStartDialogY + point.y - root.dragStartY, root.height, Overlay.overlay.height)
            }
        }
    }

    contentItem: ColumnLayout {
        id: body
        spacing: NuTokens.spaceMd
    }

    footer: Item {
        implicitHeight: footerRow.implicitHeight + NuTokens.spaceLg + NuTokens.spaceSm

        RowLayout {
            id: footerRow
            anchors.fill: parent
            anchors.leftMargin: NuTokens.spaceLg
            anchors.rightMargin: NuTokens.spaceLg
            anchors.topMargin: NuTokens.spaceSm
            anchors.bottomMargin: NuTokens.spaceLg
            spacing: NuTokens.spaceMd

            Item { Layout.fillWidth: true }
            NuActionButton {
                visible: root.showCancel
                text: root.cancelText
                Layout.preferredWidth: 120
                onClicked: root.reject()
            }
            NuActionButton {
                text: root.acceptText
                primary: true
                Layout.preferredWidth: 140
                onClicked: root.accept()
            }
        }
    }
}
