#include "NuRpcService.h"

#include <QApplication>
#include <QAbstractSocket>
#include <QClipboard>
#include <QCryptographicHash>
#include <QDate>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QDnsLookup>
#include <QEventLoop>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QHostInfo>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QPair>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QSet>
#include <QStandardPaths>
#include <QTextStream>
#include <QTimeZone>
#include <QTimer>
#include <QUrlQuery>
#include <QVector>

#include <qrencode.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#if defined(Q_OS_MACOS)
#include "MacHelp.h"
#endif

#if !defined(Q_OS_WIN)
#include <pwd.h>
#include <unistd.h>
#endif

namespace {
constexpr int QR_IMAGE_SIZE = 512;

QVariantList row(std::initializer_list<QVariant> values)
{
    QVariantList out;
    for (const QVariant& value : values) out.push_back(value);
    return out;
}

QVariantMap tableRow(std::initializer_list<QVariant> cells, const QVariantMap& meta)
{
    QVariantMap out;
    out.insert(QStringLiteral("cells"), row(cells));
    out.insert(QStringLiteral("meta"), meta);
    return out;
}

QVariantList tableRowCells(const QVariant& value)
{
    const QVariantMap map = value.toMap();
    if (map.contains(QStringLiteral("cells"))) {
        return map.value(QStringLiteral("cells")).toList();
    }
    return value.toList();
}

QPair<QString, QString> splitPeerAddressAndPort(QString raw)
{
    raw = raw.trimmed();
    if (raw.startsWith(QLatin1Char('[')) && raw.contains(QStringLiteral("]:"))) {
        const int close = raw.lastIndexOf(QStringLiteral("]:"));
        return {raw.mid(1, close - 1), raw.mid(close + 2)};
    }

    if (raw.startsWith(QLatin1Char('[')) && raw.endsWith(QLatin1Char(']'))) {
        return {raw.mid(1, raw.size() - 2), QString()};
    }

    static const QRegularExpression ipv4_with_port(QStringLiteral(R"(^(\d{1,3}(?:\.\d{1,3}){3})(?::(\d+))?$)"));
    const QRegularExpressionMatch match = ipv4_with_port.match(raw);
    if (match.hasMatch()) {
        return {match.captured(1), match.captured(2)};
    }

    const int colon = raw.lastIndexOf(QLatin1Char(':'));
    if (colon > 0 && raw.indexOf(QLatin1Char(':')) == colon) {
        const QString maybe_port = raw.mid(colon + 1);
        bool port_ok = false;
        maybe_port.toUShort(&port_ok);
        if (port_ok) return {raw.left(colon), maybe_port};
    }

    return {raw, QString()};
}

QString peerAddressWithPort(const QString& raw, const QPair<QString, QString>& endpoint)
{
    if (!raw.trimmed().isEmpty()) return raw.trimmed();
    if (endpoint.first.isEmpty()) return QStringLiteral("-");
    if (endpoint.second.isEmpty()) return endpoint.first;
    if (endpoint.first.contains(QLatin1Char(':'))) {
        return QStringLiteral("[%1]:%2").arg(endpoint.first, endpoint.second);
    }
    return QStringLiteral("%1:%2").arg(endpoint.first, endpoint.second);
}

QString fallbackDash(QString value)
{
    value = value.trimmed();
    return value.isEmpty() ? QStringLiteral("-") : value;
}

bool isIpLiteral(QString host)
{
    host = host.trimmed();
    if (host.startsWith(QLatin1Char('[')) && host.endsWith(QLatin1Char(']'))) {
        host = host.mid(1, host.size() - 2);
    }
    QHostAddress address;
    return address.setAddress(host);
}

QString normalizedPeerHost(QString host)
{
    host = host.trimmed();
    if (host.startsWith(QLatin1Char('[')) && host.endsWith(QLatin1Char(']'))) {
        host = host.mid(1, host.size() - 2);
    }
    QHostAddress address;
    if (address.setAddress(host)) return address.toString().toLower();
    return host.toLower();
}

QString normalizeDnsName(QString value)
{
    value = value.trimmed();
    while (value.endsWith(QLatin1Char('.'))) value.chop(1);
    return value;
}

QString reverseDnsNameForAddress(const QHostAddress& address)
{
    if (address.protocol() == QAbstractSocket::IPv4Protocol) {
        QStringList octets = address.toString().split(QLatin1Char('.'));
        if (octets.size() != 4) return QString();
        std::reverse(octets.begin(), octets.end());
        return octets.join(QLatin1Char('.')) + QStringLiteral(".in-addr.arpa");
    }

    if (address.protocol() == QAbstractSocket::IPv6Protocol) {
        const Q_IPV6ADDR bytes = address.toIPv6Address();
        QStringList nibbles;
        nibbles.reserve(32);
        for (int i = 15; i >= 0; --i) {
            nibbles << QString::number(bytes[i] & 0x0f, 16);
            nibbles << QString::number((bytes[i] >> 4) & 0x0f, 16);
        }
        return nibbles.join(QLatin1Char('.')) + QStringLiteral(".ip6.arpa");
    }

    return QString();
}

QStringList configuredSeedDomains()
{
    return {
        QStringLiteral("seed.defcoin.io"),
        QStringLiteral("seed.defcoin.mikej.tech"),
        QStringLiteral("seed.defcoin.dc903.org"),
        QStringLiteral("seed.defcoincore.org")
    };
}

int seedDomainPriority(const QString& domain)
{
    const QStringList domains = configuredSeedDomains();
    const int index = domains.indexOf(domain.toLower());
    return index < 0 ? domains.size() : index;
}

QString alignIPv4ForDisplay(const QString& value)
{
    const QString trimmed = value.trimmed();
    static const QRegularExpression ipv4_with_optional_port(QStringLiteral(R"(^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})(:\d+)?$)"));
    const QRegularExpressionMatch match = ipv4_with_optional_port.match(trimmed);
    if (!match.hasMatch()) return trimmed;
    return QStringLiteral("%1.%2.%3.%4%5")
        .arg(match.captured(1).toInt(), 3, 10, QLatin1Char(' '))
        .arg(match.captured(2).toInt(), 3, 10, QLatin1Char(' '))
        .arg(match.captured(3).toInt(), 3, 10, QLatin1Char(' '))
        .arg(match.captured(4).toInt(), 3, 10, QLatin1Char(' '))
        .arg(match.captured(5));
}

QString peerIpDisplay(const QPair<QString, QString>& endpoint)
{
    if (endpoint.first.trimmed().isEmpty()) return QStringLiteral("-");
    return isIpLiteral(endpoint.first) ? alignIPv4ForDisplay(endpoint.first) : QStringLiteral("-");
}

QString peerAddressPortDisplay(const QString& raw, const QPair<QString, QString>& endpoint)
{
    return alignIPv4ForDisplay(peerAddressWithPort(raw, endpoint));
}

QString peerDnsName(const QJsonObject& peer,
                    const QPair<QString, QString>& endpoint,
                    const QHash<QString, QString>& dns_cache)
{
    const QString explicit_dns = fallbackDash(peer.value(QStringLiteral("dns_name")).toString(
        peer.value(QStringLiteral("fqdn")).toString(peer.value(QStringLiteral("addr_name")).toString())));
    if (explicit_dns != QLatin1String("-") && !isIpLiteral(splitPeerAddressAndPort(explicit_dns).first)) return explicit_dns;
    if (!endpoint.first.isEmpty() && !isIpLiteral(endpoint.first)) return endpoint.first;
    const QString cached_dns = dns_cache.value(normalizedPeerHost(endpoint.first));
    if (!cached_dns.isEmpty()) return cached_dns;
    return QStringLiteral("-");
}

QString peerDomainAlias(const QJsonObject& peer,
                        const QPair<QString, QString>& endpoint,
                        const QHash<QString, QString>& alias_cache)
{
    const QString explicit_alias = fallbackDash(peer.value(QStringLiteral("domain_alias")).toString(
        peer.value(QStringLiteral("seed_domain")).toString(peer.value(QStringLiteral("source_domain")).toString())));
    if (explicit_alias != QLatin1String("-")) return explicit_alias;

    const QString alias = alias_cache.value(normalizedPeerHost(endpoint.first));
    if (!alias.isEmpty()) return alias;

    const QString host = endpoint.first.trimmed().toLower();
    if (!host.isEmpty() && configuredSeedDomains().contains(host)) return host;

    return QStringLiteral("-");
}

QString peerNumberText(const QJsonObject& peer, const QString& key)
{
    const QJsonValue value = peer.value(key);
    if (value.isDouble()) return QString::number(value.toVariant().toLongLong());
    if (value.isString()) return fallbackDash(value.toString());
    return QStringLiteral("-");
}

QString peerSyncHeightText(const QJsonObject& peer, const QString& key)
{
    const QJsonValue value = peer.value(key);
    if (value.isDouble()) {
        const qint64 height = value.toVariant().toLongLong();
        return height < 0 ? QStringLiteral("Unknown") : QString::number(height);
    }
    if (value.isString()) {
        const QString text = value.toString().trimmed();
        return text == QLatin1String("-1") ? QStringLiteral("Unknown") : fallbackDash(text);
    }
    return QStringLiteral("-");
}

QString peerBoolText(const QJsonObject& peer, const QString& key)
{
    const QJsonValue value = peer.value(key);
    return value.isBool() ? (value.toBool() ? QStringLiteral("Enabled") : QStringLiteral("Disabled")) : QStringLiteral("-");
}

QString peerUnixTimeText(const QJsonObject& peer, const QString& key)
{
    const QJsonValue value = peer.value(key);
    if (!value.isDouble()) return QStringLiteral("-");
    const qint64 seconds = value.toVariant().toLongLong();
    if (seconds <= 0) return QStringLiteral("-");
    return QDateTime::fromSecsSinceEpoch(seconds).toLocalTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
}

QString boolText(bool value)
{
    return value ? QStringLiteral("Enabled") : QStringLiteral("Disabled");
}

QString normalPurpose(const QString& purpose)
{
    if (purpose == QLatin1String("send")) return QStringLiteral("Contact");
    if (purpose == QLatin1String("receive")) return QStringLiteral("Receive");
    return purpose.isEmpty() ? QStringLiteral("Address") : purpose.left(1).toUpper() + purpose.mid(1);
}

void upsertAddressRow(QVariantList& rows, const QVariantList& candidate)
{
    if (candidate.size() < 2) return;
    const QString address = candidate.at(1).toString();
    for (QVariant& value : rows) {
        QVariantList existing = value.toList();
        if (existing.size() >= 2 && existing.at(1).toString() == address) {
            value = candidate;
            return;
        }
    }
    rows.push_back(candidate);
}

QString percentEncodeWalletName(const QString& wallet_name)
{
    return QString::fromLatin1(QUrl::toPercentEncoding(wallet_name, QByteArrayLiteral("/")));
}

QString realHomePath()
{
#if defined(Q_OS_WIN)
    return QDir::homePath();
#else
    if (const passwd* pw = getpwuid(getuid())) {
        if (pw->pw_dir && *pw->pw_dir) {
            return QString::fromLocal8Bit(pw->pw_dir);
        }
    }
    return QDir::homePath();
#endif
}

QString localLogTimestamp(const QDateTime& utc)
{
    return utc.toLocalTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss t"));
}

bool parseDebugLogTimestamp(const QString& line, QDateTime& utc, QString& message)
{
    static const QRegularExpression re(QStringLiteral(R"(^(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2})Z\s*(.*)$)"));
    const QRegularExpressionMatch match = re.match(line);
    if (!match.hasMatch()) return false;
    utc = QDateTime::fromString(match.captured(1), QStringLiteral("yyyy-MM-dd'T'HH:mm:ss"));
    utc.setTimeZone(QTimeZone::UTC);
    message = match.captured(2);
    return utc.isValid();
}

bool isBackendTransportError(QNetworkReply::NetworkError error)
{
    switch (error) {
    case QNetworkReply::ConnectionRefusedError:
    case QNetworkReply::RemoteHostClosedError:
    case QNetworkReply::HostNotFoundError:
    case QNetworkReply::TimeoutError:
    case QNetworkReply::NetworkSessionFailedError:
    case QNetworkReply::TemporaryNetworkFailureError:
        return true;
    default:
        return false;
    }
}

QString formatLocalDebugLogLine(const QString& line)
{
    QDateTime utc;
    QString message;
    if (!parseDebugLogTimestamp(line, utc, message)) return line;
    return localLogTimestamp(utc) + QStringLiteral(" ") + message;
}

QString variantColumnType(const QVariantList& column_types, const QVariantList& columns, int index)
{
    if (index >= 0 && index < column_types.size()) {
        const QString explicit_type = column_types.at(index).toString().trimmed().toLower();
        if (!explicit_type.isEmpty()) return explicit_type;
    }
    const QString name = index >= 0 && index < columns.size() ? columns.at(index).toString().toLower() : QString();
    if (name.contains(QStringLiteral("date")) || name.contains(QStringLiteral("time"))) return QStringLiteral("date");
    if (name.contains(QStringLiteral("amount")) || name.contains(QStringLiteral("balance"))) return QStringLiteral("amount");
    if (name.contains(QStringLiteral("sent")) || name.contains(QStringLiteral("rec"))) return QStringLiteral("bytes");
    if (name.contains(QStringLiteral("ping"))) return QStringLiteral("duration");
    if (name.contains(QStringLiteral("id")) || name.contains(QStringLiteral("peers")) || name.contains(QStringLiteral("block"))) return QStringLiteral("number");
    if (name.contains(QStringLiteral("address")) || name.contains(QStringLiteral("ip")) || name.contains(QStringLiteral("port"))) return QStringLiteral("ipport");
    return QStringLiteral("text");
}

double variantAt(const QVariantList& values, int index, double fallback)
{
    if (index < 0 || index >= values.size()) return fallback;
    bool ok = false;
    const double value = values.at(index).toDouble(&ok);
    return ok ? value : fallback;
}

double defaultColumnMinimum(const QString& type)
{
    if (type == QLatin1String("action") || type == QLatin1String("delete")) return 44.0;
    if (type == QLatin1String("ipport") || type == QLatin1String("address") || type == QLatin1String("hash")) return 210.0;
    if (type == QLatin1String("date")) return 132.0;
    if (type == QLatin1String("bytes") || type == QLatin1String("amount")) return 96.0;
    if (type == QLatin1String("duration")) return 78.0;
    if (type == QLatin1String("number")) return 64.0;
    return 90.0;
}

double defaultColumnMaximum(const QString& type)
{
    if (type == QLatin1String("action") || type == QLatin1String("delete")) return 44.0;
    if (type == QLatin1String("ipport")) return 520.0;
    if (type == QLatin1String("address") || type == QLatin1String("hash")) return 520.0;
    if (type == QLatin1String("text")) return 360.0;
    return 260.0;
}

bool monoColumnType(const QString& type, const QString& name = QString())
{
    const QString lower = name.toLower();
    return type == QLatin1String("ipport") || type == QLatin1String("address") || type == QLatin1String("hash")
        || type == QLatin1String("number") || type == QLatin1String("bytes") || type == QLatin1String("duration")
        || type == QLatin1String("date") || lower == QLatin1String("port") || lower.contains(QStringLiteral("magic"))
        || lower.contains(QStringLiteral("version")) || lower == QLatin1String("svcs") || lower.contains(QStringLiteral("height"))
        || lower.contains(QStringLiteral("headers")) || lower.contains(QStringLiteral("blocks"));
}
} // namespace

