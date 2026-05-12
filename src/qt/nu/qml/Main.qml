import QtQuick 2.15
import QtQuick.Controls 2.15

import "Shell"
import "Theme"

ApplicationWindow {
    id: root
    visible: true
    width: 1180
    height: 760
    minimumWidth: 980
    minimumHeight: 640
    title: "Defcoin Core Nu"
    color: NuTokens.backgroundBase

    AppFrame {
        anchors.fill: parent
    }
}
