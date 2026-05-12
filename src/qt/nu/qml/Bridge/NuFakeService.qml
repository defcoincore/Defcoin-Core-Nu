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
}
