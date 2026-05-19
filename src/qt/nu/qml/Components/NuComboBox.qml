pragma ComponentBehavior: Bound
import QtQuick 2.15
import QtQuick.Controls 2.15

import "../Theme"

ComboBox {
    id: root
    property string helpText: ""
    property bool suppressToolTip: false
    font.family: NuTokens.bodyFont
    font.pixelSize: NuTokens.fontBody
    leftPadding: NuTokens.spaceMd
    rightPadding: 36
    topPadding: NuTokens.spaceSm
    bottomPadding: NuTokens.spaceSm
    hoverEnabled: true
    activeFocusOnTab: true
    focusPolicy: Qt.StrongFocus

    Accessible.role: Accessible.ComboBox
    Accessible.name: displayText
    Accessible.description: helpText

    ToolTip.visible: !suppressToolTip && (hovered || activeFocus) && helpText.length > 0
    ToolTip.text: helpText
    ToolTip.delay: NuTokens.tooltipDelay
    ToolTip.timeout: NuTokens.tooltipTimeout

    onPressedChanged: if (pressed) suppressToolTip = true
    onHoveredChanged: if (!hovered) suppressToolTip = false
    onActiveFocusChanged: if (!activeFocus) suppressToolTip = false

    contentItem: Text {
        text: root.displayText
        color: NuTokens.textPrimary
        font: root.font
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    indicator: Text {
        anchors.right: parent.right
        anchors.rightMargin: NuTokens.spaceMd
        anchors.verticalCenter: parent.verticalCenter
        text: "v"
        color: NuTokens.textSecondary
        font.pixelSize: NuTokens.fontBody
    }

    background: Rectangle {
        color: NuTokens.panelBase
        border.color: root.activeFocus ? NuTokens.lineStrong : NuTokens.lineSubtle
        border.width: root.activeFocus ? 2 : 1
        radius: NuTokens.radiusSmall
    }

    delegate: ItemDelegate {
        id: delegateRoot
        required property string modelData
        width: root.width
        text: delegateRoot.modelData
        font.pixelSize: NuTokens.fontBody
        contentItem: Text {
            text: delegateRoot.modelData
            color: NuTokens.textPrimary
            font: root.font
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        background: Rectangle {
            color: delegateRoot.highlighted ? "#eeeeea" : NuTokens.panelBase
        }
    }

    popup: Popup {
        y: root.height + 2
        width: root.width
        implicitHeight: contentItem.implicitHeight
        padding: 1
        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: root.popup.visible ? root.delegateModel : null
            currentIndex: root.highlightedIndex
        }
        background: Rectangle {
            color: NuTokens.panelBase
            border.color: NuTokens.lineStrong
            radius: NuTokens.radiusSmall
        }
    }
}