NuRpcService::NuRpcService(QObject* parent)
    : QObject(parent),
      m_network(new QNetworkAccessManager(this))
{
    m_app_launch_utc = QDateTime::currentDateTimeUtc();
    loadLocalSettings();
    rebuildNodeMetrics();
    connect(m_network, &QNetworkAccessManager::finished, this, &NuRpcService::handleReply);
    m_uptime.start();

    m_refresh_timer = new QTimer(this);
    m_refresh_timer->setInterval(5000);
    connect(m_refresh_timer, &QTimer::timeout, this, &NuRpcService::refresh);
    m_refresh_timer->start();

    m_traffic_timer = new QTimer(this);
    m_traffic_timer->setInterval(1000);
    connect(m_traffic_timer, &QTimer::timeout, this, &NuRpcService::sampleTraffic);
    m_traffic_timer->start();

    QTimer::singleShot(0, this, &NuRpcService::refresh);
    QTimer::singleShot(250, this, &NuRpcService::sampleTraffic);
}

NuRpcService::~NuRpcService()
{
    stopOwnedBackend();
}

void NuRpcService::loadLocalSettings()
{
    QSettings legacy_settings(QStringLiteral("Defcoin"), QStringLiteral("Defcoin-Qt"));
    m_only_defcoin_user_agents = legacy_settings.value(QStringLiteral("OnlyDefcoinUserAgents"), true).toBool();
    const QString legacy_explorer_url = legacy_settings.value(QStringLiteral("strThirdPartyTxUrls"), QString()).toString();

    QSettings nu_settings;
    m_mask_balances = nu_settings.value(QStringLiteral("MaskBalances"), false).toBool();
    m_only_defcoin_magic_bytes = nu_settings.value(
        QStringLiteral("OnlyDefcoinMagicBytes"),
        !nu_settings.value(QStringLiteral("LegacyMagicEnabled"), true).toBool()).toBool();
    m_switch_to_defcoin_only_magic_starting_july_2026 = nu_settings.value(QStringLiteral("SwitchToDefcoinOnlyMagicStarting20260701"), true).toBool();
    if (m_switch_to_defcoin_only_magic_starting_july_2026 && QDate::currentDate() >= QDate(2026, 7, 1)) {
        m_only_defcoin_magic_bytes = true;
        nu_settings.setValue(QStringLiteral("OnlyDefcoinMagicBytes"), true);
    }
    m_disallow_lan_node_discovery = nu_settings.value(QStringLiteral("DisallowLanNodeDiscovery"), false).toBool();
    m_third_party_tx_urls_enabled = nu_settings.value(QStringLiteral("ThirdPartyTxUrlsEnabled"), false).toBool();
    m_third_party_tx_url = normalizedExplorerUrl(nu_settings.value(QStringLiteral("ThirdPartyTxUrl"), legacy_explorer_url).toString());
}

QString NuRpcService::defaultDataDir() const
{
    if (!qEnvironmentVariableIsEmpty("DEFCOIN_DATADIR")) {
        return QString::fromLocal8Bit(qgetenv("DEFCOIN_DATADIR"));
    }
#if defined(Q_OS_WIN)
    const QString appdata = QString::fromLocal8Bit(qgetenv("APPDATA"));
    return QDir(appdata).filePath(QStringLiteral("Defcoin"));
#elif defined(Q_OS_MACOS)
    return QDir(realHomePath()).filePath(QStringLiteral("Library/Application Support/Defcoin"));
#else
    return QDir(realHomePath()).filePath(QStringLiteral(".defcoin"));
#endif
}

void NuRpcService::readDefcoinConf(const QString& conf_path)
{
    QFile file(conf_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#'))) continue;
        const int eq = line.indexOf(QLatin1Char('='));
        if (eq < 0) continue;
        const QString key = line.left(eq).trimmed().toLower();
        const QString value = line.mid(eq + 1).trimmed();
        if (key == QLatin1String("rpcconnect")) m_rpc_host = value;
        if (key == QLatin1String("rpcport")) m_rpc_port = value.toInt();
        if (key == QLatin1String("rpcuser")) m_rpc_user = value;
        if (key == QLatin1String("rpcpassword")) m_rpc_password = value;
        if (key == QLatin1String("blockexplorer") && m_third_party_tx_url.trimmed().isEmpty()) {
            m_third_party_tx_url = normalizedExplorerUrl(value);
        }
    }
}

bool NuRpcService::readCookie()
{
    QFile file(QDir(m_data_dir).filePath(QStringLiteral(".cookie")));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    const QString cookie = QString::fromUtf8(file.readAll()).trimmed();
    const int colon = cookie.indexOf(QLatin1Char(':'));
    if (colon <= 0) return false;
    m_rpc_user = cookie.left(colon);
    m_rpc_password = cookie.mid(colon + 1);
    return true;
}

QString NuRpcService::backendBinaryPath() const
{
    if (!qEnvironmentVariableIsEmpty("DEFCOIN_BACKEND_PATH")) {
        const QString path = QString::fromLocal8Bit(qgetenv("DEFCOIN_BACKEND_PATH"));
        if (QFileInfo::exists(path) && QFileInfo(path).isExecutable()) return path;
    }

    const QString app_dir = QCoreApplication::applicationDirPath();
#if defined(Q_OS_MACOS)
    const QString bundled = QDir(app_dir).filePath(QStringLiteral("../Resources/nu/bin/defcoind"));
#elif defined(Q_OS_WIN)
    const QString bundled = QDir(app_dir).filePath(QStringLiteral("nu/bin/defcoind.exe"));
#else
    const QString bundled = QDir(app_dir).filePath(QStringLiteral("nu/bin/defcoind"));
#endif
    if (QFileInfo::exists(bundled) && QFileInfo(bundled).isExecutable()) return bundled;

    const QString sibling = QDir(app_dir).filePath(
#if defined(Q_OS_WIN)
        QStringLiteral("defcoind.exe")
#else
        QStringLiteral("defcoind")
#endif
    );
    if (QFileInfo::exists(sibling) && QFileInfo(sibling).isExecutable()) return sibling;

    return QString();
}

void NuRpcService::appendLaunchDiagnostic(const QString& message)
{
    if (message.trimmed().isEmpty()) return;
    if (!m_launch_diagnostics_section_started) {
        const QString marker = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss t"))
            + QStringLiteral(" ----- Nu startup diagnostics -----");
        m_log_lines.push_back(marker);
        appendDebugLogLineFromNu(QStringLiteral("----- Nu startup diagnostics -----"));
        m_launch_diagnostics_section_started = true;
    }
    const QString line = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss t"))
        + QStringLiteral(" Nu startup: ") + message;
    if (m_log_lines.isEmpty() || m_log_lines.constLast() != line) {
        m_log_lines.push_back(line);
    }
    appendDebugLogLineFromNu(QStringLiteral("Nu startup: %1").arg(message));
    while (m_log_lines.size() > 5000) m_log_lines.removeFirst();
    rebuildNodeMetrics();
    Q_EMIT logChanged();
    Q_EMIT stateChanged();
}

void NuRpcService::appendDebugLogLineFromNu(const QString& message)
{
    if (message.trimmed().isEmpty()) return;

    const QString path = debugLogPath();
    const QFileInfo info(path);
    QDir().mkpath(info.absolutePath());

    QFile file(path);
    if (!file.open(QIODevice::Append | QIODevice::Text)) return;

    QTextStream out(&file);
    out << QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyy-MM-ddTHH:mm:ssZ"))
        << ' ' << message << '\n';
}

void NuRpcService::beginBackendDebugLogSection(bool write_to_debug_log)
{
    if (m_backend_log_section_started) return;

    const QString marker = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss t"))
        + QStringLiteral(" ----- Backend debug.log -----");
    if (m_log_lines.isEmpty() || m_log_lines.constLast() != marker) {
        m_log_lines.push_back(marker);
    }
    if (write_to_debug_log) {
        appendDebugLogLineFromNu(QStringLiteral("----- Backend debug.log follows -----"));
    }
    m_backend_log_section_started = true;
}

QStringList NuRpcService::backendRuntimeDiagnostics(const QString& binary) const
{
    QStringList diagnostics;
    const QDir backend_dir(QFileInfo(binary).absolutePath());
    diagnostics << QStringLiteral("Backend path: %1").arg(QDir::toNativeSeparators(binary));
    diagnostics << QStringLiteral("Backend working directory: %1").arg(QDir::toNativeSeparators(backend_dir.absolutePath()));
    diagnostics << QStringLiteral("Data directory: %1").arg(QDir::toNativeSeparators(m_data_dir.isEmpty() ? defaultDataDir() : m_data_dir));
    diagnostics << QStringLiteral("Debug log path: %1").arg(QDir::toNativeSeparators(QDir(m_data_dir.isEmpty() ? defaultDataDir() : m_data_dir).filePath(QStringLiteral("debug.log"))));
#if defined(Q_OS_WIN)
    const QFileInfoList runtime_files = backend_dir.entryInfoList({QStringLiteral("*.dll")}, QDir::Files, QDir::Name);
    QStringList runtime_names;
    for (const QFileInfo& runtime_file : runtime_files) {
        runtime_names << runtime_file.fileName();
    }
    diagnostics << QStringLiteral("Backend runtime DLLs in nu/bin: %1")
        .arg(runtime_names.isEmpty() ? QStringLiteral("none bundled") : runtime_names.join(QStringLiteral(", ")));
    if (runtime_names.isEmpty()) {
        diagnostics << QStringLiteral("Backend runtime note: bundled defcoind may be static apart from Windows system and UCRT libraries.");
    }
#endif
    return diagnostics;
}

bool NuRpcService::ensureBackendStarted()
{
    if (m_backend_start_attempted || !qEnvironmentVariableIsEmpty("DEFCOIN_NU_NO_BACKEND_AUTOSTART")) {
        return false;
    }

    appendLaunchDiagnostic(QStringLiteral("Backend autostart requested."));
    const QString binary = backendBinaryPath();
    if (binary.isEmpty()) {
        const QString message = QStringLiteral("Bundled Defcoin backend was not found. Expected a defcoind executable inside this app.");
        appendLaunchDiagnostic(message);
        m_last_error = message;
        rebuildNodeMetrics();
        Q_EMIT stateChanged();
        return false;
    }

    QDir().mkpath(m_data_dir);
    for (const QString& diagnostic : backendRuntimeDiagnostics(binary)) {
        appendLaunchDiagnostic(diagnostic);
    }

    const QString working_dir = QFileInfo(binary).absolutePath();
    QProcess version_probe;
    version_probe.setWorkingDirectory(working_dir);
    version_probe.start(binary, {QStringLiteral("-version")});
    if (!version_probe.waitForStarted(3000)) {
        const QString message = QStringLiteral("Defcoin backend preflight failed to start: %1").arg(version_probe.errorString());
        m_backend_start_attempted = true;
        m_last_error = message;
        appendLaunchDiagnostic(message);
        return false;
    }
    if (!version_probe.waitForFinished(5000)) {
        version_probe.kill();
        version_probe.waitForFinished(1000);
        appendLaunchDiagnostic(QStringLiteral("Defcoin backend preflight timed out while running -version."));
    } else {
        const QString output = QString::fromUtf8(version_probe.readAllStandardOutput()).trimmed();
        const QString error_output = QString::fromUtf8(version_probe.readAllStandardError()).trimmed();
        if (version_probe.exitStatus() != QProcess::NormalExit || version_probe.exitCode() != 0) {
            const QString message = QStringLiteral("Defcoin backend preflight exited with code %1. %2%3")
                .arg(version_probe.exitCode())
                .arg(output.left(700))
                .arg(error_output.isEmpty() ? QString() : QStringLiteral(" ") + error_output.left(700));
            m_backend_start_attempted = true;
            m_last_error = message;
            appendLaunchDiagnostic(message);
            return false;
        }
        appendLaunchDiagnostic(output.isEmpty()
            ? QStringLiteral("Defcoin backend -version preflight succeeded.")
            : QStringLiteral("Defcoin backend -version preflight: %1").arg(output.section(QLatin1Char('\n'), 0, 0)));
    }

    QStringList args;
    args << QStringLiteral("-datadir=%1").arg(QDir::toNativeSeparators(m_data_dir))
         << QStringLiteral("-debuglogfile=%1").arg(QDir::toNativeSeparators(QDir(m_data_dir).filePath(QStringLiteral("debug.log"))))
         << QStringLiteral("-server=1")
         << QStringLiteral("-listen=1")
         << QStringLiteral("-networkactive=1")
         << QStringLiteral("-acceptlegacymagic=%1").arg(m_only_defcoin_magic_bytes ? 0 : 1)
         << QStringLiteral("-allowlannodediscovery=%1").arg(m_disallow_lan_node_discovery ? 0 : 1)
         << QStringLiteral("-rpcport=%1").arg(m_rpc_port)
         << QStringLiteral("-rpcbind=127.0.0.1")
         << QStringLiteral("-rpcallowip=127.0.0.1");

    args << QStringLiteral("-seednode=seed.defcoin.io")
         << QStringLiteral("-seednode=seed.defcoin.mikej.tech")
         << QStringLiteral("-seednode=seed.defcoin.dc903.org:10332")
         << QStringLiteral("-seednode=seed.defcoincore.org");

    if (m_backend_process == nullptr) {
        m_backend_process = new QProcess(this);
        m_backend_process->setProcessChannelMode(QProcess::SeparateChannels);
        m_backend_process->setStandardOutputFile(QProcess::nullDevice());
        m_backend_process->setStandardErrorFile(QProcess::nullDevice());
        connect(m_backend_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
            Q_UNUSED(error);
            if (!m_backend_started_by_nu) return;
            appendLaunchDiagnostic(QStringLiteral("Defcoin backend process error: %1").arg(m_backend_process->errorString()));
        });
        connect(m_backend_process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, [this](int exit_code, QProcess::ExitStatus status) {
            if (!m_backend_started_by_nu) return;
            appendLaunchDiagnostic(QStringLiteral("Defcoin backend process exited with code %1%2.")
                .arg(exit_code)
                .arg(status == QProcess::CrashExit ? QStringLiteral(" after a crash") : QString()));
            m_backend_started_by_nu = false;
            m_backend_pid = 0;
            Q_EMIT stateChanged();
        });
    }
    m_backend_process->setWorkingDirectory(working_dir);
    m_backend_process->setProgram(binary);
    m_backend_process->setArguments(args);
    m_backend_process->start();
    const bool started = m_backend_process->waitForStarted(5000);
    const qint64 pid = started ? m_backend_process->processId() : 0;
    m_backend_start_attempted = true;

    if (started) {
        m_backend_started_by_nu = true;
        m_backend_pid = pid;
        m_connection_status = QStringLiteral("Starting backend");
        m_metric_network_active = QStringLiteral("Starting");
        m_last_error = QStringLiteral("Starting Defcoin backend from %1%2")
            .arg(QFileInfo(binary).fileName(),
                 pid > 0 ? QStringLiteral(" (pid %1)").arg(pid) : QString());
        appendLaunchDiagnostic(m_last_error);
        beginBackendDebugLogSection(true);
        Q_EMIT logChanged();
        rebuildNodeMetrics();
        QTimer::singleShot(1500, this, &NuRpcService::refresh);
        Q_EMIT stateChanged();
        return true;
    }

    const QString process_error = m_backend_process ? m_backend_process->errorString() : QString();
    const QString message = QStringLiteral("Defcoin backend launch failed from %1. %2Check executable permissions and bundled runtime libraries.")
                                .arg(QDir::toNativeSeparators(binary),
                                     process_error.isEmpty() ? QString() : process_error + QStringLiteral(" "));
    m_last_error = message;
    appendLaunchDiagnostic(message);
    rebuildNodeMetrics();
    Q_EMIT stateChanged();
    return false;
}

