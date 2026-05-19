import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Defcoin.Nu 1.0

import "../Theme"
import "../Components"

ColumnLayout {
    id: root
    property int initialTab: 0
    property int initialPeerView: 0
    property bool trafficPaused: false
    property var frozenTrafficSamples: []
    property real trafficWindowStart: 0
    property real trafficWindowEnd: 1
    readonly property int trafficMaxChartSeconds: 15 * 60
    property var simplePeerColumns: ["Node", "Dir", "IP Address: Port", "Ping", "Sent", "Rec'd", "User Agent"]
    property var simplePeerTypes: ["number", "text", "ipport", "duration", "bytes", "bytes", "text"]
    property var simplePeerWeights: [0.38, 0.24, 1.7, 0.42, 0.42, 0.42, 1.35]
    property var simplePeerMinimums: [48, 34, 132, 52, 58, 58, 92]
    property var simplePeerMaximums: [64, 42, 390, 74, 82, 82, 280]
    property var simplePeerTooltips: [
        "Backend peer connection ID for this session.",
        "Connection direction: In means the peer connected to this wallet; Out means this wallet connected to the peer.",
        "Peer endpoint, including IP address and TCP port.",
        "Current round-trip latency reported by the backend.",
        "Total bytes sent to this peer since the connection opened.",
        "Total bytes received from this peer since the connection opened.",
        "Software name and version reported by the peer."
    ]
    property var detailedPeerColumns: ["Node", "Dir.", "IP", "Port", "DNS Name", "Domain Alias", "Protocol\nVersion", "Magic", "Svcs", "Ping", "Min Ping", "Sent", "Rec'd", "User Agent", "Conn Time", "Start\nHeight", "Last Send", "Last Recv", "Last TX", "Last Block", "Synced\nHeaders", "Synced\nBlocks", "Conn Type", "Network", "Addr\nEntries", "Min Fee\nFilter"]
    property var detailedPeerTypes: ["number", "text", "ipport", "number", "text", "text", "number", "text", "text", "duration", "duration", "bytes", "bytes", "text", "date", "number", "date", "date", "date", "date", "number", "number", "text", "text", "number", "amount"]
    property var detailedPeerWeights: [0.34, 0.28, 1.05, 0.34, 1.05, 1.05, 0.5, 0.55, 0.42, 0.46, 0.5, 0.42, 0.42, 1.35, 1.05, 0.55, 1.05, 1.05, 1.05, 1.05, 0.62, 0.62, 0.8, 0.58, 0.62, 0.76]
    property var detailedPeerMinimums: [44, 34, 128, 46, 90, 96, 62, 74, 54, 58, 58, 58, 58, 92, 130, 70, 130, 130, 130, 130, 80, 80, 84, 64, 76, 90]
    property var detailedPeerMaximums: [62, 42, 330, 70, 240, 220, 82, 92, 80, 78, 84, 82, 82, 260, 168, 96, 168, 168, 168, 168, 108, 108, 136, 110, 108, 130]
    property var detailedPeerTooltips: [
        "Backend peer connection ID for this session.",
        "Connection direction: In means the peer connected to this wallet; Out means this wallet connected to the peer.",
        "Peer IP address without the port. IPv4 values use fixed-width octet spacing so dots align.",
        "Peer TCP port.",
        "Best-effort reverse DNS name for the peer IP address. Blank means no DNS name has resolved yet.",
        "Known configured seed domain associated with this peer address, when the address matches a Defcoin seed result.",
        "P2P protocol version reported by the peer.",
        "Actual network message-start bytes selected for this peer, such as defc014e or fbc0b6db.",
        "Compact service flags advertised by the peer, such as N for NODE_NETWORK or W for witness support.",
        "Current round-trip latency reported by the backend.",
        "Best observed ping for this connection.",
        "Total bytes sent to this peer since the connection opened.",
        "Total bytes received from this peer since the connection opened.",
        "Software name and version reported by the peer.",
        "Local time when this peer connection opened.",
        "Block height the peer reported during version negotiation.",
        "Local time of the last message sent to this peer.",
        "Local time of the last message received from this peer.",
        "Local time of the last valid transaction received from this peer.",
        "Local time of the last block received from this peer.",
        "Last header height currently known in common with this peer. Unknown means the backend has not established one yet.",
        "Last block height currently known in common with this peer. Unknown means the backend has not established one yet.",
        "Backend connection type, such as outbound-full-relay, inbound, manual, or block-relay.",
        "Network transport used by the peer, such as ipv4, ipv6, onion, or i2p.",
        "Cumulative addr/addrv2 relay entries processed from this peer during this connection after Defcoin user-agent and port filters. This is not a unique node count; one peer can send up to about 1000 address records in one response.",
        "Minimum transaction relay fee rate this peer has announced with its feefilter policy, displayed as DFC per kilobyte."
    ]

    function durationText(seconds) {
        seconds = Math.max(0, Math.round(seconds))
        if (seconds < 60) return seconds + " sec"
        const minutes = Math.floor(seconds / 60)
        const remainder = seconds % 60
        if (minutes < 60) return remainder === 0 ? minutes + " min" : minutes + " min " + remainder + " sec"
        const hours = Math.floor(minutes / 60)
        const minuteRemainder = minutes % 60
        return minuteRemainder === 0 ? hours + " hr" : hours + " hr " + minuteRemainder + " min"
    }

    function sampleRangeText() {
        var samples = root.trafficPaused ? root.frozenTrafficSamples : NuService.trafficSamples
        if (!samples || samples.length < 2) return "Waiting for samples"
        var first = samples[0].seconds
        var last = samples[samples.length - 1].seconds
        var span = Math.max(1, last - first)
        var startSeconds = first + span * root.trafficWindowStart
        var endSeconds = first + span * root.trafficWindowEnd
        var firstTime = samples[0].timestampMs ? samples[0].timestampMs : Date.now()
        var startDate = new Date(firstTime + (startSeconds - first) * 1000)
        var endDate = new Date(firstTime + (endSeconds - first) * 1000)
        var visibleSeconds = Math.max(0, endSeconds - startSeconds)
        var text = startDate.toLocaleString() + " - " + endDate.toLocaleString()
        text += " | " + root.durationText(visibleSeconds) + " visible"
        if (span >= root.trafficMaxChartSeconds - 2) {
            text += " | Max chart length: " + root.durationText(root.trafficMaxChartSeconds)
        }
        return text
    }

    function tabToStack(tabIndex) {
        if (tabIndex === 1) return 3 // Log
        if (tabIndex === 2) return 4 // Console
        if (tabIndex === 3) return 2 // Traffic
        if (tabIndex === 4) return 1 // Peers
        return 0 // Status
    }

    spacing: NuTokens.spaceLg

    NuPageHeader {
        Layout.fillWidth: true
        title: "Diagnostics"
        detail: "Node status, peers, traffic, log, and console."
    }

    NuTabBar {
        id: tabs
        Layout.fillWidth: true
        currentIndex: root.initialTab
        NuTabButton { text: "Status" }
        NuTabButton { text: "Log" }
        NuTabButton { text: "Console" }
        NuTabButton { text: "Traffic" }
        NuTabButton { text: "Peers" }
    }

    StackLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        currentIndex: root.tabToStack(tabs.currentIndex)

        NuDataTable {
            tableId: "nodeStatusMetrics"
            columns: ["Metric", "Value"]
            columnTypes: ["text", "text"]
            columnWeights: [1.0, 2.8]
            rows: NuService.nodeMetrics
            emptyText: "Node status hydrates here."
        }

        ColumnLayout {
            spacing: NuTokens.spaceMd

            RowLayout {
                Layout.fillWidth: true
                spacing: NuTokens.spaceMd

                NuTabBar {
                    id: peerViewToggle
                    Layout.preferredWidth: 224
                    currentIndex: root.initialPeerView
                    NuTabButton { text: "Simple" }
                    NuTabButton { text: "Detailed" }
                    onCurrentIndexChanged: Qt.callLater(function() { peersTable.forceResetColumnWidths() })
                }

                Label {
                    Layout.fillWidth: true
                    text: peerViewToggle.currentIndex === 0
                          ? "Core peer health and traffic."
                          : "Full protocol fields for network inspection."
                    color: NuTokens.textSecondary
                    font.pixelSize: NuTokens.fontSmall
                    elide: Text.ElideRight
                }

            }

            NuDataTable {
                id: peersTable
                Layout.fillWidth: true
                Layout.fillHeight: true
                compact: true
                fontPixelSize: NuTokens.fontTiny
                alwaysShowHorizontalScrollBar: peerViewToggle.currentIndex === 1
                tableId: peerViewToggle.currentIndex === 0 ? "nodePeersSimple" : "nodePeersDetailed"
                restoreSavedColumnWidths: false
                columns: peerViewToggle.currentIndex === 0 ? root.simplePeerColumns : root.detailedPeerColumns
                columnTooltips: peerViewToggle.currentIndex === 0 ? root.simplePeerTooltips : root.detailedPeerTooltips
                columnTypes: peerViewToggle.currentIndex === 0 ? root.simplePeerTypes : root.detailedPeerTypes
                columnWeights: peerViewToggle.currentIndex === 0 ? root.simplePeerWeights : root.detailedPeerWeights
                columnMinimums: peerViewToggle.currentIndex === 0 ? root.simplePeerMinimums : root.detailedPeerMinimums
                columnMaximums: peerViewToggle.currentIndex === 0 ? root.simplePeerMaximums : root.detailedPeerMaximums
                rows: peerViewToggle.currentIndex === 0 ? NuService.peerRowsSimple : NuService.peerRowsDetailed
                emptyText: "Peers hydrate here after the tab renders."
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: NuTokens.spaceMd

                Label {
                    Layout.fillWidth: true
                    text: NuService.peerCount + " peers"
                    color: NuTokens.textSecondary
                    font.pixelSize: NuTokens.fontSmall
                }
            }
        }

        ColumnLayout {
            spacing: NuTokens.spaceMd

            NuTimelineGraph {
                Layout.fillWidth: true
                Layout.fillHeight: true
                samples: root.trafficPaused ? root.frozenTrafficSamples : NuService.trafficSamples
                windowStart: root.trafficWindowStart
                windowEnd: root.trafficWindowEnd
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: NuTokens.spaceMd
                Label {
                    text: "Visible time range"
                    color: NuTokens.textSecondary
                    font.pixelSize: NuTokens.fontSmall
                }
                Label {
                    Layout.fillWidth: true
                    text: root.sampleRangeText()
                    color: NuTokens.textPrimary
                    font.pixelSize: NuTokens.fontSmall
                    elide: Text.ElideRight
                }
            }

            NuTimelineWindow {
                Layout.fillWidth: true
                start: root.trafficWindowStart
                end: root.trafficWindowEnd
                onViewportChanged: (start, end) => {
                    root.trafficWindowStart = start
                    root.trafficWindowEnd = end
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: NuTokens.spaceMd

                Label {
                    text: "Total Received: " + NuService.trafficReceivedTotal + "    Total Sent: " + NuService.trafficSentTotal
                    color: NuTokens.textPrimary
                    font.pixelSize: NuTokens.fontBody
                }
                Item { Layout.fillWidth: true }
                NuActionButton {
                    text: root.trafficPaused ? "Resume" : "Pause"
                    Layout.preferredWidth: 112
                    helpText: "Pause graph animation while continuing capture."
                    onClicked: {
                        if (!root.trafficPaused) root.frozenTrafficSamples = NuService.trafficSamples.slice()
                        root.trafficPaused = !root.trafficPaused
                    }
                }
                NuActionButton {
                    text: "Export CSV"
                    Layout.preferredWidth: 132
                    helpText: "Export retained traffic samples."
                    onClicked: NuService.exportTrafficCsv()
                }
            }
        }

        NuPanel {
            ColumnLayout {
                anchors.fill: parent
                spacing: NuTokens.spaceMd

                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        Layout.fillWidth: true
                        text: "Log since launch"
                        color: NuTokens.textPrimary
                        font.pixelSize: NuTokens.fontBodyLarge
                        font.weight: Font.DemiBold
                    }
                    NuActionButton {
                        text: "Open debug.log"
                        Layout.preferredWidth: 148
                        helpText: "Open the backend debug.log file in the operating system's default log viewer. Nu startup diagnostics are shown only in this in-app launch log."
                        onClicked: NuService.openDebugLog()
                    }
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true

                    TextArea {
                        id: launchLogText
                        readOnly: true
                        selectByMouse: true
                        wrapMode: Text.NoWrap
                        color: NuTokens.textPrimary
                        selectionColor: NuTokens.lineStrong
                        selectedTextColor: NuTokens.textInverse
                        font.family: NuTokens.monoFont
                        font.pixelSize: NuTokens.fontLog
                        background: Rectangle { color: NuTokens.backgroundBase; border.color: NuTokens.lineSubtle }

                        function updateLogText() {
                            const oldCursor = cursorPosition
                            const oldSelectionStart = selectionStart
                            const oldSelectionEnd = selectionEnd
                            const hadSelection = oldSelectionStart !== oldSelectionEnd
                            text = NuService.logLines.join("\n")
                            cursorPosition = Math.min(oldCursor, length)
                            if (hadSelection) {
                                select(Math.min(oldSelectionStart, length), Math.min(oldSelectionEnd, length))
                            }
                        }

                        Component.onCompleted: updateLogText()
                        Connections {
                            target: NuService
                            function onLogChanged() { launchLogText.updateLogText() }
                        }

                        Shortcut {
                            sequences: [StandardKey.Copy]
                            enabled: launchLogText.activeFocus && launchLogText.selectedText.length > 0
                            onActivated: NuService.copyText(launchLogText.selectedText)
                        }
                    }
                }
            }
        }

        NuPanel {
            ColumnLayout {
                anchors.fill: parent
                spacing: NuTokens.spaceMd

                Label {
                    text: "RPC Console"
                    color: NuTokens.textPrimary
                    font.pixelSize: NuTokens.fontBodyLarge
                    font.weight: Font.DemiBold
                }

                Label {
                    Layout.fillWidth: true
                    text: "Advanced wallet and node commands run through the local Defcoin backend. Parameters must be a JSON array."
                    color: NuTokens.textSecondary
                    font.pixelSize: NuTokens.fontSmall
                    wrapMode: Text.WordWrap
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: warningText.implicitHeight + NuTokens.spaceMd
                    color: "#fff7d6"
                    border.color: NuTokens.stateWarning
                    radius: NuTokens.radiusMedium

                    Text {
                        id: warningText
                        anchors.fill: parent
                        anchors.margins: NuTokens.spaceSm
                        text: "Do not paste commands from strangers into Console."
                        color: "#4a3500"
                        font.pixelSize: NuTokens.fontBody
                        font.weight: Font.DemiBold
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: 4
                    rowSpacing: NuTokens.spaceMd
                    columnSpacing: NuTokens.spaceMd

                    Label { text: "Method"; color: NuTokens.textSecondary; font.pixelSize: NuTokens.fontBody }
                    NuTextField {
                        id: rpcMethod
                        Layout.preferredWidth: 260
                        placeholderText: "getwalletinfo"
                        onAccepted: NuService.runRpcCommand(rpcMethod.text, rpcParams.text, walletScoped.checked)
                    }
                    NuCheckBox {
                        id: walletScoped
                        text: "Wallet RPC"
                        checked: true
                    }
                    NuActionButton {
                        text: "Run"
                        primary: true
                        Layout.preferredWidth: 112
                        helpText: "Execute the RPC command against the local Defcoin backend."
                        onClicked: NuService.runRpcCommand(rpcMethod.text, rpcParams.text, walletScoped.checked)
                    }

                    Label { text: "Params"; color: NuTokens.textSecondary; font.pixelSize: NuTokens.fontBody }
                    NuTextField {
                        id: rpcParams
                        Layout.columnSpan: 3
                        Layout.fillWidth: true
                        placeholderText: "[]"
                        onAccepted: NuService.runRpcCommand(rpcMethod.text, rpcParams.text, walletScoped.checked)
                    }
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true

                    TextArea {
                        id: consoleOutput
                        readOnly: true
                        selectByMouse: true
                        wrapMode: Text.WrapAnywhere
                        text: NuService.consoleOutput
                        color: NuTokens.textPrimary
                        selectionColor: NuTokens.lineStrong
                        selectedTextColor: NuTokens.textInverse
                        font.family: NuTokens.monoFont
                        font.pixelSize: NuTokens.fontLog
                        background: Rectangle { color: NuTokens.backgroundBase; border.color: NuTokens.lineSubtle }
                    }
                }

                Connections {
                    target: NuService
                    function onConsoleChanged() {
                        consoleOutput.cursorPosition = consoleOutput.text.length
                    }
                }
            }
        }
    }
}
