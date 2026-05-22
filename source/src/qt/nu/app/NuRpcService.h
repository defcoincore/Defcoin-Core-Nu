#ifndef DEFCOIN_NU_RPC_SERVICE_H
#define DEFCOIN_NU_RPC_SERVICE_H

#include <QByteArray>
#include <QDateTime>
#include <QHash>
#include <QJsonArray>
#include <QJsonValue>
#include <QObject>
#include <QElapsedTimer>
#include <QSet>
#include <QUrl>
#include <QVariantList>

#include <functional>

class QNetworkAccessManager;
class QNetworkReply;
class QProcess;
class QTimer;

class NuRpcService final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool rpcConnected READ rpcConnected NOTIFY stateChanged)
    Q_PROPERTY(QString connectionStatus READ connectionStatus NOTIFY stateChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY stateChanged)
    Q_PROPERTY(QString networkState READ networkState NOTIFY stateChanged)
    Q_PROPERTY(int peerCount READ peerCount NOTIFY stateChanged)
    Q_PROPERTY(int blockHeight READ blockHeight NOTIFY stateChanged)
    Q_PROPERTY(QString syncState READ syncState NOTIFY stateChanged)
    Q_PROPERTY(bool walletLocked READ walletLocked NOTIFY stateChanged)
    Q_PROPERTY(bool walletEncrypted READ walletEncrypted NOTIFY walletChanged)
    Q_PROPERTY(QString totalBalance READ totalBalance NOTIFY walletChanged)
    Q_PROPERTY(QString availableBalance READ availableBalance NOTIFY walletChanged)
    Q_PROPERTY(QString pendingBalance READ pendingBalance NOTIFY walletChanged)
    Q_PROPERTY(QString immatureBalance READ immatureBalance NOTIFY walletChanged)
    Q_PROPERTY(QString receiveAddress READ receiveAddress NOTIFY walletChanged)
    Q_PROPERTY(QString receiveQrSource READ receiveQrSource NOTIFY walletChanged)
    Q_PROPERTY(QVariantList addressBook READ addressBook NOTIFY walletChanged)
    Q_PROPERTY(QVariantList receiveRequests READ receiveRequests NOTIFY walletChanged)
    Q_PROPERTY(QVariantList recentTransactions READ recentTransactions NOTIFY walletChanged)
    Q_PROPERTY(QVariantList peers READ peers NOTIFY peersChanged)
    Q_PROPERTY(QVariantList peerRowsSimple READ peerRowsSimple NOTIFY peersChanged)
    Q_PROPERTY(QVariantList peerRowsDetailed READ peerRowsDetailed NOTIFY peersChanged)
    Q_PROPERTY(QVariantList nodeMetrics READ nodeMetrics NOTIFY stateChanged)
    Q_PROPERTY(QVariantList trafficSamples READ trafficSamples NOTIFY trafficChanged)
    Q_PROPERTY(QString trafficReceivedTotal READ trafficReceivedTotal NOTIFY trafficChanged)
    Q_PROPERTY(QString trafficSentTotal READ trafficSentTotal NOTIFY trafficChanged)
    Q_PROPERTY(QStringList logLines READ logLines NOTIFY logChanged)
    Q_PROPERTY(QString consoleOutput READ consoleOutput NOTIFY consoleChanged)
    Q_PROPERTY(bool feeEstimateAvailable READ feeEstimateAvailable NOTIFY feeEstimateChanged)
    Q_PROPERTY(QString feeEstimateStatus READ feeEstimateStatus NOTIFY feeEstimateChanged)
    Q_PROPERTY(bool psbtLoaded READ psbtLoaded NOTIFY psbtChanged)
    Q_PROPERTY(bool psbtFinalized READ psbtFinalized NOTIFY psbtChanged)
    Q_PROPERTY(QString currentPsbtSummary READ currentPsbtSummary NOTIFY psbtChanged)
    Q_PROPERTY(bool onlyDefcoinUserAgents READ onlyDefcoinUserAgents WRITE setOnlyDefcoinUserAgents NOTIFY settingsChanged)
    Q_PROPERTY(bool onlyDefcoinMagicBytes READ onlyDefcoinMagicBytes WRITE setOnlyDefcoinMagicBytes NOTIFY settingsChanged)
    Q_PROPERTY(bool switchToDefcoinOnlyMagicStartingJuly2026 READ switchToDefcoinOnlyMagicStartingJuly2026 WRITE setSwitchToDefcoinOnlyMagicStartingJuly2026 NOTIFY settingsChanged)
    Q_PROPERTY(bool disallowLanNodeDiscovery READ disallowLanNodeDiscovery WRITE setDisallowLanNodeDiscovery NOTIFY settingsChanged)
    Q_PROPERTY(bool maskBalances READ maskBalances WRITE setMaskBalances NOTIFY settingsChanged)
    Q_PROPERTY(bool thirdPartyTxUrlsEnabled READ thirdPartyTxUrlsEnabled WRITE setThirdPartyTxUrlsEnabled NOTIFY settingsChanged)
    Q_PROPERTY(QString thirdPartyTxUrl READ thirdPartyTxUrl WRITE setThirdPartyTxUrl NOTIFY settingsChanged)