bool NuRpcService::loadRpcSettings()
{
    m_data_dir = defaultDataDir();
    m_rpc_host = qEnvironmentVariableIsEmpty("DEFCOIN_RPC_HOST") ? QStringLiteral("127.0.0.1") : QString::fromLocal8Bit(qgetenv("DEFCOIN_RPC_HOST"));
    m_rpc_port = qEnvironmentVariableIsEmpty("DEFCOIN_RPC_PORT") ? 9332 : QString::fromLocal8Bit(qgetenv("DEFCOIN_RPC_PORT")).toInt();
    m_rpc_user = QString::fromLocal8Bit(qgetenv("DEFCOIN_RPC_USER"));
    m_rpc_password = QString::fromLocal8Bit(qgetenv("DEFCOIN_RPC_PASSWORD"));

    readDefcoinConf(QDir(m_data_dir).filePath(QStringLiteral("defcoin.conf")));
    loadReceiveRequests();
    if ((m_rpc_user.isEmpty() || m_rpc_password.isEmpty()) && !readCookie()) {
        if (ensureBackendStarted()) {
            setError(QStringLiteral("Starting Defcoin backend. The wallet will connect when RPC credentials are ready."));
            return false;
        }
        if (m_backend_started_by_nu && m_backend_process && m_backend_process->state() != QProcess::NotRunning) {
            setError(QStringLiteral("Starting Defcoin backend. The wallet will connect when RPC credentials are ready."));
            return false;
        }
        setError(QStringLiteral("RPC credentials not found. Start Defcoin Core with RPC enabled or set DEFCOIN_RPC_USER / DEFCOIN_RPC_PASSWORD."));
        return false;
    }
    return true;
}

bool NuRpcService::shouldAutostartAfterTransportError(QNetworkReply* reply) const
{
    return isBackendTransportError(reply->error()) &&
        !m_backend_start_attempted &&
        qEnvironmentVariableIsEmpty("DEFCOIN_NU_NO_BACKEND_AUTOSTART");
}

QUrl NuRpcService::rpcUrl(bool wallet_scoped) const
{
    QUrl url;
    url.setScheme(QStringLiteral("http"));
    url.setHost(m_rpc_host);
    url.setPort(m_rpc_port);
    if (wallet_scoped && !m_wallet_name.isEmpty()) {
        url.setPath(QStringLiteral("/wallet/%1").arg(percentEncodeWalletName(m_wallet_name)));
    }
    return url;
}

void NuRpcService::rpcCall(const QString& method, const QJsonArray& params, bool wallet_scoped, RpcCallback callback)
{
    if (!loadRpcSettings()) {
        callback(QJsonValue(), m_last_error);
        return;
    }

    const int id = m_next_id++;
    QJsonObject request_obj;
    request_obj.insert(QStringLiteral("jsonrpc"), QStringLiteral("1.0"));
    request_obj.insert(QStringLiteral("id"), id);
    request_obj.insert(QStringLiteral("method"), method);
    request_obj.insert(QStringLiteral("params"), params);

    QNetworkRequest request(rpcUrl(wallet_scoped));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    const QByteArray auth = QStringLiteral("%1:%2").arg(m_rpc_user, m_rpc_password).toUtf8().toBase64();
    request.setRawHeader("Authorization", "Basic " + auth);

    m_pending.insert(id, PendingCall{method, std::move(callback)});
    QNetworkReply* reply = m_network->post(request, QJsonDocument(request_obj).toJson(QJsonDocument::Compact));
    reply->setProperty("nuRpcId", id);
}

void NuRpcService::handleReply(QNetworkReply* reply)
{
    const QByteArray body = reply->readAll();
    reply->deleteLater();

    bool property_id_ok = false;
    int id = reply->property("nuRpcId").toInt(&property_id_ok);
    const QJsonDocument doc = QJsonDocument::fromJson(body);
    const QJsonObject obj = doc.object();
    if (!property_id_ok) {
        id = obj.value(QStringLiteral("id")).toInt();
    }
    PendingCall call = m_pending.take(id);
    if (!call.callback) return;

    QString error;
    if (reply->error() != QNetworkReply::NoError) {
        error = reply->errorString();
        if (!body.isEmpty()) {
            const QJsonValue rpc_error = obj.value(QStringLiteral("error"));
            if (rpc_error.isObject()) {
                error = rpc_error.toObject().value(QStringLiteral("message")).toString(error);
            }
        }
        if (shouldAutostartAfterTransportError(reply) && ensureBackendStarted()) {
            error = QStringLiteral("Starting Defcoin backend. The wallet will connect when RPC is ready.");
            QTimer::singleShot(2500, this, &NuRpcService::refresh);
        } else if (m_backend_start_attempted && !m_rpc_connected && isBackendTransportError(reply->error())) {
            error = QStringLiteral("Starting Defcoin backend. The wallet will connect when RPC is ready.");
            QTimer::singleShot(2500, this, &NuRpcService::refresh);
        }
    } else if (!obj.value(QStringLiteral("error")).isNull()) {
        const QJsonValue rpc_error = obj.value(QStringLiteral("error"));
        error = rpc_error.isObject() ? rpc_error.toObject().value(QStringLiteral("message")).toString() : QString::fromUtf8(QJsonDocument(rpc_error.toObject()).toJson());
    }

    call.callback(obj.value(QStringLiteral("result")), error);
}

