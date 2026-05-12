pragma Singleton
import QtQuick 2.15

QtObject {
    readonly property string networkState: "connected"
    readonly property int peerCount: 4
    readonly property int blockHeight: 2324057
    readonly property string syncState: "Up to date"
    readonly property bool walletLocked: true
    readonly property string totalBalance: "1.00000000 DFC"
    readonly property string availableBalance: "1.00000000"
    readonly property string pendingBalance: "0.00000000"
    readonly property string immatureBalance: "0.00000000"
    readonly property string receiveAddress: "dfc1qexampleaddress000000000000000000000"
    readonly property var recentTransactions: [
        ["05/12/26 12:05", "Mined", "ccminer2026test", "+0.02023132 DFC"],
        ["05/12/26 12:03", "Mined", "ccminer2026test", "+0.02023132 DFC"],
        ["05/12/26 12:02", "Received", "MLKbiGcdE6kZRMGidjsdi57ZWnToU5qB2R", "+1.00000000 DFC"]
    ]
    readonly property var peers: [
        ["-1", "Local", "192.168.2.51:1337", "N/A", "0 B", "0 B", "DefcoinCore:2026.1"],
        ["0", "Outbound", "66.42.91.225:1337", "22 ms", "12 KB", "77 KB", "DefcoinCore:1.0.0"],
        ["1", "Outbound", "135.148.43.189:10332", "38 ms", "2 KB", "31 KB", "DefcoinCore:1.0.0"],
        ["2", "Outbound", "[2600:3c00::f03c:92ff:fe17:805d]:10332", "15 ms", "3 KB", "35 KB", "DefcoinCore:1.0.0"]
    ]
    readonly property var nodeMetrics: [
        ["Network", "main"],
        ["Connections", "4 peers"],
        ["Block height", "2324057"],
        ["Best block time", "Tue May 12 12:08:24 2026"],
        ["Memory pool", "0 transactions"],
        ["Traffic", "94 KB received / 6 KB sent"]
    ]
    readonly property var logLines: [
        "2026-05-12T12:00:01Z Defcoin Core Nu shell started",
        "2026-05-12T12:00:01Z GUI bridge using fake read-only service",
        "2026-05-12T12:00:02Z Node status ready",
        "2026-05-12T12:00:02Z Deferred peers and traffic hydration queued"
    ]
}