public:
    explicit NuRpcService(QObject* parent = nullptr);
    ~NuRpcService() override;

    bool rpcConnected() const { return m_rpc_connected; }
    QString connectionStatus() const { return m_connection_status; }
    QString lastError() const { return m_last_error; }
    QString networkState() const { return m_network_state; }
    int peerCount() const { return m_peer_count; }
    int blockHeight() const { return m_block_height; }
    QString syncState() const { return m_sync_state; }
    bool walletLocked() const { return m_wallet_locked; }
    bool walletEncrypted() const { return m_wallet_encrypted; }
    QString totalBalance() const { return m_mask_balances ? QStringLiteral("******** DFC") : m_total_balance; }
    QString availableBalance() const { return m_mask_balances ? QStringLiteral("********") : m_available_balance; }
    QString pendingBalance() const { return m_mask_balances ? QStringLiteral("********") : m_pending_balance; }
    QString immatureBalance() const { return m_mask_balances ? QStringLiteral("********") : m_immature_balance; }
    QString receiveAddress() const { return m_receive_address; }
    QString receiveQrSource() const { return m_receive_qr_source; }
    QVariantList addressBook() const { return m_address_book; }
    QVariantList receiveRequests() const { return m_receive_requests; }
    QVariantList recentTransactions() const { return m_recent_transactions; }
    QVariantList peers() const { return m_peers; }
    QVariantList peerRowsSimple() const { return m_peer_rows_simple; }
    QVariantList peerRowsDetailed() const { return m_peer_rows_detailed; }
    QVariantList nodeMetrics() const { return m_node_metrics; }
    QVariantList trafficSamples() const { return m_traffic_samples; }
    QString trafficReceivedTotal() const { return m_traffic_received_total; }
    QString trafficSentTotal() const { return m_traffic_sent_total; }
    QStringList logLines() const { return m_log_lines; }
    QString consoleOutput() const { return m_console_output; }
    bool feeEstimateAvailable() const { return m_fee_estimate_available; }
    QString feeEstimateStatus() const { return m_fee_estimate_status; }
    bool psbtLoaded() const { return !m_current_psbt.isEmpty(); }
    bool psbtFinalized() const { return !m_current_psbt_final_hex.isEmpty(); }
    QString currentPsbtSummary() const { return m_current_psbt_summary; }
    bool onlyDefcoinUserAgents() const { return m_only_defcoin_user_agents; }
    bool onlyDefcoinMagicBytes() const { return m_only_defcoin_magic_bytes; }
    bool switchToDefcoinOnlyMagicStartingJuly2026() const { return m_switch_to_defcoin_only_magic_starting_july_2026; }
    bool disallowLanNodeDiscovery() const { return m_disallow_lan_node_discovery; }
    bool maskBalances() const { return m_mask_balances; }
    bool thirdPartyTxUrlsEnabled() const { return m_third_party_tx_urls_enabled; }
    QString thirdPartyTxUrl() const { return m_third_party_tx_url; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void requestNewAddress(const QString& label = QString(), const QString& amount = QString(), const QString& message = QString());
    Q_INVOKABLE void deleteReceiveRequest(const QString& address);
    Q_INVOKABLE void deleteReceiveRequests(const QVariantList& addresses);
    Q_INVOKABLE void sendCoins(const QString& address,
                               const QString& amount,
                               const QString& fee_mode = QStringLiteral("recommended"),
                               const QString& fee_target = QStringLiteral("6"),
                               const QString& custom_fee_rate = QString(),
                               bool subtract_fee_from_amount = false,
                               const QString& label = QString(),
                               const QString& custom_change_address = QString());
    Q_INVOKABLE void createPsbt(const QString& address,
                                const QString& amount,
                                const QString& fee_mode = QStringLiteral("recommended"),
                                const QString& fee_target = QStringLiteral("6"),
                                const QString& custom_fee_rate = QString(),
                                bool subtract_fee_from_amount = false,
                                const QString& label = QString(),
                                const QString& custom_change_address = QString());
    Q_INVOKABLE void setAddressLabel(const QString& address, const QString& label);
    Q_INVOKABLE void setNetworkActive(bool active);
    Q_INVOKABLE void pingPeers();
    Q_INVOKABLE void runRpcCommand(const QString& method, const QString& params_json, bool wallet_scoped);
    Q_INVOKABLE void copyText(const QString& text);
    Q_INVOKABLE void backupWallet();
    Q_INVOKABLE void encryptWallet(const QString& passphrase);
    Q_INVOKABLE void changeWalletPassphrase(const QString& old_passphrase, const QString& new_passphrase);
    Q_INVOKABLE void signMessage(const QString& address, const QString& message);
    Q_INVOKABLE void verifyMessage(const QString& address, const QString& signature, const QString& message);
    Q_INVOKABLE void openDebugLog();
    Q_INVOKABLE void openHelpManual(const QString& page = QString());
    Q_INVOKABLE QString helpManualHtml(const QString& page = QString()) const;
    Q_INVOKABLE void exportTransactionsCsv();
    Q_INVOKABLE void exportTrafficCsv();
    Q_INVOKABLE QVariantList tableColumnWidths(const QString& table_id, const QVariantList& default_widths) const;
    Q_INVOKABLE QVariantList suggestedTableColumnWidths(const QVariantList& columns,
                                                        const QVariantList& rows,
                                                        const QVariantList& column_types,
                                                        const QVariantList& column_weights,
                                                        const QVariantList& column_minimums,
                                                        const QVariantList& column_maximums,
                                                        int available_width,
                                                        int font_pixel_size,
                                                        bool compact) const;
    Q_INVOKABLE void saveTableColumnWidths(const QString& table_id, const QVariantList& widths);
    Q_INVOKABLE void resetTableColumnWidths(const QString& table_id = QString());
    Q_INVOKABLE void loadPsbtFromFile();
    Q_INVOKABLE void loadPsbtFromClipboard();
    Q_INVOKABLE void signCurrentPsbt();
    Q_INVOKABLE void finalizeCurrentPsbt();
    Q_INVOKABLE void broadcastFinalizedPsbt();
    Q_INVOKABLE void copyCurrentPsbt();
    Q_INVOKABLE void saveCurrentPsbt();
    Q_INVOKABLE void clearCurrentPsbt();
    Q_INVOKABLE void requestTransactionDetails(const QString& txid);
    Q_INVOKABLE QString receiveRequestQrSource(const QString& uri) const;
    Q_INVOKABLE QString explorerPresetUrl(int index) const;
    Q_INVOKABLE QString explorerUrlForTransaction(const QString& txid) const;
    Q_INVOKABLE QString explorerUrlForAddress(const QString& address) const;

public Q_SLOTS:
    void setOnlyDefcoinUserAgents(bool enabled);
    void setOnlyDefcoinMagicBytes(bool enabled);
    void setSwitchToDefcoinOnlyMagicStartingJuly2026(bool enabled);
    void setDisallowLanNodeDiscovery(bool enabled);
    void setMaskBalances(bool enabled);
    void setThirdPartyTxUrlsEnabled(bool enabled);
    void setThirdPartyTxUrl(const QString& url);

Q_SIGNALS:
    void stateChanged();
    void walletChanged();
    void peersChanged();
    void trafficChanged();
    void settingsChanged();
    void logChanged();
    void consoleChanged();
    void feeEstimateChanged();
    void psbtChanged();
    void tableSettingsChanged();
    void userMessage(const QString& title, const QString& message);
    void transactionDetailsReady(const QString& title, const QString& html);

private:
    using RpcCallback = std::function<void(const QJsonValue&, const QString&)>;

    struct PendingCall {
        QString method;
        RpcCallback callback;
    };

    void loadLocalSettings();
    bool loadRpcSettings();
    QString defaultDataDir() const;
    void readDefcoinConf(const QString& conf_path);
    bool readCookie();
    QString backendBinaryPath() const;
    QString debugLogPath() const;
    bool ensureBackendStarted();
    void stopOwnedBackend();
    void appendLaunchDiagnostic(const QString& message);
    void appendDebugLogLineFromNu(const QString& message);
    void beginBackendDebugLogSection(bool write_to_debug_log);
    QStringList backendRuntimeDiagnostics(const QString& binary) const;
    bool shouldAutostartAfterTransportError(QNetworkReply* reply) const;
    void rpcCall(const QString& method, const QJsonArray& params, bool wallet_scoped, RpcCallback callback);
    void handleReply(QNetworkReply* reply);
    QUrl rpcUrl(bool wallet_scoped) const;
    void setError(const QString& message);
    void clearError();

    void refreshNode();
    void refreshWallet();
    void refreshAddressBook();
    void refreshFeeEstimate();
    void schedulePeerNameLookups(const QString& host);
    void scheduleConfiguredSeedAliasLookups();
    void sampleTraffic();
    void refreshDebugLog();
    void updateReceiveQr();
    QString receiveRequestSettingsKey() const;
    QVariantMap receiveRequestMeta(const QString& address,
                                   const QString& label,
                                   const QString& amount,
                                   const QString& message,
                                   const QString& uri,
                                   const QString& date,
                                   qint64 created_ms) const;
    QVariantMap receiveRequestRow(const QVariantMap& meta) const;
    void loadReceiveRequests();
    void saveReceiveRequests() const;
    QString defcoinUri(const QString& address, const QString& amount, const QString& label, const QString& message) const;
    QString qrSourceForUri(const QString& uri) const;
    QString normalizedExplorerUrl(const QString& url) const;
    QString explorerAddressUrlTemplate(const QString& url) const;
    void rebuildNodeMetrics();
    QString helpManualPath(const QString& page) const;
    void loadPsbtPayload(const QByteArray& payload, const QString& source);
    void analyzeCurrentPsbt(const QString& source);

    static QString formatAmount(const QJsonValue& value);
    static QString formatBytes(qint64 bytes);
    static QString formatPing(const QJsonValue& seconds);
    static QString formatServices(const QString& services_hex);
    static QString trimUserAgent(QString subver);
    static bool isDefcoinUserAgent(QString subver);

    QNetworkAccessManager* m_network = nullptr;
    QTimer* m_refresh_timer = nullptr;
    QTimer* m_traffic_timer = nullptr;
    QElapsedTimer m_uptime;
    QDateTime m_app_launch_utc;
    QHash<int, PendingCall> m_pending;
    int m_next_id = 1;

    QString m_data_dir;
    QString m_rpc_host = QStringLiteral("127.0.0.1");
    int m_rpc_port = 9332;
    QString m_rpc_user;
    QString m_rpc_password;
    QString m_wallet_name;
    bool m_backend_start_attempted = false;
    bool m_backend_started_by_nu = false;
    qint64 m_backend_pid = 0;
    QProcess* m_backend_process = nullptr;
    bool m_have_pending_network_active = false;
    bool m_pending_network_active = true;
    bool m_applying_pending_network_active = false;
    bool m_only_defcoin_magic_bytes = false;
    bool m_switch_to_defcoin_only_magic_starting_july_2026 = true;
    bool m_disallow_lan_node_discovery = false;

    bool m_rpc_connected = false;
    QString m_connection_status = QStringLiteral("RPC not connected");
    QString m_last_error;
    QString m_network_state = QStringLiteral("isolated");
    int m_peer_count = 0;
    int m_block_height = 0;
    QString m_sync_state = QStringLiteral("Unknown");
    bool m_wallet_locked = true;
    bool m_wallet_encrypted = false;
    QString m_metric_network_active = QStringLiteral("Hydrating");
    QString m_metric_connections = QStringLiteral("0");
    QString m_metric_inbound = QStringLiteral("0");
    QString m_metric_outbound = QStringLiteral("0");
    QString m_metric_version = QStringLiteral("Unknown");
    QString m_metric_blocks = QStringLiteral("Unknown");
    QString m_metric_headers = QStringLiteral("Unknown");
    QString m_metric_verification = QStringLiteral("Unknown");
    QString m_metric_traffic = QStringLiteral("0 B received / 0 B sent");
    QString m_total_balance = QStringLiteral("0.00000000 DFC");
    QString m_available_balance = QStringLiteral("0.00000000");
    QString m_pending_balance = QStringLiteral("0.00000000");
    QString m_immature_balance = QStringLiteral("0.00000000");
    QString m_receive_address;
    QString m_receive_qr_source;
    QString m_receive_label;
    QString m_receive_amount;
    QString m_receive_message;
    QString m_loaded_receive_request_settings_key;
    bool m_seed_alias_lookups_started = false;
    QHash<QString, QString> m_peer_dns_name_by_host;
    QHash<QString, QString> m_peer_domain_alias_by_host;
    QHash<QString, int> m_peer_domain_alias_priority_by_host;
    QSet<QString> m_peer_reverse_lookup_pending;
    QSet<QString> m_peer_reverse_lookup_attempted;
    int m_address_book_refresh_generation = 0;
    QVariantList m_address_book;
    QVariantList m_receive_requests;
    QVariantList m_recent_transactions;
    QVariantList m_peers;
    QVariantList m_peer_rows_simple;
    QVariantList m_peer_rows_detailed;
    QVariantList m_node_metrics;
    QVariantList m_traffic_samples;
    qint64 m_last_traffic_ms = 0;
    qint64 m_last_bytes_recv = -1;
    qint64 m_last_bytes_sent = -1;
    QString m_traffic_received_total = QStringLiteral("0 B");
    QString m_traffic_sent_total = QStringLiteral("0 B");
    QStringList m_log_lines;
    QString m_last_logged_error_message;
    QString m_debug_log_path;
    qint64 m_debug_log_offset = -1;
    bool m_debug_log_collecting_continuation = false;
    bool m_launch_diagnostics_section_started = false;
    bool m_backend_log_section_started = false;
    QString m_console_output = QStringLiteral("Enter an RPC method and JSON parameter array, then run the command.");
    bool m_fee_estimate_available = false;
    QString m_fee_estimate_status = QStringLiteral("Fee estimate hydrates after RPC connects.");
    QString m_current_psbt;
    QString m_current_psbt_summary = QStringLiteral("No PSBT loaded.");
    QString m_current_psbt_final_hex;
    bool m_only_defcoin_user_agents = true;
    bool m_mask_balances = false;
    bool m_third_party_tx_urls_enabled = false;
    QString m_third_party_tx_url;
};

#endif // DEFCOIN_NU_RPC_SERVICE_H