void NuRpcService::setError(const QString& message)
{
    m_rpc_connected = false;
    m_connection_status = QStringLiteral("RPC not connected");
    m_last_error = message;
    if (!message.isEmpty()) {
        rebuildNodeMetrics();
        m_node_metrics.push_front(row({"Detail", message}));
        m_node_metrics.push_front(row({"Backend", m_connection_status}));
        m_node_metrics.push_back(row({"Data directory", m_data_dir.isEmpty() ? defaultDataDir() : m_data_dir}));
    }
    const bool log_message = !message.startsWith(QStringLiteral("Starting Defcoin backend"));
    if (log_message && !message.isEmpty() && m_last_logged_error_message != message) {
        m_log_lines.push_back(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss t")) + QStringLiteral(" ") + message);
        m_last_logged_error_message = message;
        while (m_log_lines.size() > 5000) m_log_lines.removeFirst();
        Q_EMIT logChanged();
    }
    Q_EMIT stateChanged();
}

void NuRpcService::clearError()
{
    m_rpc_connected = true;
    m_connection_status = QStringLiteral("RPC connected");
    m_last_error.clear();
    m_last_logged_error_message.clear();
    Q_EMIT stateChanged();
    if (m_have_pending_network_active && !m_applying_pending_network_active) {
        m_applying_pending_network_active = true;
        const bool active = m_pending_network_active;
        QTimer::singleShot(0, this, [this, active] {
            setNetworkActive(active);
        });
    }
}

void NuRpcService::refresh()
{
    refreshNode();
    refreshWallet();
    refreshAddressBook();
    refreshFeeEstimate();
    refreshDebugLog();
}

void NuRpcService::schedulePeerNameLookups(const QString& host)
{
    QHostAddress address;
    if (!address.setAddress(host.trimmed())) return;

    scheduleConfiguredSeedAliasLookups();

    const QString key = normalizedPeerHost(host);
    if (m_peer_dns_name_by_host.contains(key) ||
        m_peer_reverse_lookup_pending.contains(key) ||
        m_peer_reverse_lookup_attempted.contains(key)) {
        return;
    }

    const QString ptr_name = reverseDnsNameForAddress(address);
    if (ptr_name.isEmpty()) return;

    m_peer_reverse_lookup_pending.insert(key);
    m_peer_reverse_lookup_attempted.insert(key);
    QDnsLookup* dns = new QDnsLookup(QDnsLookup::PTR, ptr_name, this);
    connect(dns, &QDnsLookup::finished, this, [this, dns, key] {
        m_peer_reverse_lookup_pending.remove(key);

        if (dns->error() == QDnsLookup::NoError && !dns->pointerRecords().isEmpty()) {
            const QString name = normalizeDnsName(dns->pointerRecords().constFirst().value());
            if (!name.isEmpty() && !isIpLiteral(name) && m_peer_dns_name_by_host.value(key) != name) {
                m_peer_dns_name_by_host.insert(key, name);
                refreshNode();
            }
        }

        dns->deleteLater();
    });
    dns->lookup();
}

void NuRpcService::scheduleConfiguredSeedAliasLookups()
{
    if (m_seed_alias_lookups_started) return;
    m_seed_alias_lookups_started = true;

    for (const QString& domain : configuredSeedDomains()) {
        QHostInfo::lookupHost(domain, this, [this, domain](const QHostInfo& info) {
            if (info.error() != QHostInfo::NoError) return;

            bool changed = false;
            const int priority = seedDomainPriority(domain);
            for (const QHostAddress& address : info.addresses()) {
                const QString key = normalizedPeerHost(address.toString());
                const int current_priority = m_peer_domain_alias_priority_by_host.value(key, std::numeric_limits<int>::max());
                if (priority < current_priority) {
                    m_peer_domain_alias_by_host.insert(key, domain);
                    m_peer_domain_alias_priority_by_host.insert(key, priority);
                    changed = true;
                }
            }

            if (changed) refreshNode();
        });
    }
}

void NuRpcService::refreshNode()
{
    rpcCall(QStringLiteral("getonlydefcoinuseragents"), {}, false, [this](const QJsonValue& result, const QString& error) {
        if (error.isEmpty() && result.isBool() && m_only_defcoin_user_agents != result.toBool()) {
            m_only_defcoin_user_agents = result.toBool();
            QSettings(QStringLiteral("Defcoin"), QStringLiteral("Defcoin-Qt")).setValue(QStringLiteral("OnlyDefcoinUserAgents"), m_only_defcoin_user_agents);
            Q_EMIT settingsChanged();
        }
    });

    rpcCall(QStringLiteral("listwallets"), {}, false, [this](const QJsonValue& result, const QString& error) {
        if (error.isEmpty() && result.isArray() && !result.toArray().isEmpty()) {
            const QString wallet_name = result.toArray().first().toString();
            if (m_wallet_name != wallet_name) {
                m_wallet_name = wallet_name;
                loadReceiveRequests();
            }
        }
    });

    rpcCall(QStringLiteral("getnetworkinfo"), {}, false, [this](const QJsonValue& result, const QString& error) {
        if (!error.isEmpty()) {
            setError(error);
            return;
        }
        clearError();
        const QJsonObject info = result.toObject();
        const bool active = info.value(QStringLiteral("networkactive")).toBool();
        m_peer_count = info.value(QStringLiteral("connections")).toInt();
        m_network_state = active ? QStringLiteral("connected") : QStringLiteral("isolated");
        m_metric_network_active = boolText(active);
        m_metric_connections = QString::number(m_peer_count);
        m_metric_inbound = QString::number(info.value(QStringLiteral("connections_in")).toInt());
        m_metric_outbound = QString::number(info.value(QStringLiteral("connections_out")).toInt());
        m_metric_version = info.value(QStringLiteral("subversion")).toString();
        rebuildNodeMetrics();
        Q_EMIT stateChanged();
    });

    rpcCall(QStringLiteral("getblockchaininfo"), {}, false, [this](const QJsonValue& result, const QString& error) {
        if (!error.isEmpty()) return;
        const QJsonObject chain = result.toObject();
        m_block_height = chain.value(QStringLiteral("blocks")).toInt();
        const int headers = chain.value(QStringLiteral("headers")).toInt();
        const bool ibd = chain.value(QStringLiteral("initialblockdownload")).toBool();
        m_sync_state = ibd ? QStringLiteral("Syncing") : QStringLiteral("Up to date");
        m_metric_blocks = QString::number(m_block_height);
        m_metric_headers = QString::number(headers);
        m_metric_verification = QString::number(chain.value(QStringLiteral("verificationprogress")).toDouble() * 100.0, 'f', 2) + QStringLiteral("%");
        rebuildNodeMetrics();
        Q_EMIT stateChanged();
    });

    rpcCall(QStringLiteral("getnettotals"), {}, false, [this](const QJsonValue& result, const QString& error) {
        if (!error.isEmpty()) return;
        const QJsonObject totals = result.toObject();
        m_traffic_received_total = formatBytes(totals.value(QStringLiteral("totalbytesrecv")).toVariant().toLongLong());
        m_traffic_sent_total = formatBytes(totals.value(QStringLiteral("totalbytessent")).toVariant().toLongLong());
        m_metric_traffic = m_traffic_received_total + QStringLiteral(" received / ") + m_traffic_sent_total + QStringLiteral(" sent");
        rebuildNodeMetrics();
        Q_EMIT stateChanged();
        Q_EMIT trafficChanged();
    });

    rpcCall(QStringLiteral("getpeerinfo"), {}, false, [this](const QJsonValue& result, const QString& error) {
        if (!error.isEmpty()) return;
        QVariantList simple_rows;
        QVariantList detailed_rows;
        for (const QJsonValue& peer_value : result.toArray()) {
            const QJsonObject peer = peer_value.toObject();
            const QString subver = trimUserAgent(peer.value(QStringLiteral("subver")).toString());
            if (m_only_defcoin_user_agents && !isDefcoinUserAgent(subver)) continue;
            const QString raw_addr = peer.value(QStringLiteral("addr")).toString();
            const QPair<QString, QString> endpoint = splitPeerAddressAndPort(peer.value(QStringLiteral("addr")).toString());
            schedulePeerNameLookups(endpoint.first);
            const QString node_id = QString::number(peer.value(QStringLiteral("id")).toInt());
            const QString direction = peer.value(QStringLiteral("inbound")).toBool() ? QStringLiteral("In") : QStringLiteral("Out");
            const QString ping = formatPing(peer.value(QStringLiteral("pingtime")));
            const QString min_ping = formatPing(peer.value(QStringLiteral("minping")));
            const QString sent = formatBytes(peer.value(QStringLiteral("bytessent")).toVariant().toLongLong());
            const QString received = formatBytes(peer.value(QStringLiteral("bytesrecv")).toVariant().toLongLong());
            const QString magic = fallbackDash(peer.value(QStringLiteral("p2p_magic")).toString(
                peer.value(QStringLiteral("magic")).toString(QStringLiteral("pending"))));
            const QString protocol_version = peerNumberText(peer, QStringLiteral("version"));
            const QString services = formatServices(peer.value(QStringLiteral("services")).toString());

            simple_rows.push_back(row({
                node_id,
                direction,
                peerAddressPortDisplay(raw_addr, endpoint),
                ping,
                sent,
                received,
                subver
            }));

            detailed_rows.push_back(row({
                node_id,
                direction,
                peerIpDisplay(endpoint),
                fallbackDash(endpoint.second),
                peerDnsName(peer, endpoint, m_peer_dns_name_by_host),
                peerDomainAlias(peer, endpoint, m_peer_domain_alias_by_host),
                protocol_version,
                magic,
                services,
                ping,
                min_ping,
                sent,
                received,
                subver,
                peerUnixTimeText(peer, QStringLiteral("conntime")),
                peerNumberText(peer, QStringLiteral("startingheight")),
                peerUnixTimeText(peer, QStringLiteral("lastsend")),
                peerUnixTimeText(peer, QStringLiteral("lastrecv")),
                peerUnixTimeText(peer, QStringLiteral("last_transaction")),
                peerUnixTimeText(peer, QStringLiteral("last_block")),
                peerSyncHeightText(peer, QStringLiteral("synced_headers")),
                peerSyncHeightText(peer, QStringLiteral("synced_blocks")),
                fallbackDash(peer.value(QStringLiteral("connection_type")).toString()),
                fallbackDash(peer.value(QStringLiteral("network")).toString()),
                peerNumberText(peer, QStringLiteral("addr_processed")),
                peer.value(QStringLiteral("minfeefilter")).isDouble()
                    ? formatAmount(peer.value(QStringLiteral("minfeefilter"))) + QStringLiteral(" DFC/kB")
                    : QStringLiteral("-")
            }));
        }
        m_peer_rows_simple = simple_rows;
        m_peer_rows_detailed = detailed_rows;
        m_peers = detailed_rows;
        m_peer_count = detailed_rows.size();
        Q_EMIT peersChanged();
        Q_EMIT stateChanged();
    });
}

void NuRpcService::refreshWallet()
{
    rpcCall(QStringLiteral("getwalletinfo"), {}, true, [this](const QJsonValue& result, const QString& error) {
        if (!error.isEmpty()) {
            m_wallet_locked = true;
            Q_EMIT walletChanged();
            return;
        }
        const QJsonObject wallet = result.toObject();
        const double available = wallet.value(QStringLiteral("balance")).toDouble();
        const double pending = wallet.value(QStringLiteral("unconfirmed_balance")).toDouble();
        const double immature = wallet.value(QStringLiteral("immature_balance")).toDouble();
        const double total = available + pending + immature;
        m_available_balance = QString::number(available, 'f', 8);
        m_pending_balance = QString::number(pending, 'f', 8);
        m_immature_balance = QString::number(immature, 'f', 8);
        m_total_balance = QString::number(total, 'f', 8) + QStringLiteral(" DFC");
        const QJsonValue unlocked_until = wallet.value(QStringLiteral("unlocked_until"));
        m_wallet_encrypted = wallet.contains(QStringLiteral("unlocked_until"));
        m_wallet_locked = m_wallet_encrypted ? unlocked_until.toDouble() == 0 : false;
        Q_EMIT walletChanged();
    });

    rpcCall(QStringLiteral("listtransactions"), {QStringLiteral("*"), 25, 0, true}, true, [this](const QJsonValue& result, const QString& error) {
        if (!error.isEmpty()) return;
        QVariantList rows;
        for (const QJsonValue& tx_value : result.toArray()) {
            const QJsonObject tx = tx_value.toObject();
            const qint64 time = tx.value(QStringLiteral("time")).toVariant().toLongLong();
            const QString amount = QString::number(tx.value(QStringLiteral("amount")).toDouble(), 'f', 8) + QStringLiteral(" DFC");
            const QString category = tx.value(QStringLiteral("category")).toString();
            QString display_category = category;
            if (category == QLatin1String("send")) display_category = QStringLiteral("Sent");
            if (category == QLatin1String("receive")) display_category = QStringLiteral("Received");
            if (category == QLatin1String("generate") || category == QLatin1String("immature") || category == QLatin1String("orphan")) {
                display_category = QStringLiteral("Mined");
            }
            const QString txid = tx.value(QStringLiteral("txid")).toString();
            const QString address = tx.value(QStringLiteral("address")).toString();
            QVariantMap meta;
            meta.insert(QStringLiteral("kind"), QStringLiteral("transaction"));
            meta.insert(QStringLiteral("txid"), txid);
            meta.insert(QStringLiteral("address"), address);
            meta.insert(QStringLiteral("category"), category);
            meta.insert(QStringLiteral("confirmations"), tx.value(QStringLiteral("confirmations")).toInt());
            meta.insert(QStringLiteral("time"), time);
            meta.insert(QStringLiteral("amount"), tx.value(QStringLiteral("amount")).toDouble());
            meta.insert(QStringLiteral("label"), tx.value(QStringLiteral("label")).toString());
            rows.push_front(tableRow({
                QString(),
                QDateTime::fromSecsSinceEpoch(time).toString(QStringLiteral("MM/dd/yy HH:mm")),
                display_category,
                tx.value(QStringLiteral("label")).toString(address),
                amount
            }, meta));
        }
        m_recent_transactions = rows;
        Q_EMIT walletChanged();
    });
}

void NuRpcService::refreshAddressBook()
{
    const int generation = ++m_address_book_refresh_generation;
    rpcCall(QStringLiteral("listreceivedbyaddress"), {0, true, true}, true, [this, generation](const QJsonValue& result, const QString& error) {
        if (generation != m_address_book_refresh_generation) return;
        if (!error.isEmpty()) return;
        QVariantList receive_rows;
        for (const QJsonValue& value : result.toArray()) {
            const QJsonObject item = value.toObject();
            upsertAddressRow(receive_rows, row({
                item.value(QStringLiteral("label")).toString(QStringLiteral("(no label)")),
                item.value(QStringLiteral("address")).toString(),
                normalPurpose(QStringLiteral("receive")),
                QString::number(item.value(QStringLiteral("amount")).toDouble(), 'f', 8) + QStringLiteral(" DFC")
            }));
        }

        rpcCall(QStringLiteral("listlabels"), {QStringLiteral("send")}, true, [this, receive_rows, generation](const QJsonValue& labels, const QString& label_error) {
            if (generation != m_address_book_refresh_generation) return;
            auto rows = std::make_shared<QVariantList>(receive_rows);
            auto commit = [this, rows, generation]() {
                if (generation != m_address_book_refresh_generation) return;
                if (m_address_book == *rows) return;
                m_address_book = *rows;
                Q_EMIT walletChanged();
            };

            if (!label_error.isEmpty() || !labels.isArray()) {
                commit();
                return;
            }

            const QJsonArray label_array = labels.toArray();
            if (label_array.isEmpty()) {
                commit();
                return;
            }

            auto pending = std::make_shared<int>(label_array.size());
            for (const QJsonValue& label_value : label_array) {
                const QString label = label_value.toString();
                rpcCall(QStringLiteral("getaddressesbylabel"), {label}, true, [this, label, rows, pending, commit, generation](const QJsonValue& addresses, const QString& address_error) {
                    if (generation != m_address_book_refresh_generation) return;
                    if (address_error.isEmpty() && addresses.isObject()) {
                        const QJsonObject address_object = addresses.toObject();
                        for (auto it = address_object.constBegin(); it != address_object.constEnd(); ++it) {
                            const QJsonObject meta = it.value().toObject();
                            upsertAddressRow(*rows, row({
                                label.isEmpty() ? QStringLiteral("(no label)") : label,
                                it.key(),
                                normalPurpose(meta.value(QStringLiteral("purpose")).toString()),
                                QStringLiteral("")
                            }));
                        }
                    }

                    --(*pending);
                    if (*pending == 0) commit();
                });
            }
        });
    });
}

QString NuRpcService::receiveRequestSettingsKey() const
{
    const QString data_dir = QDir(m_data_dir.isEmpty() ? defaultDataDir() : m_data_dir).absolutePath();
    const QString wallet_name = m_wallet_name.trimmed().isEmpty() ? QStringLiteral("__default__") : m_wallet_name.trimmed();
    const QString hash = QString::fromLatin1(QCryptographicHash::hash(QStringLiteral("%1|%2").arg(data_dir, wallet_name).toUtf8(), QCryptographicHash::Sha256).toHex().left(32));
    return QStringLiteral("NuReceiveRequests/%1/json").arg(hash);
}

QVariantMap NuRpcService::receiveRequestMeta(const QString& address,
                                             const QString& label,
                                             const QString& amount,
                                             const QString& message,
                                             const QString& uri,
                                             const QString& date,
                                             qint64 created_ms) const
{
    const qint64 timestamp = created_ms > 0 ? created_ms : QDateTime::currentMSecsSinceEpoch();
    const QString display_date = date.trimmed().isEmpty()
        ? QDateTime::fromMSecsSinceEpoch(timestamp).toString(QStringLiteral("MM/dd/yy HH:mm"))
        : date.trimmed();
    const QString clean_address = address.trimmed();
    const QString clean_label = label.trimmed();
    const QString clean_amount = amount.trimmed();
    const QString clean_message = message.trimmed();
    const QString clean_uri = uri.trimmed().isEmpty()
        ? defcoinUri(clean_address, clean_amount, clean_label, clean_message)
        : uri.trimmed();

    QVariantMap meta;
    meta.insert(QStringLiteral("kind"), QStringLiteral("receiveRequest"));
    meta.insert(QStringLiteral("address"), clean_address);
    meta.insert(QStringLiteral("label"), clean_label);
    meta.insert(QStringLiteral("amount"), clean_amount);
    meta.insert(QStringLiteral("message"), clean_message);
    meta.insert(QStringLiteral("uri"), clean_uri);
    meta.insert(QStringLiteral("date"), display_date);
    meta.insert(QStringLiteral("createdMs"), timestamp);
    meta.insert(QStringLiteral("qrSource"), qrSourceForUri(clean_uri));
    return meta;
}

QVariantMap NuRpcService::receiveRequestRow(const QVariantMap& meta) const
{
    const QString label = meta.value(QStringLiteral("label")).toString().trimmed();
    return tableRow({
        QString(),
        QString(),
        meta.value(QStringLiteral("date")).toString(),
        label.isEmpty() ? QStringLiteral("Request") : label,
        meta.value(QStringLiteral("address")).toString(),
        meta.value(QStringLiteral("amount")).toString()
    }, meta);
}

void NuRpcService::loadReceiveRequests()
{
    const QString key = receiveRequestSettingsKey();
    if (m_loaded_receive_request_settings_key == key) return;
    m_loaded_receive_request_settings_key = key;

    QSettings settings;
    const QString raw = settings.value(key).toString();
    QVariantList rows;
    if (!raw.trimmed().isEmpty()) {
        const QJsonDocument doc = QJsonDocument::fromJson(raw.toUtf8());
        if (doc.isArray()) {
            for (const QJsonValue& value : doc.array()) {
                const QJsonObject item = value.toObject();
                const QString address = item.value(QStringLiteral("address")).toString().trimmed();
                if (address.isEmpty()) continue;
                const QVariantMap meta = receiveRequestMeta(address,
                                                            item.value(QStringLiteral("label")).toString(),
                                                            item.value(QStringLiteral("amount")).toString(),
                                                            item.value(QStringLiteral("message")).toString(),
                                                            item.value(QStringLiteral("uri")).toString(),
                                                            item.value(QStringLiteral("date")).toString(),
                                                            item.value(QStringLiteral("createdMs")).toVariant().toLongLong());
                rows.push_back(receiveRequestRow(meta));
            }
        }
    }

    if (m_receive_requests == rows) return;
    m_receive_requests = rows;
    Q_EMIT walletChanged();
}

void NuRpcService::saveReceiveRequests() const
{
    QJsonArray records;
    for (const QVariant& value : m_receive_requests) {
        const QVariantMap meta = value.toMap().value(QStringLiteral("meta")).toMap();
        const QString address = meta.value(QStringLiteral("address")).toString().trimmed();
        if (address.isEmpty()) continue;
        QJsonObject item;
        item.insert(QStringLiteral("address"), address);
        item.insert(QStringLiteral("label"), meta.value(QStringLiteral("label")).toString());
        item.insert(QStringLiteral("amount"), meta.value(QStringLiteral("amount")).toString());
        item.insert(QStringLiteral("message"), meta.value(QStringLiteral("message")).toString());
        item.insert(QStringLiteral("uri"), meta.value(QStringLiteral("uri")).toString());
        item.insert(QStringLiteral("date"), meta.value(QStringLiteral("date")).toString());
        item.insert(QStringLiteral("createdMs"), QJsonValue::fromVariant(meta.value(QStringLiteral("createdMs"))));
        records.push_back(item);
    }

    QSettings settings;
    settings.setValue(receiveRequestSettingsKey(), QString::fromUtf8(QJsonDocument(records).toJson(QJsonDocument::Compact)));
    settings.sync();
}

void NuRpcService::sampleTraffic()
{
    rpcCall(QStringLiteral("getnettotals"), {}, false, [this](const QJsonValue& result, const QString& error) {
        if (!error.isEmpty()) return;
        const QJsonObject totals = result.toObject();
        const qint64 now_ms = m_uptime.elapsed();
        const qint64 recv = totals.value(QStringLiteral("totalbytesrecv")).toVariant().toLongLong();
        const qint64 sent = totals.value(QStringLiteral("totalbytessent")).toVariant().toLongLong();
        m_traffic_received_total = formatBytes(recv);
        m_traffic_sent_total = formatBytes(sent);

        double recv_rate = 0.0;
        double sent_rate = 0.0;
        if (m_last_traffic_ms > 0 && now_ms > m_last_traffic_ms) {
            const double seconds = (now_ms - m_last_traffic_ms) / 1000.0;
            recv_rate = qMax(0.0, (recv - m_last_bytes_recv) / seconds);
            sent_rate = qMax(0.0, (sent - m_last_bytes_sent) / seconds);
        }

        m_last_traffic_ms = now_ms;
        m_last_bytes_recv = recv;
        m_last_bytes_sent = sent;

        QVariantMap point;
        point.insert(QStringLiteral("seconds"), now_ms / 1000.0);
        point.insert(QStringLiteral("timestampMs"), QDateTime::currentMSecsSinceEpoch());
        point.insert(QStringLiteral("received"), recv_rate);
        point.insert(QStringLiteral("sent"), sent_rate);
        const bool zero_sample = recv_rate == 0.0 && sent_rate == 0.0;
        if (zero_sample && !m_traffic_samples.isEmpty()) {
            const QVariantMap last_point = m_traffic_samples.constLast().toMap();
            const bool last_zero = last_point.value(QStringLiteral("received")).toDouble() == 0.0
                && last_point.value(QStringLiteral("sent")).toDouble() == 0.0;
            if (last_zero) {
                m_traffic_samples.last() = point;
            } else {
                m_traffic_samples.push_back(point);
            }
        } else {
            m_traffic_samples.push_back(point);
        }
        while (m_traffic_samples.size() > 900) {
            m_traffic_samples.removeFirst();
        }
        Q_EMIT trafficChanged();
    });
}

void NuRpcService::refreshFeeEstimate()
{
    rpcCall(QStringLiteral("estimatesmartfee"), {6, QStringLiteral("CONSERVATIVE")}, false, [this](const QJsonValue& result, const QString& error) {
        QString status;
        bool available = false;
        if (!error.isEmpty()) {
            status = QStringLiteral("Fee estimate unavailable: %1").arg(error);
        } else {
            const QJsonObject estimate = result.toObject();
            if (estimate.contains(QStringLiteral("feerate")) && estimate.value(QStringLiteral("feerate")).isDouble()) {
                available = true;
                status = QStringLiteral("Reference estimate, 6-block conservative: %1 DFC/kB").arg(QString::number(estimate.value(QStringLiteral("feerate")).toDouble(), 'f', 8));
            } else {
                QStringList reasons;
                for (const QJsonValue& reason : estimate.value(QStringLiteral("errors")).toArray()) {
                    reasons.push_back(reason.toString());
                }
                status = reasons.isEmpty()
                    ? QStringLiteral("Fee estimate unavailable; the backend will use fallback or minimum relay policy.")
                    : QStringLiteral("Fee estimate unavailable: %1").arg(reasons.join(QStringLiteral("; ")));
            }
        }
        if (m_fee_estimate_available != available || m_fee_estimate_status != status) {
            m_fee_estimate_available = available;
            m_fee_estimate_status = status;
            Q_EMIT feeEstimateChanged();
        }
    });
}

void NuRpcService::refreshDebugLog()
{
    const QString path = debugLogPath();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    const qint64 size = file.size();
    constexpr qint64 INITIAL_READ_LIMIT = 2 * 1024 * 1024;
    constexpr qint64 INCREMENTAL_READ_LIMIT = 512 * 1024;
    const bool reset_reader = (m_debug_log_path != path || m_debug_log_offset < 0 || m_debug_log_offset > size);
    QByteArray data;
    if (reset_reader) {
        const qint64 start = qMax<qint64>(0, size - INITIAL_READ_LIMIT);
        file.seek(start);
        data = file.read(size - start);
        if (start > 0) {
            const int first_newline = data.indexOf('\n');
            if (first_newline >= 0) data.remove(0, first_newline + 1);
        }
        m_debug_log_path = path;
        m_debug_log_collecting_continuation = false;
        if (m_log_lines.size() == 1 && m_log_lines.first().startsWith(QStringLiteral("No debug.log lines"))) {
            m_log_lines.clear();
        }
    } else {
        if (size == m_debug_log_offset) return;
        qint64 start = m_debug_log_offset;
        if (size - start > INCREMENTAL_READ_LIMIT) {
            start = size - INCREMENTAL_READ_LIMIT;
            m_debug_log_collecting_continuation = false;
            if (m_log_lines.isEmpty() || !m_log_lines.last().startsWith(QStringLiteral("... log gap"))) {
                m_log_lines.push_back(QStringLiteral("... log gap omitted from in-app view. Use Open debug.log for the backend log file."));
            }
        }
        file.seek(start);
        data = file.read(size - start);
        if (start != m_debug_log_offset) {
            const int first_newline = data.indexOf('\n');
            if (first_newline >= 0) data.remove(0, first_newline + 1);
        }
    }
    m_debug_log_offset = size;
    if (data.isEmpty()) return;

    const QList<QByteArray> lines = data.split('\n');
    const QDateTime launch_floor = m_app_launch_utc.addSecs(-2);
    QStringList new_lines;
    for (int i = 0; i < lines.size(); ++i) {
        const QString line = QString::fromUtf8(lines.at(i)).trimmed();
        if (line.isEmpty()) continue;

        QDateTime utc;
        QString message;
        if (parseDebugLogTimestamp(line, utc, message)) {
            if (message.startsWith(QStringLiteral("Nu startup:"))
                || message.startsWith(QStringLiteral("----- Nu startup diagnostics"))
                || message.startsWith(QStringLiteral("----- Backend debug.log"))) {
                continue;
            }
            m_debug_log_collecting_continuation = utc >= launch_floor;
            if (m_debug_log_collecting_continuation) new_lines.push_back(localLogTimestamp(utc) + QStringLiteral(" ") + message);
            continue;
        }

        if (m_debug_log_collecting_continuation) new_lines.push_back(line);
    }
    if (new_lines.isEmpty() && m_log_lines.isEmpty()) {
        new_lines.push_back(QStringLiteral("No debug.log lines have been written since this Nu launch yet (%1).")
                                .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss t"))));
    }
    if (new_lines.isEmpty()) return;
    if (m_log_lines.size() == 1 && m_log_lines.first().startsWith(QStringLiteral("No debug.log lines"))) {
        m_log_lines.clear();
    }
    beginBackendDebugLogSection(false);
    m_log_lines.append(new_lines);
    if (m_log_lines.size() > 5000) {
        m_log_lines = m_log_lines.mid(m_log_lines.size() - 5000);
        m_log_lines.push_front(QStringLiteral("... earlier current-launch log lines omitted from the in-app view. Use Open debug.log for the backend log file."));
    }
    Q_EMIT logChanged();
}

void NuRpcService::requestNewAddress(const QString& label, const QString& amount, const QString& message)
{
    const QString clean_amount = amount.trimmed();
    if (!clean_amount.isEmpty()) {
        bool amount_ok = false;
        const double amount_value = clean_amount.toDouble(&amount_ok);
        if (!amount_ok || amount_value <= 0.0) {
            Q_EMIT userMessage(QStringLiteral("Payment request not created"),
                               QStringLiteral("Enter a positive amount in DFC, or leave the amount blank."));
            return;
        }
    }

    QJsonArray params;
    params.push_back(label);
    rpcCall(QStringLiteral("getnewaddress"), params, true, [this, label, amount, message](const QJsonValue& result, const QString& error) {
        if (!error.isEmpty()) {
            Q_EMIT userMessage(QStringLiteral("Address request failed"), error);
            return;
        }
        m_receive_address = result.toString();
        m_receive_label = label.trimmed();
        m_receive_amount = amount.trimmed();
        m_receive_message = message.trimmed();
        const QString uri = defcoinUri(m_receive_address, m_receive_amount, m_receive_label, m_receive_message);
        const QVariantMap meta = receiveRequestMeta(m_receive_address,
                                                    m_receive_label,
                                                    m_receive_amount,
                                                    m_receive_message,
                                                    uri,
                                                    QString(),
                                                    QDateTime::currentMSecsSinceEpoch());
        m_receive_requests.push_front(receiveRequestRow(meta));
        saveReceiveRequests();
        updateReceiveQr();
        refreshAddressBook();
        Q_EMIT walletChanged();
    });
}

void NuRpcService::deleteReceiveRequest(const QString& address)
{
    QVariantList addresses;
    addresses.push_back(address);
    deleteReceiveRequests(addresses);
}

void NuRpcService::deleteReceiveRequests(const QVariantList& addresses)
{
    QSet<QString> targets;
    for (const QVariant& value : addresses) {
        const QString address = value.toString().trimmed();
        if (!address.isEmpty()) targets.insert(address);
    }
    if (targets.isEmpty()) return;

    QVariantList kept;
    int removed = 0;
    for (const QVariant& value : m_receive_requests) {
        const QVariantMap meta = value.toMap().value(QStringLiteral("meta")).toMap();
        const QString address = meta.value(QStringLiteral("address")).toString().trimmed();
        if (!address.isEmpty() && targets.contains(address)) {
            ++removed;
            continue;
        }
        kept.push_back(value);
    }
    if (removed == 0) return;

    m_receive_requests = kept;
    saveReceiveRequests();
    Q_EMIT walletChanged();
}

void NuRpcService::sendCoins(const QString& address,
                             const QString& amount,
                             const QString& fee_mode,
                             const QString& fee_target,
                             const QString& custom_fee_rate,
                             bool subtract_fee_from_amount,
                             const QString& label,
                             const QString& custom_change_address)
{
    QString recipient = address.trimmed();
    QString effective_amount = amount.trimmed();
    QString effective_label = label.trimmed();
    if (recipient.startsWith(QStringLiteral("defcoin:"), Qt::CaseInsensitive)) {
        QUrl uri(recipient);
        recipient = uri.path().isEmpty() ? uri.host() : uri.path();
        QUrlQuery query(uri);
        if (effective_amount.isEmpty() && query.hasQueryItem(QStringLiteral("amount"))) {
            effective_amount = query.queryItemValue(QStringLiteral("amount"));
        }
        if (effective_label.isEmpty() && query.hasQueryItem(QStringLiteral("label"))) {
            effective_label = query.queryItemValue(QStringLiteral("label"));
        }
    }

    bool amount_ok = false;
    const double amount_value = effective_amount.toDouble(&amount_ok);
    if (recipient.isEmpty() || !amount_ok || amount_value <= 0.0) {
        Q_EMIT userMessage(QStringLiteral("Payment not sent"), QStringLiteral("Enter a valid Defcoin address and amount."));
        return;
    }

    QString mode = fee_mode.trimmed().toLower();
    if (mode.contains(QStringLiteral("custom"))) {
        mode = QStringLiteral("custom");
    } else if (mode.contains(QStringLiteral("econom"))) {
        mode = QStringLiteral("economical");
    } else {
        mode = QStringLiteral("conservative");
    }
    bool target_ok = false;
    const int target = fee_target.toInt(&target_ok);
    bool fee_ok = false;
    const double fee_rate = custom_fee_rate.toDouble(&fee_ok);
    if (mode == QLatin1String("custom") && (!fee_ok || fee_rate <= 0.0)) {
        Q_EMIT userMessage(QStringLiteral("Payment not sent"), QStringLiteral("Enter a positive custom fee rate in motes/vB."));
        return;
    }

    const QString change_address = custom_change_address.trimmed();
    if (!change_address.isEmpty()) {
        QJsonObject outputs;
        outputs.insert(recipient, amount_value);

        QJsonObject options;
        options.insert(QStringLiteral("add_inputs"), true);
        options.insert(QStringLiteral("add_to_wallet"), true);
        options.insert(QStringLiteral("change_address"), change_address);
        if (subtract_fee_from_amount) {
            QJsonArray subtract_outputs;
            subtract_outputs.push_back(0);
            options.insert(QStringLiteral("subtract_fee_from_outputs"), subtract_outputs);
        }
        QJsonArray params;
        params.push_back(outputs);
        params.push_back(mode == QLatin1String("custom") ? QJsonValue(QJsonValue::Null) : QJsonValue(target_ok && target > 0 ? target : 6));
        params.push_back(mode == QLatin1String("economical") ? QStringLiteral("economical") : (mode == QLatin1String("custom") ? QStringLiteral("unset") : QStringLiteral("conservative")));
        params.push_back(mode == QLatin1String("custom") ? QJsonValue(fee_rate) : QJsonValue(QJsonValue::Null));
        params.push_back(options);

        rpcCall(QStringLiteral("send"), params, true, [this, recipient, effective_label](const QJsonValue& result, const QString& error) {
            if (!error.isEmpty()) {
                Q_EMIT userMessage(QStringLiteral("Payment failed"), error);
                return;
            }
            if (!effective_label.isEmpty()) {
                setAddressLabel(recipient, effective_label);
            }
            const QString txid = result.toObject().value(QStringLiteral("txid")).toString();
            Q_EMIT userMessage(QStringLiteral("Payment sent"), QStringLiteral("Transaction ID: %1").arg(txid));
            refreshWallet();
        });
        return;
    }

    QJsonArray params;
    params.push_back(recipient);
    params.push_back(amount_value);
    params.push_back(effective_label);
    params.push_back(effective_label);
    params.push_back(subtract_fee_from_amount);
    params.push_back(QJsonValue(QJsonValue::Null));

    if (mode == QLatin1String("custom")) {
        params.push_back(QJsonValue(QJsonValue::Null));
        params.push_back(QStringLiteral("unset"));
    } else {
        params.push_back(target_ok && target > 0 ? QJsonValue(target) : QJsonValue(6));
        params.push_back(mode == QLatin1String("economical") ? QStringLiteral("economical") : QStringLiteral("conservative"));
    }
    params.push_back(QJsonValue(QJsonValue::Null));

    if (mode == QLatin1String("custom")) {
        params.push_back(fee_rate);
    } else {
        params.push_back(QJsonValue(QJsonValue::Null));
    }
    params.push_back(false);

    rpcCall(QStringLiteral("sendtoaddress"), params, true, [this, recipient, effective_label](const QJsonValue& result, const QString& error) {
        if (!error.isEmpty()) {
            Q_EMIT userMessage(QStringLiteral("Payment failed"), error);
            return;
        }
        if (!effective_label.isEmpty()) {
            setAddressLabel(recipient, effective_label);
        }
        Q_EMIT userMessage(QStringLiteral("Payment sent"), QStringLiteral("Transaction ID: %1").arg(result.toString()));
        refreshWallet();
    });
}

void NuRpcService::createPsbt(const QString& address,
                              const QString& amount,
                              const QString& fee_mode,
                              const QString& fee_target,
                              const QString& custom_fee_rate,
                              bool subtract_fee_from_amount,
                              const QString& label,
                              const QString& custom_change_address)
{
    QString recipient = address.trimmed();
    QString effective_amount = amount.trimmed();
    QString effective_label = label.trimmed();
    if (recipient.startsWith(QStringLiteral("defcoin:"), Qt::CaseInsensitive)) {
        QUrl uri(recipient);
        recipient = uri.path().isEmpty() ? uri.host() : uri.path();
        QUrlQuery query(uri);
        if (effective_amount.isEmpty() && query.hasQueryItem(QStringLiteral("amount"))) {
            effective_amount = query.queryItemValue(QStringLiteral("amount"));
        }
        if (effective_label.isEmpty() && query.hasQueryItem(QStringLiteral("label"))) {
            effective_label = query.queryItemValue(QStringLiteral("label"));
        }
    }

    bool amount_ok = false;
    const double amount_value = effective_amount.toDouble(&amount_ok);
    if (recipient.isEmpty() || !amount_ok || amount_value <= 0.0) {
        Q_EMIT userMessage(QStringLiteral("PSBT not created"), QStringLiteral("Enter a valid Defcoin address and amount."));
        return;
    }

    QString mode = fee_mode.trimmed().toLower();
    if (mode.contains(QStringLiteral("custom"))) {
        mode = QStringLiteral("custom");
    } else if (mode.contains(QStringLiteral("econom"))) {
        mode = QStringLiteral("economical");
    } else {
        mode = QStringLiteral("conservative");
    }
    bool target_ok = false;
    const int target = fee_target.toInt(&target_ok);
    bool fee_ok = false;
    const double fee_rate = custom_fee_rate.toDouble(&fee_ok);
    if (mode == QLatin1String("custom") && (!fee_ok || fee_rate <= 0.0)) {
        Q_EMIT userMessage(QStringLiteral("PSBT not created"), QStringLiteral("Enter a positive custom fee rate in motes/vB."));
        return;
    }

    QJsonObject output;
    output.insert(recipient, amount_value);
    QJsonArray outputs;
    outputs.push_back(output);

    QJsonObject options;
    options.insert(QStringLiteral("add_inputs"), true);
    if (mode == QLatin1String("custom")) {
        options.insert(QStringLiteral("fee_rate"), fee_rate);
    } else {
        options.insert(QStringLiteral("conf_target"), target_ok && target > 0 ? target : 6);
        options.insert(QStringLiteral("estimate_mode"), mode == QLatin1String("economical") ? QStringLiteral("economical") : QStringLiteral("conservative"));
    }
    const QString change_address = custom_change_address.trimmed();
    if (!change_address.isEmpty()) options.insert(QStringLiteral("changeAddress"), change_address);
    if (subtract_fee_from_amount) {
        QJsonArray subtract_outputs;
        subtract_outputs.push_back(0);
        options.insert(QStringLiteral("subtractFeeFromOutputs"), subtract_outputs);
    }

    QJsonArray params;
    params.push_back(QJsonArray());
    params.push_back(outputs);
    params.push_back(0);
    params.push_back(options);
    params.push_back(true);

    rpcCall(QStringLiteral("walletcreatefundedpsbt"), params, true, [this, recipient, effective_label](const QJsonValue& result, const QString& error) {
        if (!error.isEmpty()) {
            Q_EMIT userMessage(QStringLiteral("PSBT creation failed"), error);
            return;
        }
        const QJsonObject object = result.toObject();
        const QString psbt = object.value(QStringLiteral("psbt")).toString();
        if (psbt.isEmpty()) {
            Q_EMIT userMessage(QStringLiteral("PSBT creation failed"), QStringLiteral("The backend did not return PSBT data."));
            return;
        }
        if (!effective_label.isEmpty()) setAddressLabel(recipient, effective_label);
        m_current_psbt = psbt;
        m_current_psbt_final_hex.clear();
        m_current_psbt_summary = QStringLiteral("Created PSBT from Send review.\n\n%1")
            .arg(QString::fromUtf8(QJsonDocument(object).toJson(QJsonDocument::Indented)).trimmed());
        Q_EMIT psbtChanged();
        analyzeCurrentPsbt(QStringLiteral("walletcreatefundedpsbt"));
        Q_EMIT userMessage(QStringLiteral("PSBT created"), QStringLiteral("Review, copy, save, sign, or finalize the PSBT in Send > Advanced send options."));
    });
}

void NuRpcService::setAddressLabel(const QString& address, const QString& label)
{
    if (address.trimmed().isEmpty()) return;
    rpcCall(QStringLiteral("setlabel"), {address.trimmed(), label.trimmed()}, true, [this](const QJsonValue&, const QString& error) {
        if (!error.isEmpty()) {
            Q_EMIT userMessage(QStringLiteral("Label update failed"), error);
            return;
        }
        refreshAddressBook();
    });
}

void NuRpcService::setNetworkActive(bool active)
{
    m_have_pending_network_active = true;
    m_pending_network_active = active;
    rpcCall(QStringLiteral("setnetworkactive"), {active}, false, [this, active](const QJsonValue&, const QString& error) {
        m_applying_pending_network_active = false;
        if (!error.isEmpty()) {
            if (m_connection_status == QLatin1String("Starting backend") ||
                error.startsWith(QStringLiteral("Starting Defcoin backend"))) {
                Q_EMIT userMessage(QStringLiteral("Starting backend"),
                                   QStringLiteral("Defcoin Core Nu is starting the backend and will apply the network setting when RPC is ready."));
                QTimer::singleShot(2500, this, &NuRpcService::refresh);
                return;
            }
            Q_EMIT userMessage(QStringLiteral("Network update failed"), error);
            return;
        }
        m_have_pending_network_active = false;
        m_network_state = active ? QStringLiteral("connected") : QStringLiteral("isolated");
        refreshNode();
    });
}

void NuRpcService::pingPeers()
{
    rpcCall(QStringLiteral("ping"), {}, false, [this](const QJsonValue&, const QString& error) {
        if (!error.isEmpty()) Q_EMIT userMessage(QStringLiteral("Ping failed"), error);
    });
}

void NuRpcService::runRpcCommand(const QString& method, const QString& params_json, bool wallet_scoped)
{
    const QString clean_method = method.trimmed();
    if (clean_method.isEmpty()) {
        m_console_output += QStringLiteral("\n\n> \nEnter an RPC method name.");
        Q_EMIT consoleChanged();
        return;
    }

    QJsonArray params;
    const QString clean_params = params_json.trimmed();
    if (!clean_params.isEmpty()) {
        QJsonParseError parse_error;
        const QJsonDocument doc = QJsonDocument::fromJson(clean_params.toUtf8(), &parse_error);
        if (parse_error.error != QJsonParseError::NoError || !doc.isArray()) {
            m_console_output += QStringLiteral("\n\n> %1 %2\nParameters must be a JSON array, for example: [\"address\", \"message\"]").arg(clean_method, clean_params);
            Q_EMIT consoleChanged();
            return;
        }
        params = doc.array();
    }

    const QString prompt = clean_params.isEmpty() ? clean_method : clean_method + QStringLiteral(" ") + clean_params;
    rpcCall(clean_method, params, wallet_scoped, [this, clean_method, prompt](const QJsonValue& result, const QString& error) {
        QString rendered;
        if (!error.isEmpty()) {
            rendered = QStringLiteral("%1 failed:\n%2").arg(clean_method, error);
        } else {
            QJsonDocument out_doc;
            if (result.isObject()) {
                out_doc = QJsonDocument(result.toObject());
            } else if (result.isArray()) {
                out_doc = QJsonDocument(result.toArray());
            }
            if (!out_doc.isNull()) {
                rendered = QString::fromUtf8(out_doc.toJson(QJsonDocument::Indented));
            } else if (result.isString()) {
                rendered = result.toString();
            } else if (result.isBool()) {
                rendered = result.toBool() ? QStringLiteral("true") : QStringLiteral("false");
            } else if (result.isDouble()) {
                rendered = QString::number(result.toDouble(), 'f', 8);
            } else if (result.isNull()) {
                rendered = QStringLiteral("null");
            } else {
                rendered = QStringLiteral("(empty result)");
            }
        }
        if (m_console_output.startsWith(QStringLiteral("Enter an RPC method"))) {
            m_console_output.clear();
        }
        m_console_output += QStringLiteral("%1> %2\n%3").arg(m_console_output.isEmpty() ? QString() : QStringLiteral("\n\n"), prompt, rendered.trimmed());
        constexpr int max_console_chars = 80000;
        if (m_console_output.size() > max_console_chars) {
            m_console_output = m_console_output.right(max_console_chars);
        }
        Q_EMIT consoleChanged();
    });
}

void NuRpcService::rebuildNodeMetrics()
{
    m_node_metrics = {
        row({"Network active", m_metric_network_active}),
        row({"Connections", m_metric_connections}),
        row({"Inbound", m_metric_inbound}),
        row({"Outbound", m_metric_outbound}),
        row({"Version", m_metric_version}),
        row({"Blocks", m_metric_blocks}),
        row({"Headers", m_metric_headers}),
        row({"Verification", m_metric_verification}),
        row({"Traffic", m_metric_traffic})
    };
}

void NuRpcService::setOnlyDefcoinUserAgents(bool enabled)
{
    if (m_only_defcoin_user_agents == enabled) return;
    m_only_defcoin_user_agents = enabled;
    QSettings(QStringLiteral("Defcoin"), QStringLiteral("Defcoin-Qt")).setValue(QStringLiteral("OnlyDefcoinUserAgents"), enabled);
    Q_EMIT settingsChanged();
    QTimer::singleShot(0, this, &NuRpcService::refreshNode);
    rpcCall(QStringLiteral("setonlydefcoinuseragents"), {enabled}, false, [this](const QJsonValue&, const QString&) {
        refreshNode();
    });
}

void NuRpcService::setOnlyDefcoinMagicBytes(bool enabled)
{
    if (m_only_defcoin_magic_bytes == enabled) return;
    m_only_defcoin_magic_bytes = enabled;
    QSettings settings;
    settings.setValue(QStringLiteral("OnlyDefcoinMagicBytes"), enabled);
    Q_EMIT settingsChanged();
    if (m_backend_started_by_nu || m_rpc_connected) {
        Q_EMIT userMessage(QStringLiteral("Restart required"),
                           QStringLiteral("The Defcoin magic setting is applied when the Defcoin backend starts. Restart Defcoin Core Nu to use the new setting."));
    }
}

void NuRpcService::setSwitchToDefcoinOnlyMagicStartingJuly2026(bool enabled)
{
    if (m_switch_to_defcoin_only_magic_starting_july_2026 == enabled) return;
    m_switch_to_defcoin_only_magic_starting_july_2026 = enabled;
    QSettings settings;
    settings.setValue(QStringLiteral("SwitchToDefcoinOnlyMagicStarting20260701"), enabled);
    if (enabled && QDate::currentDate() >= QDate(2026, 7, 1) && !m_only_defcoin_magic_bytes) {
        m_only_defcoin_magic_bytes = true;
        settings.setValue(QStringLiteral("OnlyDefcoinMagicBytes"), true);
    }
    Q_EMIT settingsChanged();
    if (m_backend_started_by_nu || m_rpc_connected) {
        Q_EMIT userMessage(QStringLiteral("Restart required"),
                           QStringLiteral("The scheduled Defcoin-only magic setting is applied when the Defcoin backend starts. Restart Defcoin Core Nu to use the new setting."));
    }
}

void NuRpcService::setDisallowLanNodeDiscovery(bool enabled)
{
    if (m_disallow_lan_node_discovery == enabled) return;
    m_disallow_lan_node_discovery = enabled;
    QSettings settings;
    settings.setValue(QStringLiteral("DisallowLanNodeDiscovery"), enabled);
    Q_EMIT settingsChanged();
    if (m_backend_started_by_nu || m_rpc_connected) {
        Q_EMIT userMessage(QStringLiteral("Restart required"),
                           QStringLiteral("The LAN node discovery setting is applied when the Defcoin backend starts. Restart Defcoin Core Nu to use the new setting."));
    }
}

void NuRpcService::setMaskBalances(bool enabled)
{
    if (m_mask_balances == enabled) return;
    m_mask_balances = enabled;
    QSettings().setValue(QStringLiteral("MaskBalances"), enabled);
    Q_EMIT settingsChanged();
    Q_EMIT walletChanged();
}

void NuRpcService::setThirdPartyTxUrlsEnabled(bool enabled)
{
    if (m_third_party_tx_urls_enabled == enabled) return;
    m_third_party_tx_urls_enabled = enabled;
    QSettings settings;
    settings.setValue(QStringLiteral("ThirdPartyTxUrlsEnabled"), enabled);
    QSettings(QStringLiteral("Defcoin"), QStringLiteral("Defcoin-Qt")).setValue(QStringLiteral("ThirdPartyTxUrlsEnabled"), enabled);
    Q_EMIT settingsChanged();
}

void NuRpcService::setThirdPartyTxUrl(const QString& url)
{
    const QString normalized = normalizedExplorerUrl(url);
    if (m_third_party_tx_url == normalized) return;
    m_third_party_tx_url = normalized;
    QSettings settings;
    settings.setValue(QStringLiteral("ThirdPartyTxUrl"), normalized);
    QSettings(QStringLiteral("Defcoin"), QStringLiteral("Defcoin-Qt")).setValue(QStringLiteral("strThirdPartyTxUrls"), normalized);
    Q_EMIT settingsChanged();
}

QString NuRpcService::normalizedExplorerUrl(const QString& url) const
{
    QString clean = url.trimmed();
    if (clean.isEmpty()) return QString();
    if (!clean.startsWith(QStringLiteral("http://"), Qt::CaseInsensitive) &&
        !clean.startsWith(QStringLiteral("https://"), Qt::CaseInsensitive)) {
        clean.prepend(QStringLiteral("https://"));
    }
    return clean;
}

QString NuRpcService::explorerPresetUrl(int index) const
{
    if (index == 1) return QStringLiteral("https://defcoin.dc903.org/explorer/tx/%s");
    if (index == 2) return QStringLiteral("https://defcoin.dc903.org/legacyexplorer/tx/%s");
    return QString();
}

QString NuRpcService::explorerAddressUrlTemplate(const QString& url) const
{
    QString out = url;
    out.replace(QStringLiteral("/tx/%s"), QStringLiteral("/address/%s"), Qt::CaseInsensitive);
    out.replace(QStringLiteral("/transaction/%s"), QStringLiteral("/address/%s"), Qt::CaseInsensitive);
    return out;
}

QString NuRpcService::explorerUrlForTransaction(const QString& txid) const
{
    if (!m_third_party_tx_urls_enabled || txid.trimmed().isEmpty() || !m_third_party_tx_url.contains(QStringLiteral("%s"))) {
        return QString();
    }
    QString out = m_third_party_tx_url;
    out.replace(QStringLiteral("%s"), QString::fromLatin1(QUrl::toPercentEncoding(txid.trimmed())));
    return out;
}

QString NuRpcService::explorerUrlForAddress(const QString& address) const
{
    if (!m_third_party_tx_urls_enabled || address.trimmed().isEmpty() || !m_third_party_tx_url.contains(QStringLiteral("%s"))) {
        return QString();
    }
    QString out = explorerAddressUrlTemplate(m_third_party_tx_url);
    out.replace(QStringLiteral("%s"), QString::fromLatin1(QUrl::toPercentEncoding(address.trimmed())));
    return out;
}

void NuRpcService::copyText(const QString& text)
{
    QApplication::clipboard()->setText(text);
}

void NuRpcService::backupWallet()
{
    const QString path = QFileDialog::getSaveFileName(nullptr, QStringLiteral("Backup Wallet"), QDir(realHomePath()).filePath(QStringLiteral("wallet.dat")));
    if (path.isEmpty()) return;
    rpcCall(QStringLiteral("backupwallet"), {path}, true, [this](const QJsonValue&, const QString& error) {
        Q_EMIT userMessage(error.isEmpty() ? QStringLiteral("Wallet backup complete") : QStringLiteral("Wallet backup failed"),
                           error.isEmpty() ? QStringLiteral("The wallet backup was written successfully.") : error);
    });
}

void NuRpcService::encryptWallet(const QString& passphrase)
{
    if (m_wallet_encrypted) {
        Q_EMIT userMessage(QStringLiteral("Wallet already encrypted"),
                           QStringLiteral("This wallet already has a passphrase. Use Change passphrase to update it."));
        return;
    }
    if (passphrase.length() < 8) {
        Q_EMIT userMessage(QStringLiteral("Wallet not encrypted"), QStringLiteral("Enter a passphrase of at least 8 characters."));
        return;
    }
    rpcCall(QStringLiteral("encryptwallet"), {passphrase}, true, [this](const QJsonValue& result, const QString& error) {
        Q_EMIT userMessage(error.isEmpty() ? QStringLiteral("Wallet encrypted") : QStringLiteral("Wallet encryption failed"),
                           error.isEmpty() ? result.toString(QStringLiteral("Wallet encrypted. Close and reopen the backend before spending.")) : error);
        refreshWallet();
    });
}

void NuRpcService::changeWalletPassphrase(const QString& old_passphrase, const QString& new_passphrase)
{
    if (!m_wallet_encrypted) {
        Q_EMIT userMessage(QStringLiteral("Passphrase not changed"),
                           QStringLiteral("This wallet is not encrypted yet. Use Encrypt wallet to create the first wallet passphrase."));
        return;
    }
    if (old_passphrase.isEmpty()) {
        Q_EMIT userMessage(QStringLiteral("Passphrase not changed"), QStringLiteral("Enter the current wallet passphrase."));
        return;
    }
    if (new_passphrase.length() < 8) {
        Q_EMIT userMessage(QStringLiteral("Passphrase not changed"), QStringLiteral("Enter a new passphrase of at least 8 characters."));
        return;
    }
    rpcCall(QStringLiteral("walletpassphrasechange"), {old_passphrase, new_passphrase}, true, [this](const QJsonValue&, const QString& error) {
        QString message = error;
        if (message.contains(QStringLiteral("unencrypted wallet"), Qt::CaseInsensitive)) {
            message = QStringLiteral("This wallet is not encrypted yet. Use Encrypt wallet to create the first wallet passphrase.");
        }
        Q_EMIT userMessage(error.isEmpty() ? QStringLiteral("Passphrase changed") : QStringLiteral("Passphrase change failed"),
                           error.isEmpty() ? QStringLiteral("The wallet passphrase was changed.") : message);
    });
}

void NuRpcService::signMessage(const QString& address, const QString& message)
{
    if (address.trimmed().isEmpty() || message.isEmpty()) {
        Q_EMIT userMessage(QStringLiteral("Message not signed"), QStringLiteral("Enter an address and message."));
        return;
    }
    rpcCall(QStringLiteral("signmessage"), {address.trimmed(), message}, true, [this](const QJsonValue& result, const QString& error) {
        if (!error.isEmpty()) {
            Q_EMIT userMessage(QStringLiteral("Message signing failed"), error);
            return;
        }
        const QString signature = result.toString();
        copyText(signature);
        Q_EMIT userMessage(QStringLiteral("Message signed"), QStringLiteral("Signature copied to clipboard:\n%1").arg(signature));
    });
}

void NuRpcService::verifyMessage(const QString& address, const QString& signature, const QString& message)
{
    if (address.trimmed().isEmpty() || signature.trimmed().isEmpty() || message.isEmpty()) {
        Q_EMIT userMessage(QStringLiteral("Message not verified"), QStringLiteral("Enter an address, signature, and message."));
        return;
    }
    rpcCall(QStringLiteral("verifymessage"), {address.trimmed(), signature.trimmed(), message}, false, [this](const QJsonValue& result, const QString& error) {
        if (!error.isEmpty()) {
            Q_EMIT userMessage(QStringLiteral("Message verification failed"), error);
            return;
        }
        Q_EMIT userMessage(QStringLiteral("Message verification"), result.toBool() ? QStringLiteral("Signature is valid.") : QStringLiteral("Signature is not valid."));
    });
}

void NuRpcService::openDebugLog()
{
    const QString path = debugLogPath();
    if (!QFileInfo::exists(path)) {
        Q_EMIT userMessage(QStringLiteral("Debug log not found"),
                           QStringLiteral("No debug.log file was found at:\n%1").arg(path));
        return;
    }
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(path))) {
        Q_EMIT userMessage(QStringLiteral("Debug log not opened"),
                           QStringLiteral("The operating system did not open:\n%1").arg(path));
    }
}

