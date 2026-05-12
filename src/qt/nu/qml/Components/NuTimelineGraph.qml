import QtQuick 2.15

import "../Theme"

Canvas {
    id: root
    antialiasing: true
    implicitHeight: 360

    property color receivedColor: NuTokens.dataReceived
    property color sentColor: NuTokens.dataSent

    onPaint: {
        const ctx = getContext("2d")
        ctx.reset()
        ctx.fillStyle = NuTokens.panelBase
        ctx.fillRect(0, 0, width, height)
        ctx.strokeStyle = NuTokens.lineSubtle
        ctx.lineWidth = 1
        for (let y = 0; y <= 4; y++) {
            const py = Math.round((height - 1) * y / 4)
            ctx.beginPath()
            ctx.moveTo(0, py)
            ctx.lineTo(width, py)
            ctx.stroke()
        }
        ctx.strokeStyle = receivedColor
        ctx.lineWidth = 2
        ctx.beginPath()
        ctx.moveTo(0, height * 0.72)
        ctx.lineTo(width * 0.25, height * 0.60)
        ctx.lineTo(width * 0.55, height * 0.45)
        ctx.lineTo(width, height * 0.52)
        ctx.stroke()
        ctx.strokeStyle = sentColor
        ctx.beginPath()
        ctx.moveTo(0, height * 0.80)
        ctx.lineTo(width * 0.35, height * 0.76)
        ctx.lineTo(width * 0.70, height * 0.62)
        ctx.lineTo(width, height * 0.70)
        ctx.stroke()
    }
}
