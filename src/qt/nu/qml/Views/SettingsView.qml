import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Defcoin.Nu 1.0

import "../Theme"
import "../Components"

ColumnLayout {
    id: root
    spacing: NuTokens.spaceLg
    property bool helpEnabled: NuHelpEnabled

    NuPageHeader {
        Layout.fillWidth: true
        title: "Settings"
        detail: "Wallet, network, display, advanced options, and product information."
    }

    NuTabBar {
        id: tabs
        Layout.fillWidth: true
        NuTabButton { text: "Wallet" }
        NuTabButton { text: "Network" }
        NuTabButton { text: "Display" }
    }

    StackLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        currentIndex: tabs.currentIndex

        NuPanel {
            ColumnLayout {
                anchors.fill: parent
                spacing: NuTokens.spaceLg

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: NuTokens.spaceSm

                    Label {
                        text: "Wallet file"
                        color: NuTokens.textPrimary
                        font.pixelSize: NuTokens.fontBody
                        font.weight: Font.DemiBold
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: NuTokens.spaceMd

                        NuActionButton {
                            text: "Backup wallet"
                            Layout.preferredWidth: 180
                            helpText: "Write a backup copy of the current wallet file."
                            onClicked: NuService.backupWallet()
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: NuTokens.lineSubtle
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: NuTokens.spaceSm

                    Label {
                        text: "Passphrase protection"
                        color: NuTokens.textPrimary
                        font.pixelSize: NuTokens.fontBody
                        font.weight: Font.DemiBold
                    }

                    Label {
                        Layout.fillWidth: true
                        text: NuService.walletEncrypted
                              ? "This wallet is encrypted. You can change its passphrase, but Core wallets cannot be unencrypted in place."
                              : "This wallet is not encrypted. Encrypt it before relying on passphrase protection."
                        color: NuTokens.textSecondary
                        font.pixelSize: NuTokens.fontBody
                        wrapMode: Text.WordWrap
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: NuTokens.spaceMd

                        NuActionButton {
                            text: "Wallet security..."
                            Layout.preferredWidth: 190
                            primary: !NuService.walletEncrypted
                            helpText: "Encrypt this wallet or change the current wallet passphrase."
                            onClicked: walletSecurityDialog.open()
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: NuTokens.lineSubtle
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: NuTokens.spaceSm

                    Label {
                        text: "Message signing"
                        color: NuTokens.textPrimary
                        font.pixelSize: NuTokens.fontBody
                        font.weight: Font.DemiBold
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: NuTokens.spaceMd

                        NuActionButton {
                            text: "Sign message"
                            Layout.preferredWidth: 170
                            helpText: "Sign a message with a wallet address."
                            onClicked: signDialog.open()
                        }
                        NuActionButton {
                            text: "Verify message"
                            Layout.preferredWidth: 180
                            helpText: "Verify an address, message, and signature."
                            onClicked: verifyDialog.open()
                        }
                    }
                }

                Label {
                    Layout.fillWidth: true
                    text: "Default currency unit is DFC. Address labels are shared with Send and Receive through the wallet address book."
                    color: NuTokens.textSecondary
                    font.pixelSize: NuTokens.fontBody
                    wrapMode: Text.WordWrap
                }

                NuDataTable {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    tableId: "settingsAddressBook"
                    columns: ["Label", "Address", "Type", "Received"]
                    columnTypes: ["text", "address", "text", "amount"]
                    columnTooltips: [
                        "Wallet label used to recognize this contact or receive address.",
                        "Wallet address. Select and copy the full value exactly when sharing it.",
                        "Address-book purpose, such as contact or receive.",
                        "Total amount received by this wallet address."
                    ]
                    rows: NuService.addressBook
                    columnWeights: [1.4, 3.5, 0.85, 1.1]
                    columnMinimums: [130, 260, 80, 130]
                    columnMaximums: [360, 560, 130, 170]
                    rowSelectionEnabled: true
                    rowKeyMetaField: "address"
                    emptyText: "Wallet address book entries appear after RPC connects."
                }

                NuDialog {
                    id: walletSecurityDialog
                    title: "Wallet security"
                    acceptText: NuService.walletEncrypted ? "Change passphrase" : "Encrypt wallet"
                    dialogWidth: 640

                    Label {
                        Layout.fillWidth: true
                        text: NuService.walletEncrypted
                              ? "This wallet is encrypted. Enter the current passphrase and a new passphrase to change it."
                              : "This wallet is not encrypted. Add a passphrase to encrypt the wallet file."
                        color: NuTokens.textPrimary
                        font.pixelSize: NuTokens.fontBody
                        wrapMode: Text.WordWrap
                    }

                    NuTextField {
                        id: oldPassphrase
                        Layout.fillWidth: true
                        visible: NuService.walletEncrypted
                        Layout.preferredHeight: visible ? implicitHeight : 0
                        echoMode: TextInput.Password
                        placeholderText: "Current passphrase"
                    }
                    NuTextField {
                        id: newPassphrase
                        Layout.fillWidth: true
                        echoMode: TextInput.Password
                        placeholderText: NuService.walletEncrypted ? "New passphrase" : "Passphrase"
                        helpText: "Use a long passphrase. Losing it can make wallet funds permanently inaccessible."
                    }

                    Label {
                        Layout.fillWidth: true
                        text: "Removing wallet encryption is not supported in place. To end up with an unencrypted wallet, make verified backups, create or restore into a new unencrypted wallet, then move funds or perform an expert key migration and verify the result before retiring the encrypted wallet."
                        color: NuTokens.textSecondary
                        font.pixelSize: NuTokens.fontSmall
                        wrapMode: Text.WordWrap
                    }

                    onAccepted: NuService.walletEncrypted
                                ? NuService.changeWalletPassphrase(oldPassphrase.text, newPassphrase.text)
                                : NuService.encryptWallet(newPassphrase.text)
                    onClosed: {
                        oldPassphrase.text = ""
                        newPassphrase.text = ""
                    }
                }

                NuDialog {
                    id: signDialog
                    title: "Sign message"
                    acceptText: "Sign"
                    dialogWidth: 620

                    NuTextField {
                        id: signAddress
                        Layout.fillWidth: true
                        placeholderText: "Wallet address"
                    }
                    TextArea {
                        id: signMessage
                        Layout.fillWidth: true
                        Layout.preferredHeight: 120
                        placeholderText: "Message"
                        color: NuTokens.textPrimary
                        selectByMouse: true
                        wrapMode: TextArea.Wrap
                        background: Rectangle { color: NuTokens.backgroundBase; border.color: NuTokens.lineSubtle; radius: NuTokens.radiusSmall }
                    }

                    onAccepted: NuService.signMessage(signAddress.text, signMessage.text)
                    onClosed: signMessage.text = ""
                }

                NuDialog {
                    id: verifyDialog
                    title: "Verify message"
                    acceptText: "Verify"
                    dialogWidth: 640

                    NuTextField {
                        id: verifyAddress
                        Layout.fillWidth: true
                        placeholderText: "Address"
                    }
                    NuTextField {
                        id: verifySignature
                        Layout.fillWidth: true
                        placeholderText: "Signature"
                    }
                    TextArea {
                        id: verifyMessage
                        Layout.fillWidth: true
                        Layout.preferredHeight: 120
                        placeholderText: "Message"
                        color: NuTokens.textPrimary
                        selectByMouse: true
                        wrapMode: TextArea.Wrap
                        background: Rectangle { color: NuTokens.backgroundBase; border.color: NuTokens.lineSubtle; radius: NuTokens.radiusSmall }
                    }

                    onAccepted: NuService.verifyMessage(verifyAddress.text, verifySignature.text, verifyMessage.text)
                    onClosed: verifyMessage.text = ""
                }
            }
        }

        NuPanel {
            ColumnLayout {
                anchors.fill: parent
                spacing: NuTokens.spaceLg

                NuStatusDot {
                    label: NuService.networkState === "connected" ? "Network connected" : "Network isolated"
                    stateColor: NuService.networkState === "connected" ? NuTokens.stateConnected : NuTokens.stateError
                }

                RowLayout {
                    spacing: NuTokens.spaceMd
                    NuActionButton {
                        text: "Connect"
                        Layout.preferredWidth: 130
                        primary: NuService.networkState !== "connected"
                        helpText: "Enable P2P network activity."
                        onClicked: NuService.setNetworkActive(true)
                    }
                    NuActionButton {
                        text: "Isolate"
                        Layout.preferredWidth: 130
                        danger: NuService.networkState === "connected"
                        helpText: "Disable P2P network activity without closing the wallet."
                        onClicked: NuService.setNetworkActive(false)
                    }
                }

                NuCheckBox {
                    text: "Only show and accept peers whose user agent begins with /Defcoin"
                    checked: NuService.onlyDefcoinUserAgents
                    onToggled: NuService.onlyDefcoinUserAgents = checked
                }

                NuCheckBox {
                    text: "Only use Defcoin magic bytes"
                    checked: NuService.onlyDefcoinMagicBytes
                    helpText: "When enabled, the backend rejects legacy fbc0b6db peers and only accepts Defcoin-specific defc014e network magic. Turn it off during migration to accept both old and new Defcoin peers."
                    onToggled: NuService.onlyDefcoinMagicBytes = checked
                }

                NuCheckBox {
                    text: "Switch to Defcoin-only magic starting July 1, 2026"
                    checked: NuService.switchToDefcoinOnlyMagicStartingJuly2026
                    helpText: "When enabled, each launch on or after July 1, 2026 automatically enables Defcoin-only magic before networking starts."
                    onToggled: NuService.switchToDefcoinOnlyMagicStartingJuly2026 = checked
                }

                NuCheckBox {
                    text: "Disallow finding nodes on LAN"
                    checked: NuService.disallowLanNodeDiscovery
                    helpText: "Off by default. When enabled, peer address relay will not add local/private LAN addresses to automatic discovery. Manual connections and inbound LAN peers are still allowed."
                    onToggled: NuService.disallowLanNodeDiscovery = checked
                }

                Label {
                    Layout.fillWidth: true
                    text: "The user-agent filter is applied in the Nu peer table immediately. Defcoin magic and LAN discovery settings are applied at backend startup, so changing them requires a restart."
                    color: NuTokens.textSecondary
                    font.pixelSize: NuTokens.fontBody
                    wrapMode: Text.WordWrap
                }
            }
        }

        NuPanel {
            ColumnLayout {
                anchors.fill: parent
                spacing: NuTokens.spaceLg
                Label {
                    Layout.fillWidth: true
                    text: "Nu uses a neutral high-contrast interface designed for legibility, clear hierarchy, and fewer visual distractions."
                    color: NuTokens.textSecondary
                    font.pixelSize: NuTokens.fontBody
                    wrapMode: Text.WordWrap
                }

                NuActionButton {
                    text: "Reset widths"
                    Layout.preferredWidth: 168
                    helpText: "Forget saved table column widths, remove active table sorts, and restore first-launch table defaults."
                    onClicked: NuService.resetTableColumnWidths("")
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: NuTokens.lineSubtle
                }

                NuCheckBox {
                    text: "Enable third party transaction URLs"
                    checked: NuService.thirdPartyTxUrlsEnabled
                    helpText: "Show block explorer links in transaction and receive-request detail windows when a valid URL template is configured."
                    onToggled: NuService.thirdPartyTxUrlsEnabled = checked
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: 2
                    rowSpacing: NuTokens.spaceMd
                    columnSpacing: NuTokens.spaceLg

                    Label {
                        text: "Explorer preset"
                        color: NuTokens.textSecondary
                        font.pixelSize: NuTokens.fontBody
                    }
                    NuComboBox {
                        id: explorerPreset
                        Layout.fillWidth: true
                        model: ["Explorer preset...", "DC903 Explorer", "DC903 Legacy Explorer"]
                        helpText: "Choose a known Defcoin block explorer transaction URL. Selecting one fills the URL template field."
                        onActivated: function(index) {
                            var preset = NuService.explorerPresetUrl(index)
                            if (preset.length > 0) {
                                explorerUrl.text = preset
                                NuService.thirdPartyTxUrl = preset
                                NuService.thirdPartyTxUrlsEnabled = true
                            }
                        }
                    }

                    Label {
                        text: "Transaction URL"
                        color: NuTokens.textSecondary
                        font.pixelSize: NuTokens.fontBody
                    }
                    NuTextField {
                        id: explorerUrl
                        Layout.fillWidth: true
                        text: NuService.thirdPartyTxUrl
                        placeholderText: "https://example.invalid/tx/%s"
                        helpText: "Use %s where the transaction ID should be inserted. Address links are derived from the same template."
                        onEditingFinished: NuService.thirdPartyTxUrl = text
                    }
                }

                Label {
                    Layout.fillWidth: true
                    text: NuService.thirdPartyTxUrlsEnabled && NuService.thirdPartyTxUrl.indexOf("%s") < 0
                          ? "Explorer links are enabled but the URL must include %s."
                          : "Explorer links appear only in detail dialogs and open outside the wallet."
                    color: NuService.thirdPartyTxUrlsEnabled && NuService.thirdPartyTxUrl.indexOf("%s") < 0 ? NuTokens.stateWarning : NuTokens.textSecondary
                    font.pixelSize: NuTokens.fontSmall
                    wrapMode: Text.WordWrap
                }
            }
        }
    }
}