void NuRpcService::requestTransactionDetails(const QString& txid)
{
    const QString clean_txid = txid.trimmed();
    if (clean_txid.isEmpty()) {
        Q_EMIT userMessage(QStringLiteral("Transaction details unavailable"), QStringLiteral("This row does not include a transaction ID."));
        return;
    }

    rpcCall(QStringLiteral("gettransaction"), {clean_txid, true}, true, [this, clean_txid](const QJsonValue& result, const QString& error) {
        if (!error.isEmpty()) {
            Q_EMIT userMessage(QStringLiteral("Transaction details unavailable"), error);
            return;
        }
        const QJsonObject tx = result.toObject();
        const QJsonArray details = tx.value(QStringLiteral("details")).toArray();
        QString address;
        QString label;
        QString category;
        if (!details.isEmpty()) {
            const QJsonObject first = details.first().toObject();
            address = first.value(QStringLiteral("address")).toString();
            label = first.value(QStringLiteral("label")).toString();
            category = first.value(QStringLiteral("category")).toString();
        }

        auto localTime = [](const QJsonValue& value) {
            const qint64 seconds = value.toVariant().toLongLong();
            return seconds > 0 ? QDateTime::fromSecsSinceEpoch(seconds).toLocalTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss t")) : QStringLiteral("N/A");
        };
        auto rowHtml = [](const QString& key, const QString& value) {
            return QStringLiteral("<tr><th>%1</th><td>%2</td></tr>")
                .arg(key.toHtmlEscaped(), value.toHtmlEscaped());
        };

        QString html = QStringLiteral("<h2>Transaction details</h2><table>");
        html += rowHtml(QStringLiteral("Status"), tx.value(QStringLiteral("confirmations")).toInt() > 0 ? QStringLiteral("Confirmed") : QStringLiteral("Unconfirmed"));
        html += rowHtml(QStringLiteral("Confirmations"), QString::number(tx.value(QStringLiteral("confirmations")).toInt()));
        html += rowHtml(QStringLiteral("Date"), localTime(tx.value(QStringLiteral("time"))));
        html += rowHtml(QStringLiteral("Received by node"), localTime(tx.value(QStringLiteral("timereceived"))));
        if (!category.isEmpty()) html += rowHtml(QStringLiteral("Type"), category.left(1).toUpper() + category.mid(1));
        if (!label.isEmpty()) html += rowHtml(QStringLiteral("Label"), label);
        if (!address.isEmpty()) html += rowHtml(QStringLiteral("Address"), address);
        html += rowHtml(QStringLiteral("Amount"), QString::number(tx.value(QStringLiteral("amount")).toDouble(), 'f', 8) + QStringLiteral(" DFC"));
        if (tx.contains(QStringLiteral("fee"))) {
            html += rowHtml(QStringLiteral("Fee"), QString::number(tx.value(QStringLiteral("fee")).toDouble(), 'f', 8) + QStringLiteral(" DFC"));
        }
        html += rowHtml(QStringLiteral("Transaction ID"), clean_txid);
        if (tx.contains(QStringLiteral("blockhash"))) html += rowHtml(QStringLiteral("Block hash"), tx.value(QStringLiteral("blockhash")).toString());
        html += QStringLiteral("</table>");

        const QString tx_url = explorerUrlForTransaction(clean_txid);
        const QString address_url = explorerUrlForAddress(address);
        if (!tx_url.isEmpty() || !address_url.isEmpty()) {
            const QString host = QUrl(m_third_party_tx_url, QUrl::StrictMode).host();
            html += QStringLiteral("<h3>Explorer links</h3><p>Use %1 block explorer to open:</p><ul>").arg(host.toHtmlEscaped());
            if (!tx_url.isEmpty()) {
                html += QStringLiteral("<li>Transaction ID: <a href=\"%1\" title=\"Open %1\">%2</a></li>")
                    .arg(tx_url.toHtmlEscaped(), clean_txid.toHtmlEscaped());
            }
            if (!address_url.isEmpty()) {
                html += QStringLiteral("<li>Wallet Address: <a href=\"%1\" title=\"Open %1\">%2</a></li>")
                    .arg(address_url.toHtmlEscaped(), address.toHtmlEscaped());
            }
            html += QStringLiteral("</ul>");
        }

        html += QStringLiteral("<h3>Backend JSON</h3><pre>%1</pre>")
            .arg(QString::fromUtf8(QJsonDocument(tx).toJson(QJsonDocument::Indented)).toHtmlEscaped());
        Q_EMIT transactionDetailsReady(QStringLiteral("Transaction details"), html);
    });
}

