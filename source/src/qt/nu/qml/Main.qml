import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import Defcoin.Nu 1.0

import "Shell"
import "Theme"
import "Components"

ApplicationWindow {
    id: root
    visible: true
    width: 1280
    height: 820
    minimumWidth: 1080
    minimumHeight: 680
    title: "Defcoin Core Nu"
    color: NuTokens.backgroundBase
    property alias currentRoute: frame.currentRoute
    property alias nodeInitialTab: frame.nodeInitialTab
    property alias peerInitialView: frame.peerInitialView
    property string buildVersion: NuBuildVersion
    property string buildId: NuBuildId
    property string buildTimestamp: NuBuildTimestamp
    property string gitCommit: NuGitCommit
    property bool helpEnabled: NuHelpEnabled
    readonly property string releaseCodeName: "Core Memories"
    readonly property string trademarkNotice: "The DEFCON and 'Smiling Jack' wordmarks, design marks, and associated logos are registered trademarks of Def Con Communications, Inc. (Canadian Reg. No. TMA917353; US Reg. No. 4582595). Coin image utilized under a long-standing non-commercial permission agreement. This project is an independent creation and is not affiliated with, sponsored by, or endorsed by Def Con Communications, Inc. Def Con Communications, Inc. reserves all rights to the exclusive use of its registered trademarks and design marks."
    palette.window: NuTokens.panelBase
    palette.base: NuTokens.panelBase
    palette.alternateBase: NuTokens.backgroundBase
    palette.text: NuTokens.textPrimary
    palette.windowText: NuTokens.textPrimary
    palette.button: NuTokens.panelBase
    palette.buttonText: NuTokens.textPrimary
    palette.highlight: NuTokens.lineStrong
    palette.highlightedText: NuTokens.textInverse

    function openPreferences() {
        frame.currentRoute = "settings"
    }

    function openNode() {
        frame.currentRoute = "node"
    }

    function basicAboutText() {
        return "Defcoin Core Nu v" + root.buildVersion + " - " + root.releaseCodeName + ". New Qt Quick interface build. Backend derived from Litecoin Core v0.21.5.5 with Defcoin consensus and network parameters. Verify recipients, amounts, and backups carefully before use. © 2014-2026 The Defcoin Core developers. © 2011-2026 The Litecoin Core developers. © 2009-2021 The Bitcoin Core developers."
    }

    function showHelpPage(windowTitle, page) {
        if (!root.helpEnabled) {
            messageDialog.title = "Help not included"
            messageDialog.text = "Build notes are available from About in this build."
            messageDialog.open()
            return
        }
        if (Qt.platform.os === "osx") {
            NuService.openHelpManual(page)
        } else {
            root.openHelpWindow(windowTitle, NuService.helpManualHtml(page), "")
        }
    }

    function nuBuildDetailsHtml() {
        return "<h1>Defcoin Core Nu v" + root.buildVersion + "</h1>"
             + "<p><b>Codename:</b> Core Memories</p>"
             + "<p><b>Status:</b> Nu is a new Qt Quick interface for Defcoin Core. It keeps the Litecoin Core v0.21.5.5-derived backend, Defcoin consensus parameters, and the existing Defcoin data directory, while introducing a desktop shell inspired by Nothing Company product design and the Bitcoin Design Community.</p>"
             + "<h2>Build metadata</h2>"
             + "<ul>"
             + "<li><b>Release:</b> " + root.buildVersion + "</li>"
             + "<li><b>Codename:</b> Core Memories</li>"
             + "<li><b>Build ID:</b> " + (root.buildId.length > 0 ? root.buildId : "not recorded in this bundle") + "</li>"
             + "<li><b>Build time:</b> " + (root.buildTimestamp.length > 0 ? root.buildTimestamp : "not recorded in this bundle") + "</li>"
             + "<li><b>Source:</b> " + (root.gitCommit.length > 0 ? root.gitCommit : "not recorded in this bundle") + "</li>"
             + "</ul>"
             + "<h2>What went into Nu</h2>"
             + "<ul>"
             + "<li>Qt Quick interface organized around Home, Send, Receive, Activity, Diagnostics, and Settings.</li>"
             + "<li>Visual system, copy, and interaction patterns are guided by Nothing-style restraint and Bitcoin Design Community wallet usability patterns.</li>"
             + "<li>Bundled backend autostart, RPC connection handling, launch diagnostics, and current-launch log viewing.</li>"
             + "<li>Dual-magic migration support for legacy <code>fbc0b6db</code> and Defcoin-specific <code>defc014e</code> P2P message headers.</li>"
             + "<li>Peer pollution filtering now happens at both the peer and address-relay layers: non-Defcoin-prefixed peers are disconnected before their address tables are accepted, and unvalidated relayed mainnet addresses are only stored when they advertise Defcoin service ports.</li>"
             + "<li>The address filter is endpoint-specific, not IP-wide. If the same host runs Litecoin Core on one port and Defcoin Core on another, Nu keeps the Defcoin endpoint eligible and can replace older same-IP non-Defcoin ports in addrman. Defcoin nodes on non-standard ports can still communicate and be retained after completing an actual Defcoin handshake.</li>"
             + "<li>Peer inspection with simple and detailed views, including actual per-peer magic bytes where reported by the backend.</li>"
             + "<li>Wallet basics including receive requests, transaction inspection, PSBT tools, message signing, wallet backup, encryption, and optional third-party explorer links.</li>"
             + "</ul>"
             + "<h2>How Nu differs from Defcoin 1.0.0 and 1.0.1</h2>"
             + "<ul>"
             + "<li>Nu keeps the Defcoin chain and wallet data directory shared, but replaces the classic Qt wallet surface with a new Qt Quick shell.</li>"
             + "<li>Nu includes explicit peer filtering and magic-byte migration controls intended to reduce Litecoin-family peer pollution.</li>"
             + "<li>Nu adds clearer diagnostics for backend startup, RPC readiness, network state, peers, logs, and traffic.</li>"
             + "<li>Nu runs a bundled <code>defcoind</code> backend as a managed child process, instead of keeping node, wallet, and UI work inside one classic Qt wallet process.</li>"
             + "<li>Nu's newer part is the desktop interface and packaging model; the chain rules, wallet data directory, and Litecoin-derived backend remain the Defcoin Core compatibility baseline.</li>"
             + "</ul>"
             + "<h2>Source</h2>"
             + "<p>Source can be found at <a href=\"https://github.com/DefcoinCore/Defcoin-Core-Nu\">https://github.com/DefcoinCore/Defcoin-Core-Nu</a> as Defcoin Core Nu, a Defcoin fork derived from Litecoin Core.</p>"
    }

    function openNuBuildDetails() {
        root.openHelpWindow("Defcoin Core Nu Build Notes", root.nuBuildDetailsHtml(), "")
    }

    function openDetailedAbout() {
        if (root.helpEnabled)
            root.showHelpPage("About Defcoin Core Nu", "details.html")
        else
            root.openNuBuildDetails()
    }

    function openHelpManual() {
        root.showHelpPage("Defcoin Core Nu Help Manual", "index.html")
    }

    function openAboutSummary() {
        aboutDialog.open()
    }

    function openHelpWindow(windowTitle, html, anchor) {
        helpWindow.title = windowTitle
        helpText.text = html
        helpWindow.show()
        helpWindow.raise()
        helpWindow.requestActivate()
        if (anchor && anchor.length > 0) {
            Qt.callLater(function() { root.scrollHelpToAnchor(anchor) })
        } else {
            Qt.callLater(function() { helpScroll.ScrollBar.vertical.position = 0 })
        }
    }

    function scrollHelpToAnchor(anchor) {
        var anchors = {
            "home": "Home (Overview)",
            "send": "Send",
            "receive": "Receive",
            "activity": "Activity (Transactions)",
            "diagnostics": "Diagnostics",
            "settings": "Settings",
            "psbt": "Partially signed transactions"
        }
        var title = anchors[anchor] || anchor
        var plain = helpText.getText(0, helpText.length)
        var pos = plain.indexOf(title)
        if (pos < 0)
            pos = plain.toLowerCase().indexOf(String(title).toLowerCase())
        if (pos < 0) return
        helpText.cursorPosition = pos
        var rect = helpText.positionToRectangle(pos)
        if (helpScroll.contentItem && helpScroll.contentItem.contentY !== undefined) {
            helpScroll.contentItem.contentY = Math.max(0, rect.y - 24)
        } else {
            helpScroll.ScrollBar.vertical.position = Math.max(0, Math.min(1, pos / Math.max(1, plain.length)))
        }
    }

    function openHelpLink(link) {
        var target = String(link)
        if (target.indexOf("http://") === 0 || target.indexOf("https://") === 0) {
            Qt.openUrlExternally(target)
            return
        }
        if (target.charAt(0) === "#") {
            root.scrollHelpToAnchor(target.substring(1))
            return
        }
        var clean = target.split("#")[0]
        var hashIndex = target.indexOf("#")
        var anchor = hashIndex >= 0 ? target.substring(hashIndex + 1) : ""
        if (clean.length === 0)
            clean = "index.html"
        if (clean.indexOf(".html") >= 0) {
            root.openHelpWindow(clean === "details.html" ? "About Defcoin Core Nu" : "Defcoin Core Nu Help Manual",
                                NuService.helpManualHtml(clean),
                                anchor)
        }
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("File")
            NuMenuItem { text: qsTr("Open URI..."); onTriggered: openUriDialog.open() }
            MenuSeparator {}
            NuMenuItem { text: qsTr("Backup Wallet..."); onTriggered: NuService.backupWallet() }
            MenuSeparator {}
            NuMenuItem { text: qsTr("Quit"); onTriggered: Qt.quit() }
        }

        Menu {
            title: qsTr("Settings")
            NuMenuItem { text: qsTr("Preferences..."); onTriggered: root.openPreferences() }
        }

        Menu {
            title: qsTr("Window")
            NuMenuItem { text: qsTr("Home"); onTriggered: frame.currentRoute = "home" }
            NuMenuItem { text: qsTr("Send"); onTriggered: frame.currentRoute = "send" }
            NuMenuItem { text: qsTr("Receive"); onTriggered: frame.currentRoute = "receive" }
            NuMenuItem { text: qsTr("Activity"); onTriggered: frame.currentRoute = "activity" }
            MenuSeparator {}
            NuMenuItem { text: qsTr("Diagnostics"); onTriggered: root.openNode() }
        }

        Menu {
            title: root.helpEnabled ? qsTr("Help") : qsTr("About")
            NuMenuItem {
                text: root.helpEnabled ? qsTr("Help Manual") : qsTr("About Defcoin Core Nu")
                onTriggered: {
                    if (root.helpEnabled) root.openHelpManual()
                    else aboutDialog.open()
                }
            }
            NuMenuItem {
                text: qsTr("About Defcoin Core Nu")
                visible: root.helpEnabled
                height: root.helpEnabled ? implicitHeight : 0
                onTriggered: aboutDialog.open()
            }
        }
    }

    AppFrame {
        id: frame
        anchors.fill: parent
        onAboutRequested: root.openAboutSummary()
    }

    Connections {
        target: NuService
        function onUserMessage(title, message) {
            messageDialog.title = title
            messageDialog.text = message
            messageDialog.open()
        }
        function onTransactionDetailsReady(title, html) {
            transactionDetailsDialog.title = title
            transactionDetailsText.text = html
            transactionDetailsDialog.open()
        }
    }

    NuDialog {
        id: messageDialog
        showCancel: false
        dialogWidth: 620
        property alias text: messageLabel.text

        Label {
            id: messageLabel
            Layout.fillWidth: true
            Layout.preferredWidth: Math.max(0, messageDialog.width - messageDialog.leftPadding - messageDialog.rightPadding)
            Layout.maximumWidth: Layout.preferredWidth
            color: NuTokens.textPrimary
            font.pixelSize: NuTokens.fontBody
            wrapMode: Text.WordWrap
            textFormat: Text.PlainText
        }
    }

    NuDialog {
        id: transactionDetailsDialog
        showCancel: false
        acceptText: qsTr("Close")
        dialogWidth: 760

        ScrollView {
            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(root.height - 220, 520)
            clip: true

            TextEdit {
                id: transactionDetailsText
                width: parent.availableWidth
                readOnly: true
                selectByMouse: true
                textFormat: TextEdit.RichText
                wrapMode: TextEdit.WordWrap
                font.pixelSize: NuTokens.fontBody
                color: NuTokens.textPrimary
                selectedTextColor: NuTokens.textInverse
                selectionColor: NuTokens.lineStrong
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }
    }

    NuDialog {
        id: openUriDialog
        title: qsTr("Open URI")
        acceptText: qsTr("Open")
        dialogWidth: 620

        NuTextField {
            id: openUriField
            Layout.fillWidth: true
            placeholderText: qsTr("defcoin:<address>?amount=...")
            helpText: qsTr("Open a Defcoin payment URI in the Send screen for review before broadcast.")
        }

        onAccepted: {
            frame.openUri(openUriField.text)
            openUriField.text = ""
        }
        onRejected: openUriField.text = ""
        onClosed: if (!visible) openUriField.text = ""
    }

    Dialog {
        id: aboutDialog
        title: ""
        modal: true
        standardButtons: Dialog.NoButton
        width: Math.min(root.width - 120, 780)
        height: Math.min(root.height - 96, 640)
        anchors.centerIn: parent
        padding: NuTokens.spaceLg
        background: Rectangle {
            color: NuTokens.panelBase
            border.color: NuTokens.lineStrong
            radius: NuTokens.radiusLarge
        }

        contentItem: ColumnLayout {
            spacing: NuTokens.spaceMd

            NuAboutSummary {
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(405, aboutDialog.height - 200)
                showDetailsButton: false
                showOverlayText: true
                buildVersion: root.buildVersion
                buildId: root.buildId
                codeName: root.releaseCodeName
                heroHeight: Math.min(405, aboutDialog.height - 200)
                textHeight: 0
            }

            TextEdit {
                Layout.fillWidth: true
                Layout.preferredHeight: Math.max(56, implicitHeight)
                readOnly: true
                selectByMouse: true
                persistentSelection: true
                wrapMode: TextEdit.WordWrap
                text: root.trademarkNotice
                color: NuTokens.textSecondary
                selectedTextColor: NuTokens.textInverse
                selectionColor: NuTokens.lineStrong
                font.pixelSize: 10
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: NuTokens.spaceMd

                NuActionButton {
                    text: root.helpEnabled ? qsTr("Details") : qsTr("Build Notes")
                    Layout.preferredWidth: 132
                    helpText: root.helpEnabled ? qsTr("Open the detailed Help window with build, feature, history, and design notes.")
                                               : qsTr("Open Nu build notes and release differences.")
                    onClicked: {
                        aboutDialog.close()
                        root.openDetailedAbout()
                    }
                }

                Item { Layout.fillWidth: true }

                NuActionButton {
                    text: qsTr("Close")
                    Layout.preferredWidth: 112
                    helpText: qsTr("Close this About summary.")
                    onClicked: aboutDialog.close()
                }
            }
        }
    }

    Window {
        id: helpWindow
        visible: false
        width: 900
        height: 700
        minimumWidth: 720
        minimumHeight: 520
        color: NuTokens.panelBase
        flags: Qt.Window

        Rectangle {
            anchors.fill: parent
            color: NuTokens.panelBase
            border.color: NuTokens.lineStrong
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: NuTokens.spaceLg
            spacing: NuTokens.spaceMd

            ScrollView {
                id: helpScroll
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                TextEdit {
                    id: helpText
                    width: helpScroll.availableWidth
                    readOnly: true
                    selectByMouse: true
                    textFormat: TextEdit.RichText
                    wrapMode: TextEdit.WordWrap
                    font.pixelSize: NuTokens.fontBody
                    color: NuTokens.textPrimary
                    selectedTextColor: NuTokens.textInverse
                    selectionColor: NuTokens.lineStrong
                    onLinkActivated: root.openHelpLink(link)
                }
            }

            RowLayout {
                Layout.fillWidth: true

                Item { Layout.fillWidth: true }

                NuActionButton {
                    text: qsTr("Close")
                    Layout.preferredWidth: 112
                    helpText: qsTr("Close this Help window.")
                    onClicked: helpWindow.close()
                }
            }
        }
    }

}