QString NuRpcService::debugLogPath() const
{
    const QDir data_dir(m_data_dir.isEmpty() ? defaultDataDir() : m_data_dir);
    return data_dir.filePath(QStringLiteral("debug.log"));
}

void NuRpcService::stopOwnedBackend()
{
    if (!m_backend_started_by_nu || !qEnvironmentVariableIsEmpty("DEFCOIN_NU_KEEP_BACKEND_RUNNING")) {
        return;
    }

    bool stop_requested = false;
    if (m_data_dir.isEmpty()) m_data_dir = defaultDataDir();
    readDefcoinConf(QDir(m_data_dir).filePath(QStringLiteral("defcoin.conf")));
    if ((!m_rpc_user.isEmpty() && !m_rpc_password.isEmpty()) || readCookie()) {
        QNetworkAccessManager manager;
        QNetworkRequest request(rpcUrl(false));
        request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
        const QByteArray auth = QStringLiteral("%1:%2").arg(m_rpc_user, m_rpc_password).toUtf8().toBase64();
        request.setRawHeader("Authorization", "Basic " + auth);

        QJsonObject request_obj;
        request_obj.insert(QStringLiteral("jsonrpc"), QStringLiteral("1.0"));
        request_obj.insert(QStringLiteral("id"), QStringLiteral("nu-shutdown"));
        request_obj.insert(QStringLiteral("method"), QStringLiteral("stop"));
        request_obj.insert(QStringLiteral("params"), QJsonArray());

        QEventLoop loop;
        QTimer timeout;
        timeout.setSingleShot(true);
        QNetworkReply* reply = manager.post(request, QJsonDocument(request_obj).toJson(QJsonDocument::Compact));
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
        timeout.start(2500);
        loop.exec();
        stop_requested = reply->isFinished() && reply->error() == QNetworkReply::NoError;
        reply->deleteLater();
    }

    if (m_backend_process && m_backend_process->state() != QProcess::NotRunning) {
        if (!stop_requested) m_backend_process->terminate();
        if (!m_backend_process->waitForFinished(5000)) {
            m_backend_process->kill();
            m_backend_process->waitForFinished(3000);
        }
    }

    m_backend_started_by_nu = false;
    m_backend_pid = 0;
}

QString NuRpcService::helpManualPath(const QString& page) const
{
    QString clean_page = QFileInfo(page.trimmed().isEmpty() ? QStringLiteral("index.html") : page.trimmed()).fileName();
    if (clean_page.isEmpty()) clean_page = QStringLiteral("index.html");

#if defined(Q_OS_MACOS)
    return QDir(QCoreApplication::applicationDirPath()).filePath(
        QStringLiteral("../Resources/DefcoinCoreNu.help/Contents/Resources/en.lproj/%1").arg(clean_page));
#else
    return QDir(QCoreApplication::applicationDirPath()).filePath(
        QStringLiteral("nu/help/DefcoinCoreNu.help/Contents/Resources/en.lproj/%1").arg(clean_page));
#endif
}

QString NuRpcService::helpManualHtml(const QString& page) const
{
    const QString path = helpManualPath(page);
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QStringLiteral("<h1>Help not found</h1><p>The Defcoin Core Nu help file was not found at:<br><code>%1</code></p>")
            .arg(path.toHtmlEscaped());
    }
    QString html = QString::fromUtf8(file.readAll());
#if defined(Q_OS_WIN)
    // Qt Quick TextEdit supports a useful but limited HTML subset and may ignore
    // some stylesheet paragraph margins. Keep the shipped HTML standards-based,
    // then add conservative explicit spacing for the in-app Windows reader.
    html.replace(QRegularExpression(QStringLiteral("(<h2\\b)")),
                 QStringLiteral("<br><br>\\1"));
    html.replace(QRegularExpression(QStringLiteral("(<h3\\b)")),
                 QStringLiteral("<br>\\1"));
    html.replace(QRegularExpression(QStringLiteral("</p>")),
                 QStringLiteral("</p><br>"));
    html.replace(QRegularExpression(QStringLiteral("</ul>")),
                 QStringLiteral("</ul><br>"));
    html.replace(QRegularExpression(QStringLiteral("</ol>")),
                 QStringLiteral("</ol><br>"));
#endif
    return html;
}

void NuRpcService::openHelpManual(const QString& page)
{
    const QString path = helpManualPath(page);
    if (!QFileInfo::exists(path)) {
        Q_EMIT userMessage(QStringLiteral("Help not found"),
                           QStringLiteral("The Defcoin Core Nu help file was not found at:\n%1").arg(path));
        return;
    }

#if defined(Q_OS_MACOS)
    const QString help_book = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../Resources/DefcoinCoreNu.help"));
    if (QFileInfo::exists(help_book) && OpenDefcoinNuHelpBook(page)) {
        return;
    }
#elif defined(Q_OS_WIN)
    Q_EMIT userMessage(QStringLiteral("Open Help"),
                       QStringLiteral("Use the in-app Help window from the Help menu. The Windows build ships help files with the app instead of using CHM."));
    return;
#endif

    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(path))) {
        Q_EMIT userMessage(QStringLiteral("Help not opened"),
                           QStringLiteral("The operating system did not open:\n%1").arg(path));
    }
}

void NuRpcService::exportTransactionsCsv()
{
    const QString path = QFileDialog::getSaveFileName(nullptr, QStringLiteral("Export Transactions"), QDir(realHomePath()).filePath(QStringLiteral("defcoin-transactions.csv")), QStringLiteral("CSV files (*.csv)"));
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Q_EMIT userMessage(QStringLiteral("Export failed"), file.errorString());
        return;
    }
    QTextStream out(&file);
    out << "Date,Type,Label,Amount\n";
    for (const QVariant& row_value : m_recent_transactions) {
        QVariantList fields = tableRowCells(row_value);
        if (!fields.isEmpty() && fields.first().toString().isEmpty()) fields.removeFirst();
        QStringList escaped;
        for (const QVariant& field : fields) {
            QString text = field.toString();
            text.replace("\"", "\"\"");
            escaped.push_back("\"" + text + "\"");
        }
        out << escaped.join(',') << '\n';
    }
    Q_EMIT userMessage(QStringLiteral("Export complete"), QStringLiteral("The transaction CSV was written successfully."));
}

void NuRpcService::exportTrafficCsv()
{
    const QString path = QFileDialog::getSaveFileName(nullptr, QStringLiteral("Export Network Traffic"), QDir(realHomePath()).filePath(QStringLiteral("defcoin-network-traffic.csv")), QStringLiteral("CSV files (*.csv)"));
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Q_EMIT userMessage(QStringLiteral("Export failed"), file.errorString());
        return;
    }
    QTextStream out(&file);
    out << "Seconds,Local Time,Received bytes per second,Sent bytes per second\n";
    for (const QVariant& point_value : m_traffic_samples) {
        const QVariantMap point = point_value.toMap();
        out << point.value(QStringLiteral("seconds")).toDouble() << ','
            << '"' << QDateTime::fromMSecsSinceEpoch(point.value(QStringLiteral("timestampMs")).toLongLong()).toLocalTime().toString(Qt::ISODate) << '"' << ','
            << point.value(QStringLiteral("received")).toDouble() << ','
            << point.value(QStringLiteral("sent")).toDouble() << '\n';
    }
    Q_EMIT userMessage(QStringLiteral("Export complete"), QStringLiteral("The network traffic CSV was written successfully."));
}

void NuRpcService::loadPsbtPayload(const QByteArray& payload, const QString& source)
{
    QByteArray trimmed = payload.trimmed();
    if (trimmed.isEmpty()) {
        Q_EMIT userMessage(QStringLiteral("PSBT not loaded"), QStringLiteral("No PSBT data was found in %1.").arg(source));
        return;
    }

    const QString text = QString::fromLatin1(trimmed);
    const bool text_looks_base64 = QRegularExpression(QStringLiteral(R"(^[A-Za-z0-9+/=\r\n\t ]+$)")).match(text).hasMatch();
    QString psbt = text_looks_base64 ? text : QString::fromLatin1(trimmed.toBase64());
    psbt.remove(QRegularExpression(QStringLiteral(R"(\s+)")));
    if (psbt.isEmpty()) {
        Q_EMIT userMessage(QStringLiteral("PSBT not loaded"), QStringLiteral("The PSBT data from %1 could not be normalized.").arg(source));
        return;
    }

    m_current_psbt = psbt;
    m_current_psbt_final_hex.clear();
    m_current_psbt_summary = QStringLiteral("Loaded PSBT from %1. Analyzing...").arg(source);
    Q_EMIT psbtChanged();
    analyzeCurrentPsbt(source);
}

void NuRpcService::analyzeCurrentPsbt(const QString& source)
{
    if (m_current_psbt.isEmpty()) return;
    rpcCall(QStringLiteral("analyzepsbt"), {m_current_psbt}, false, [this, source](const QJsonValue& result, const QString& error) {
        if (!error.isEmpty()) {
            m_current_psbt_summary = QStringLiteral("Loaded PSBT from %1, but analyzepsbt failed:\n%2").arg(source, error);
            Q_EMIT psbtChanged();
            Q_EMIT userMessage(QStringLiteral("PSBT loaded with warning"), m_current_psbt_summary);
            return;
        }
        const QString json = QString::fromUtf8(QJsonDocument(result.toObject()).toJson(QJsonDocument::Indented)).trimmed();
        m_current_psbt_summary = QStringLiteral("Loaded PSBT from %1.\n\n%2").arg(source, json);
        Q_EMIT psbtChanged();
        Q_EMIT userMessage(QStringLiteral("PSBT loaded"), QStringLiteral("Review the PSBT summary in Send > Advanced send options."));
    });
}

void NuRpcService::loadPsbtFromFile()
{
    const QString path = QFileDialog::getOpenFileName(nullptr,
                                                      QStringLiteral("Load PSBT"),
                                                      realHomePath(),
                                                      QStringLiteral("PSBT files (*.psbt *.txt);;All files (*)"));
    if (path.isEmpty()) return;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        Q_EMIT userMessage(QStringLiteral("PSBT not loaded"), file.errorString());
        return;
    }
    loadPsbtPayload(file.readAll(), QFileInfo(path).fileName());
}

void NuRpcService::loadPsbtFromClipboard()
{
    const QString text = QApplication::clipboard()->text().trimmed();
    if (text.isEmpty()) {
        Q_EMIT userMessage(QStringLiteral("PSBT not loaded"), QStringLiteral("The clipboard does not contain text PSBT data."));
        return;
    }
    loadPsbtPayload(text.toLatin1(), QStringLiteral("clipboard"));
}

void NuRpcService::signCurrentPsbt()
{
    if (m_current_psbt.isEmpty()) {
        Q_EMIT userMessage(QStringLiteral("No PSBT loaded"), QStringLiteral("Load a PSBT before signing."));
        return;
    }
    rpcCall(QStringLiteral("walletprocesspsbt"), {m_current_psbt, true}, true, [this](const QJsonValue& result, const QString& error) {
        if (!error.isEmpty()) {
            Q_EMIT userMessage(QStringLiteral("PSBT signing failed"), error);
            return;
        }
        const QJsonObject object = result.toObject();
        const QString processed = object.value(QStringLiteral("psbt")).toString();
        if (!processed.isEmpty()) m_current_psbt = processed;
        m_current_psbt_final_hex.clear();
        const bool complete = object.value(QStringLiteral("complete")).toBool();
        m_current_psbt_summary = QStringLiteral("Wallet processed PSBT. Complete: %1\n\n%2")
            .arg(complete ? QStringLiteral("yes") : QStringLiteral("no"),
                 QString::fromUtf8(QJsonDocument(object).toJson(QJsonDocument::Indented)).trimmed());
        Q_EMIT psbtChanged();
        analyzeCurrentPsbt(QStringLiteral("walletprocesspsbt"));
    });
}

void NuRpcService::finalizeCurrentPsbt()
{
    if (m_current_psbt.isEmpty()) {
        Q_EMIT userMessage(QStringLiteral("No PSBT loaded"), QStringLiteral("Load a PSBT before finalizing."));
        return;
    }
    rpcCall(QStringLiteral("finalizepsbt"), {m_current_psbt}, false, [this](const QJsonValue& result, const QString& error) {
        if (!error.isEmpty()) {
            Q_EMIT userMessage(QStringLiteral("PSBT finalization failed"), error);
            return;
        }
        const QJsonObject object = result.toObject();
        const bool complete = object.value(QStringLiteral("complete")).toBool();
        m_current_psbt_final_hex = object.value(QStringLiteral("hex")).toString();
        const QString updated_psbt = object.value(QStringLiteral("psbt")).toString();
        if (!updated_psbt.isEmpty()) m_current_psbt = updated_psbt;
        m_current_psbt_summary = QStringLiteral("Finalize PSBT result. Complete: %1\n\n%2")
            .arg(complete ? QStringLiteral("yes") : QStringLiteral("no"),
                 QString::fromUtf8(QJsonDocument(object).toJson(QJsonDocument::Indented)).trimmed());
        Q_EMIT psbtChanged();
        Q_EMIT userMessage(complete ? QStringLiteral("PSBT finalized") : QStringLiteral("PSBT incomplete"),
                           complete ? QStringLiteral("The finalized transaction is ready to broadcast.") : QStringLiteral("The PSBT still needs more signatures or data."));
    });
}

void NuRpcService::broadcastFinalizedPsbt()
{
    if (m_current_psbt_final_hex.isEmpty()) {
        Q_EMIT userMessage(QStringLiteral("PSBT not finalized"), QStringLiteral("Finalize the PSBT before broadcasting."));
        return;
    }
    rpcCall(QStringLiteral("sendrawtransaction"), {m_current_psbt_final_hex}, false, [this](const QJsonValue& result, const QString& error) {
        if (!error.isEmpty()) {
            Q_EMIT userMessage(QStringLiteral("Broadcast failed"), error);
            return;
        }
        Q_EMIT userMessage(QStringLiteral("Transaction broadcast"), QStringLiteral("Transaction ID: %1").arg(result.toString()));
        clearCurrentPsbt();
        refreshWallet();
    });
}

void NuRpcService::copyCurrentPsbt()
{
    if (m_current_psbt.isEmpty()) {
        Q_EMIT userMessage(QStringLiteral("No PSBT loaded"), QStringLiteral("Load a PSBT before copying."));
        return;
    }
    copyText(m_current_psbt);
    Q_EMIT userMessage(QStringLiteral("PSBT copied"), QStringLiteral("The current PSBT was copied to the clipboard."));
}

void NuRpcService::saveCurrentPsbt()
{
    if (m_current_psbt.isEmpty()) {
        Q_EMIT userMessage(QStringLiteral("No PSBT loaded"), QStringLiteral("Load a PSBT before saving."));
        return;
    }
    const QString path = QFileDialog::getSaveFileName(nullptr,
                                                      QStringLiteral("Save PSBT"),
                                                      QDir(realHomePath()).filePath(QStringLiteral("defcoin.psbt")),
                                                      QStringLiteral("PSBT files (*.psbt);;Text files (*.txt);;All files (*)"));
    if (path.isEmpty()) return;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Q_EMIT userMessage(QStringLiteral("PSBT not saved"), file.errorString());
        return;
    }
    file.write(m_current_psbt.toLatin1());
    file.write("\n");
    Q_EMIT userMessage(QStringLiteral("PSBT saved"), QStringLiteral("The current PSBT was saved."));
}

void NuRpcService::clearCurrentPsbt()
{
    m_current_psbt.clear();
    m_current_psbt_final_hex.clear();
    m_current_psbt_summary = QStringLiteral("No PSBT loaded.");
    Q_EMIT psbtChanged();
}

QVariantList NuRpcService::tableColumnWidths(const QString& table_id, const QVariantList& default_widths) const
{
    if (table_id.trimmed().isEmpty()) return default_widths;
    const QStringList parts = QSettings().value(QStringLiteral("NuTables/%1/columnWidths").arg(table_id)).toStringList();
    if (parts.size() != default_widths.size()) return default_widths;
    QVariantList out;
    for (const QString& part : parts) {
        bool ok = false;
        const double value = part.toDouble(&ok);
        if (!ok || value < 24.0) return default_widths;
        out.push_back(value);
    }
    return out;
}

QVariantList NuRpcService::suggestedTableColumnWidths(const QVariantList& columns,
                                                       const QVariantList& rows,
                                                       const QVariantList& column_types,
                                                       const QVariantList& column_weights,
                                                       const QVariantList& column_minimums,
                                                       const QVariantList& column_maximums,
                                                       int available_width,
                                                       int font_pixel_size,
                                                       bool compact) const
{
    QVariantList out;
    if (columns.isEmpty()) return out;

    QFont font = QApplication::font();
    QFont mono_font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    if (font_pixel_size > 0) {
        font.setPixelSize(font_pixel_size);
        mono_font.setPixelSize(font_pixel_size);
    }
    QFont header_font = font;
    header_font.setBold(true);
    QFont mono_header_font = mono_font;
    mono_header_font.setBold(true);
    QFontMetrics metrics(font);
    QFontMetrics mono_metrics(mono_font);
    QFontMetrics header_metrics(header_font);
    QFontMetrics mono_header_metrics(mono_header_font);

    Q_UNUSED(column_weights);
    Q_UNUSED(available_width);
    const int padding = compact ? 18 : 28;

    QVector<double> widths;
    widths.reserve(columns.size());
    for (int c = 0; c < columns.size(); ++c) {
        const QString type = variantColumnType(column_types, columns, c);
        const double minimum = qMax(36.0, variantAt(column_minimums, c, defaultColumnMinimum(type)));
        const double maximum = qMax(minimum, variantAt(column_maximums, c, defaultColumnMaximum(type)));
        const QString title = columns.at(c).toString();
        const bool mono = monoColumnType(type, title);
        const QFontMetrics& cell_metrics = mono ? mono_metrics : metrics;
        const QFontMetrics& title_metrics = mono ? mono_header_metrics : header_metrics;
        double wanted = minimum;
        wanted = qMax(wanted, double(title_metrics.horizontalAdvance(title) + padding + 8));
        for (const QVariant& row_value : rows) {
            const QVariantList row = tableRowCells(row_value);
            if (row.size() <= c) continue;
            wanted = qMax(wanted, double(cell_metrics.horizontalAdvance(row.at(c).toString()) + padding + 8));
        }
        widths.push_back(qBound(minimum, std::ceil(wanted), maximum));
    }

    for (double width : widths) out.push_back(std::ceil(width));
    return out;
}

void NuRpcService::saveTableColumnWidths(const QString& table_id, const QVariantList& widths)
{
    if (table_id.trimmed().isEmpty() || widths.isEmpty()) return;
    QStringList parts;
    for (const QVariant& value : widths) parts.push_back(QString::number(value.toDouble(), 'f', 2));
    QSettings().setValue(QStringLiteral("NuTables/%1/columnWidths").arg(table_id), parts);
}

void NuRpcService::resetTableColumnWidths(const QString& table_id)
{
    QSettings settings;
    if (table_id.trimmed().isEmpty()) {
        settings.beginGroup(QStringLiteral("NuTables"));
        settings.remove(QString());
        settings.endGroup();
    } else {
        settings.remove(QStringLiteral("NuTables/%1").arg(table_id));
    }
    settings.sync();
    Q_EMIT tableSettingsChanged();
    Q_EMIT userMessage(QStringLiteral("Table columns reset"),
                       table_id.trimmed().isEmpty()
                           ? QStringLiteral("Saved column widths were cleared for all Nu tables. Table sorting was also restored to first-launch defaults.")
                           : QStringLiteral("Saved column widths were cleared for this table. Table sorting was also restored to first-launch defaults."));
}

void NuRpcService::updateReceiveQr()
{
    if (m_receive_address.isEmpty()) return;
    const QString uri = defcoinUri(m_receive_address, m_receive_amount, m_receive_label, m_receive_message);
    m_receive_qr_source = qrSourceForUri(uri);
}

QString NuRpcService::defcoinUri(const QString& address, const QString& amount, const QString& label, const QString& message) const
{
    QUrlQuery query;
    if (!amount.trimmed().isEmpty()) query.addQueryItem(QStringLiteral("amount"), amount.trimmed());
    if (!label.trimmed().isEmpty()) query.addQueryItem(QStringLiteral("label"), label.trimmed());
    if (!message.trimmed().isEmpty()) query.addQueryItem(QStringLiteral("message"), message.trimmed());
    QString uri = QStringLiteral("defcoin:%1").arg(address.trimmed());
    const QString query_string = query.toString(QUrl::FullyEncoded);
    if (!query_string.isEmpty()) uri += QStringLiteral("?") + query_string;
    return uri;
}

QString NuRpcService::qrSourceForUri(const QString& uri) const
{
    if (uri.trimmed().isEmpty()) return QString();
    QRcode* code = QRcode_encodeString(uri.toUtf8().constData(), 0, QR_ECLEVEL_L, QR_MODE_8, 1);
    if (!code) return QString();

    QImage qr(code->width + 8, code->width + 8, QImage::Format_RGB32);
    qr.fill(Qt::white);
    unsigned char* p = code->data;
    for (int y = 0; y < code->width; ++y) {
        for (int x = 0; x < code->width; ++x) {
            qr.setPixel(x + 4, y + 4, ((*p & 1) ? 0x000000 : 0xffffff));
            ++p;
        }
    }
    QRcode_free(code);

    QImage out(QR_IMAGE_SIZE, QR_IMAGE_SIZE, QImage::Format_RGB32);
    out.fill(Qt::white);
    {
        QPainter painter(&out);
        painter.drawImage(out.rect().adjusted(24, 24, -24, -24), qr);
    }
    const QString hash = QString::fromLatin1(QCryptographicHash::hash(uri.toUtf8(), QCryptographicHash::Sha1).toHex().left(16));
    const QString qr_path = QDir::temp().filePath(QStringLiteral("defcoin-core-nu-qr-%1.png").arg(hash));
    out.save(qr_path);
    return QUrl::fromLocalFile(qr_path).toString();
}

QString NuRpcService::receiveRequestQrSource(const QString& uri) const
{
    return qrSourceForUri(uri);
}

QString NuRpcService::formatAmount(const QJsonValue& value)
{
    return QString::number(value.toDouble(), 'f', 8);
}

QString NuRpcService::formatBytes(qint64 bytes)
{
    static const QStringList units{QStringLiteral("B"), QStringLiteral("KB"), QStringLiteral("MB"), QStringLiteral("GB")};
    double value = bytes;
    int unit = 0;
    while (value >= 1024.0 && unit + 1 < units.size()) {
        value /= 1024.0;
        ++unit;
    }
    return QStringLiteral("%1 %2").arg(value, 0, unit == 0 ? 'f' : 'f', unit == 0 ? 0 : 1).arg(units.at(unit));
}

QString NuRpcService::formatPing(const QJsonValue& seconds)
{
    if (!seconds.isDouble()) return QStringLiteral("N/A");
    return QStringLiteral("%1 ms").arg(qRound(seconds.toDouble() * 1000.0));
}

QString NuRpcService::formatServices(const QString& services_hex)
{
    bool ok = false;
    const qulonglong services = services_hex.trimmed().toULongLong(&ok, 16);
    if (!ok || services == 0) return QStringLiteral("-");

    QStringList codes;
    if (services & (1ULL << 0)) codes << QStringLiteral("N");
    if (services & (1ULL << 1)) codes << QStringLiteral("G");
    if (services & (1ULL << 2)) codes << QStringLiteral("B");
    if (services & (1ULL << 3)) codes << QStringLiteral("W");
    if (services & (1ULL << 6)) codes << QStringLiteral("CF");
    if (services & (1ULL << 10)) codes << QStringLiteral("NL");
    if (services & (1ULL << 24)) codes << QStringLiteral("M");
    if (services & (1ULL << 25)) codes << QStringLiteral("MLC");
    return codes.isEmpty() ? services_hex : codes.join(QLatin1Char(' '));
}

QString NuRpcService::trimUserAgent(QString subver)
{
    subver = subver.trimmed();
    while (subver.startsWith(QLatin1Char('/'))) subver.remove(0, 1);
    while (subver.endsWith(QLatin1Char('/'))) subver.chop(1);
    return subver;
}

bool NuRpcService::isDefcoinUserAgent(QString subver)
{
    subver = trimUserAgent(subver);
    return subver.startsWith(QStringLiteral("Defcoin"), Qt::CaseInsensitive);
}
