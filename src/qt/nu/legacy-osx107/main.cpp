#include <QtWidgets>

#ifndef DEFCOIN_NU_VERSION
#define DEFCOIN_NU_VERSION "26.3.1-lion-nu"
#endif

namespace {
QString appResourceRoot()
{
    QDir dir(QCoreApplication::applicationDirPath());
    if (dir.cdUp() && dir.cd("Resources") && dir.cd("nu")) {
        return dir.absolutePath();
    }
    QDir source(QCoreApplication::applicationDirPath());
    source.cdUp();
    return source.absoluteFilePath("nu");
}

QString bundledPluginRootFromArgv(char** argv)
{
    if (!argv || !argv[0]) return QString();
    QFileInfo exe(QString::fromLocal8Bit(argv[0]));
    QDir dir(exe.absoluteDir());
    if (dir.cdUp() && dir.cd("PlugIns")) {
        return dir.canonicalPath();
    }
    return QString();
}

void configureBundledQtPlugins(char** argv)
{
    const QString pluginRoot = bundledPluginRootFromArgv(argv);
    if (!pluginRoot.isEmpty()) {
        qputenv("QT_PLUGIN_PATH", QFile::encodeName(pluginRoot));
    }
}

void configureBundledCertificates()
{
    const QString bundledCerts = QDir(appResourceRoot()).filePath("ssl/cert.pem");
    if (QFileInfo(bundledCerts).isReadable()) {
        qputenv("SSL_CERT_FILE", QFile::encodeName(bundledCerts));
    }
}

QString defaultDataDir()
{
    return QDir(QDir::homePath()).filePath("Library/Application Support/Defcoin");
}

QString debugLogPath()
{
    return QDir(defaultDataDir()).filePath("debug.log");
}

QString backendPath()
{
    const QString bundled = QDir(appResourceRoot()).filePath("bin/defcoind");
    if (QFileInfo(bundled).isExecutable()) return bundled;
    return QString();
}

QString assetPath(const QString& relative)
{
    return QDir(appResourceRoot()).filePath(QString("assets/%1").arg(relative));
}

QString tailFile(const QString& path, int maxLines)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString("No readable log at %1").arg(path);
    }
    QList<QByteArray> lines;
    while (!file.atEnd()) {
        lines.append(file.readLine());
        while (lines.size() > maxLines) lines.removeFirst();
    }
    QString out;
    foreach (const QByteArray& line, lines) out += QString::fromLocal8Bit(line);
    return out.trimmed();
}

QString findLastLogLine(const QString& needle)
{
    const QStringList lines = tailFile(debugLogPath(), 900).split('\n');
    for (int i = lines.size() - 1; i >= 0; --i) {
        if (lines.at(i).contains(needle, Qt::CaseInsensitive)) return lines.at(i).trimmed();
    }
    return QString();
}

QString extractPeerCount()
{
    const QStringList lines = tailFile(debugLogPath(), 900).split('\n');
    int count = 0;
    foreach (const QString& line, lines) {
        if (line.contains("New outbound peer connected", Qt::CaseInsensitive)) ++count;
    }
    if (count == 0) return "Watching";
    return QString::number(count);
}

QLabel* makeLabel(const QString& text, const QString& className, QWidget* parent)
{
    QLabel* label = new QLabel(text, parent);
    label->setProperty("class", className);
    label->setWordWrap(true);
    return label;
}
}

class NavButton : public QPushButton
{
    Q_OBJECT

public:
    NavButton(const QString& iconName, const QString& text, QWidget* parent = 0) : QPushButton(parent)
    {
        setText(text);
        setCheckable(true);
        setMinimumHeight(44);
        setCursor(Qt::PointingHandCursor);
        const QString iconFile = assetPath(QString("icons/%1.svg").arg(iconName));
        if (QFileInfo(iconFile).isReadable()) {
            setIcon(QIcon(iconFile));
            setIconSize(QSize(18, 18));
        }
    }
};

class MetricCard : public QFrame
{
    Q_OBJECT

public:
    MetricCard(const QString& title, const QString& value, const QString& detail, QWidget* parent = 0) : QFrame(parent)
    {
        setObjectName("metricCard");
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setContentsMargins(16, 14, 16, 14);
        layout->setSpacing(8);
        m_title = makeLabel(title, "metricTitle", this);
        m_value = makeLabel(value, "metricValue", this);
        m_detail = makeLabel(detail, "metricDetail", this);
        layout->addWidget(m_title);
        layout->addWidget(m_value);
        layout->addWidget(m_detail);
        layout->addStretch();
    }

    void setValue(const QString& value) { m_value->setText(value); }
    void setDetail(const QString& detail) { m_detail->setText(detail); }

private:
    QLabel* m_title;
    QLabel* m_value;
    QLabel* m_detail;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow()
    {
        setWindowTitle("Defcoin Core Nu");
        resize(1180, 740);
        setMinimumSize(980, 640);

        QWidget* central = new QWidget(this);
        QHBoxLayout* shell = new QHBoxLayout(central);
        shell->setContentsMargins(0, 0, 0, 0);
        shell->setSpacing(0);

        QWidget* nav = new QWidget(central);
        nav->setObjectName("navRail");
        nav->setFixedWidth(216);
        QVBoxLayout* navLayout = new QVBoxLayout(nav);
        navLayout->setContentsMargins(18, 22, 18, 18);
        navLayout->setSpacing(10);

        QLabel* lockup = new QLabel(nav);
        lockup->setObjectName("brandLockup");
        lockup->setText("Defcoin\nCore Nu");
        QPixmap mark(assetPath("brand/defcoin-nu-icon-1024.png"));
        if (!mark.isNull()) {
            lockup->setPixmap(mark.scaled(86, 86, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            lockup->setMinimumHeight(100);
        }
        QLabel* product = makeLabel("Defcoin Core Nu", "navTitle", nav);
        QLabel* build = makeLabel(QString("Lion Legacy UI\n%1").arg(DEFCOIN_NU_VERSION), "navSubtle", nav);
        navLayout->addWidget(lockup);
        navLayout->addWidget(product);
        navLayout->addWidget(build);
        navLayout->addSpacing(14);

        m_homeButton = addNavButton(navLayout, "home", "Home", 0);
        m_walletButton = addNavButton(navLayout, "wallet", "Wallet", 1);
        m_sendButton = addNavButton(navLayout, "send", "Send", 2);
        m_receiveButton = addNavButton(navLayout, "receive", "Receive", 3);
        m_nodeButton = addNavButton(navLayout, "node", "Node", 4);
        m_activityButton = addNavButton(navLayout, "activity", "Activity", 5);
        navLayout->addStretch();

        m_statusPill = makeLabel("Backend idle", "statusPill", nav);
        navLayout->addWidget(m_statusPill);

        QWidget* content = new QWidget(central);
        content->setObjectName("content");
        QVBoxLayout* contentLayout = new QVBoxLayout(content);
        contentLayout->setContentsMargins(28, 24, 28, 22);
        contentLayout->setSpacing(18);

        QHBoxLayout* top = new QHBoxLayout();
        QVBoxLayout* heading = new QVBoxLayout();
        heading->setSpacing(3);
        m_pageTitle = makeLabel("Home", "pageTitle", content);
        m_pageSubtitle = makeLabel("Nu interface preview for Mac OS X 10.7.5. Backend logic stays in defcoind.", "pageSubtitle", content);
        heading->addWidget(m_pageTitle);
        heading->addWidget(m_pageSubtitle);
        top->addLayout(heading, 1);
        QPushButton* startButton = new QPushButton("Start Node", content);
        QPushButton* stopButton = new QPushButton("Stop", content);
        QPushButton* refreshButton = new QPushButton("Refresh", content);
        startButton->setObjectName("primaryButton");
        stopButton->setObjectName("secondaryButton");
        refreshButton->setObjectName("secondaryButton");
        top->addWidget(startButton);
        top->addWidget(stopButton);
        top->addWidget(refreshButton);
        contentLayout->addLayout(top);

        m_stack = new QStackedWidget(content);
        m_stack->addWidget(buildHomePage());
        m_stack->addWidget(buildWalletPage());
        m_stack->addWidget(buildActionPage("Send", "Prepare outgoing payments in the modern Nu workflow on newer systems. This Lion build keeps signing and wallet mutation inside the bundled backend."));
        m_stack->addWidget(buildActionPage("Receive", "Watch addresses and incoming activity from the backend. QR and mnemonic workflows remain part of the modern Nu app."));
        m_stack->addWidget(buildNodePage());
        m_stack->addWidget(buildActivityPage());
        contentLayout->addWidget(m_stack, 1);

        shell->addWidget(nav);
        shell->addWidget(content, 1);
        setCentralWidget(central);

        m_process = new QProcess(this);
        connect(startButton, SIGNAL(clicked()), this, SLOT(startBackend()));
        connect(stopButton, SIGNAL(clicked()), this, SLOT(stopBackend()));
        connect(refreshButton, SIGNAL(clicked()), this, SLOT(refreshAll()));
        connect(m_process, SIGNAL(started()), this, SLOT(onBackendStarted()));
        connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onBackendError(QProcess::ProcessError)));
        connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onBackendFinished(int,QProcess::ExitStatus)));

        m_timer = new QTimer(this);
        connect(m_timer, SIGNAL(timeout()), this, SLOT(refreshAll()));
        m_timer->start(5000);

        applyStyle();
        setRoute(0);
        refreshAll();
    }

private slots:
    void startBackend()
    {
        if (m_process->state() != QProcess::NotRunning) {
            m_statusPill->setText("Node running");
            return;
        }
        const QString binary = backendPath();
        if (binary.isEmpty()) {
            m_statusPill->setText("Backend missing");
            m_activityLog->setPlainText("No bundled defcoind found at Resources/nu/bin/defcoind.");
            return;
        }
        QDir().mkpath(defaultDataDir());
        QStringList args;
        args << "-server" << "-listen=1" << "-discover=1" << "-allowlannodediscovery=1";
        m_process->start(binary, args);
        m_statusPill->setText("Starting node");
    }

    void stopBackend()
    {
        if (m_process->state() == QProcess::NotRunning) {
            m_statusPill->setText("Node idle");
            return;
        }
        m_process->terminate();
        if (!m_process->waitForFinished(5000)) {
            m_process->kill();
        }
    }

    void refreshAll()
    {
        const QString peerCount = extractPeerCount();
        const QString syncLine = findLastLogLine("Synchronizing blockheaders");
        const QString tipLine = findLastLogLine("UpdateTip");
        const QString connectedLine = findLastLogLine("New outbound peer connected");
        m_peerCard->setValue(peerCount);
        m_peerCard->setDetail(connectedLine.isEmpty() ? "Waiting for outbound peers" : connectedLine);
        m_syncCard->setValue(syncLine.isEmpty() ? "Waiting" : "Syncing");
        m_syncCard->setDetail(syncLine.isEmpty() ? "No header sync line yet" : syncLine);
        m_chainCard->setValue(tipLine.isEmpty() ? "Genesis" : "Active");
        m_chainCard->setDetail(tipLine.isEmpty() ? "No downloaded block tip yet" : tipLine);
        m_walletCard->setValue(QFileInfo(QDir(defaultDataDir()).filePath("wallet.dat")).exists() ? "Present" : "No wallet loaded");
        m_walletCard->setDetail(QString("Data dir: %1").arg(defaultDataDir()));
        m_nodeText->setPlainText(nodeSummary());
        m_activityLog->setPlainText(tailFile(debugLogPath(), 260));
    }

    void onBackendStarted()
    {
        m_statusPill->setText("Node running");
        refreshAll();
    }

    void onBackendError(QProcess::ProcessError)
    {
        m_statusPill->setText("Launch error");
        m_activityLog->setPlainText(QString("Backend launch error: %1").arg(m_process->errorString()));
    }

    void onBackendFinished(int code, QProcess::ExitStatus status)
    {
        m_statusPill->setText(status == QProcess::NormalExit ? "Node stopped" : "Node crashed");
        m_activityLog->setPlainText(QString("Backend exited: code %1, status %2\n\n%3")
            .arg(code)
            .arg(status == QProcess::NormalExit ? "normal" : "crashed")
            .arg(tailFile(debugLogPath(), 220)));
    }

    void routeHome() { setRoute(0); }
    void routeWallet() { setRoute(1); }
    void routeSend() { setRoute(2); }
    void routeReceive() { setRoute(3); }
    void routeNode() { setRoute(4); }
    void routeActivity() { setRoute(5); }

private:
    NavButton* addNavButton(QVBoxLayout* layout, const QString& icon, const QString& text, int route)
    {
        NavButton* button = new NavButton(icon, text);
        layout->addWidget(button);
        switch (route) {
        case 0: connect(button, SIGNAL(clicked()), this, SLOT(routeHome())); break;
        case 1: connect(button, SIGNAL(clicked()), this, SLOT(routeWallet())); break;
        case 2: connect(button, SIGNAL(clicked()), this, SLOT(routeSend())); break;
        case 3: connect(button, SIGNAL(clicked()), this, SLOT(routeReceive())); break;
        case 4: connect(button, SIGNAL(clicked()), this, SLOT(routeNode())); break;
        case 5: connect(button, SIGNAL(clicked()), this, SLOT(routeActivity())); break;
        }
        m_navButtons.append(button);
        return button;
    }

    QWidget* buildHomePage()
    {
        QWidget* page = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(page);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(18);
        QHBoxLayout* cards = new QHBoxLayout();
        cards->setSpacing(16);
        m_peerCard = new MetricCard("Peers", "Watching", "Waiting for network activity", page);
        m_syncCard = new MetricCard("Sync", "Waiting", "No header sync line yet", page);
        m_chainCard = new MetricCard("Chain", "Genesis", "No downloaded block tip yet", page);
        cards->addWidget(m_peerCard);
        cards->addWidget(m_syncCard);
        cards->addWidget(m_chainCard);
        layout->addLayout(cards);
        layout->addWidget(makePanel("Legacy Nu Boundary",
            "This Lion build uses the Nu product shell, iconography, colors, and backend boundary, but keeps the modern Qt 6/QML implementation on supported macOS releases. Private keys, signing, chain validation, and peer logic remain inside defcoind.",
            page));
        layout->addStretch();
        return page;
    }

    QWidget* buildWalletPage()
    {
        QWidget* page = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(page);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(16);
        m_walletCard = new MetricCard("Wallet", "No wallet loaded", QString("Data dir: %1").arg(defaultDataDir()), page);
        layout->addWidget(m_walletCard);
        layout->addWidget(makePanel("Wallet Safety",
            "Use the full Qt 6 Nu app on modern macOS for the complete wallet creation and review flow. This Lion shell is for node startup, packaging validation, network visibility, and compatibility testing.",
            page));
        layout->addStretch();
        return page;
    }

    QWidget* buildActionPage(const QString& title, const QString& body)
    {
        QWidget* page = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(page);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(makePanel(title, body, page));
        layout->addStretch();
        return page;
    }

    QWidget* buildNodePage()
    {
        QWidget* page = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(page);
        layout->setContentsMargins(0, 0, 0, 0);
        m_nodeText = new QTextEdit(page);
        m_nodeText->setReadOnly(true);
        m_nodeText->setObjectName("logBox");
        layout->addWidget(m_nodeText);
        return page;
    }

    QWidget* buildActivityPage()
    {
        QWidget* page = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(page);
        layout->setContentsMargins(0, 0, 0, 0);
        m_activityLog = new QTextEdit(page);
        m_activityLog->setReadOnly(true);
        m_activityLog->setObjectName("logBox");
        layout->addWidget(m_activityLog);
        return page;
    }

    QWidget* makePanel(const QString& title, const QString& body, QWidget* parent)
    {
        QFrame* panel = new QFrame(parent);
        panel->setObjectName("panel");
        QVBoxLayout* layout = new QVBoxLayout(panel);
        layout->setContentsMargins(20, 18, 20, 18);
        layout->setSpacing(10);
        layout->addWidget(makeLabel(title, "panelTitle", panel));
        layout->addWidget(makeLabel(body, "panelBody", panel));
        return panel;
    }

    QString nodeSummary() const
    {
        QStringList lines;
        lines << "Defcoin Core Nu Lion shell";
        lines << "";
        lines << QString("Backend: %1").arg(backendPath().isEmpty() ? "not bundled" : backendPath());
        lines << QString("Data directory: %1").arg(defaultDataDir());
        lines << QString("Debug log: %1").arg(debugLogPath());
        lines << "";
        lines << "Launch defaults:";
        lines << "listen=1";
        lines << "discover=1";
        lines << "allowlannodediscovery=1";
        lines << "";
        lines << "Recent sync:";
        lines << (findLastLogLine("Synchronizing blockheaders").isEmpty() ? "No header sync line yet" : findLastLogLine("Synchronizing blockheaders"));
        lines << "";
        lines << "Recent peer:";
        lines << (findLastLogLine("New outbound peer connected").isEmpty() ? "No peer connection line yet" : findLastLogLine("New outbound peer connected"));
        return lines.join("\n");
    }

    void setRoute(int route)
    {
        m_stack->setCurrentIndex(route);
        for (int i = 0; i < m_navButtons.size(); ++i) {
            m_navButtons.at(i)->setChecked(i == route);
        }
        const QStringList titles = QStringList() << "Home" << "Wallet" << "Send" << "Receive" << "Node" << "Activity";
        const QStringList subtitles = QStringList()
            << "Nu interface preview for Mac OS X 10.7.5. Backend logic stays in defcoind."
            << "Wallet state is read from the backend data directory."
            << "Review-first send workflow placeholder for the Lion compatibility shell."
            << "Receive workflow placeholder for the Lion compatibility shell."
            << "Node startup, LAN discovery, and network state."
            << "Live backend log tail.";
        m_pageTitle->setText(titles.value(route));
        m_pageSubtitle->setText(subtitles.value(route));
    }

    void applyStyle()
    {
        setStyleSheet(
            "QWidget#navRail { background: #111820; color: #f2f6f9; }"
            "QWidget#content { background: #eef3f7; }"
            "QLabel[class='navTitle'] { color: #f6fbff; font-size: 19px; font-weight: 700; }"
            "QLabel[class='navSubtle'] { color: #9eb1be; font-size: 12px; }"
            "QLabel[class='statusPill'] { color: #7ce0c3; background: #17242d; border: 1px solid #244250; border-radius: 6px; padding: 8px; }"
            "QPushButton { border: 1px solid #c8d5df; border-radius: 5px; padding: 8px 12px; background: #f8fbfd; color: #1d2a33; }"
            "QPushButton:hover { background: #edf6fb; }"
            "QPushButton:checked { background: #1b8fbd; color: white; border-color: #1b8fbd; }"
            "QPushButton#primaryButton { background: #1b8fbd; color: white; border-color: #1b8fbd; font-weight: 700; }"
            "QPushButton#secondaryButton { background: #ffffff; color: #26343d; }"
            "QLabel[class='pageTitle'] { color: #15222c; font-size: 28px; font-weight: 700; }"
            "QLabel[class='pageSubtitle'] { color: #62737f; font-size: 13px; }"
            "QFrame#metricCard, QFrame#panel { background: #ffffff; border: 1px solid #d5e0e8; border-radius: 6px; }"
            "QLabel[class='metricTitle'] { color: #637481; font-size: 12px; text-transform: uppercase; }"
            "QLabel[class='metricValue'] { color: #15222c; font-size: 24px; font-weight: 700; }"
            "QLabel[class='metricDetail'], QLabel[class='panelBody'] { color: #4d5d68; font-size: 13px; }"
            "QLabel[class='panelTitle'] { color: #15222c; font-size: 18px; font-weight: 700; }"
            "QTextEdit#logBox { background: #101820; color: #d8e7ee; border: 1px solid #263b48; border-radius: 5px; font-family: Menlo, Monaco, monospace; font-size: 11px; }"
        );
    }

    QStackedWidget* m_stack = 0;
    QLabel* m_pageTitle = 0;
    QLabel* m_pageSubtitle = 0;
    QLabel* m_statusPill = 0;
    MetricCard* m_peerCard = 0;
    MetricCard* m_syncCard = 0;
    MetricCard* m_chainCard = 0;
    MetricCard* m_walletCard = 0;
    QTextEdit* m_nodeText = 0;
    QTextEdit* m_activityLog = 0;
    QProcess* m_process = 0;
    QTimer* m_timer = 0;
    QList<NavButton*> m_navButtons;
    NavButton* m_homeButton = 0;
    NavButton* m_walletButton = 0;
    NavButton* m_sendButton = 0;
    NavButton* m_receiveButton = 0;
    NavButton* m_nodeButton = 0;
    NavButton* m_activityButton = 0;
};

int main(int argc, char** argv)
{
    configureBundledQtPlugins(argv);
    QApplication app(argc, argv);
    QApplication::setApplicationName("Defcoin Core Nu");
    QApplication::setOrganizationName("Defcoin Core");
    QApplication::setOrganizationDomain("defcoincore.org");
    const QString pluginRoot = bundledPluginRootFromArgv(argv);
    if (!pluginRoot.isEmpty()) {
        QApplication::setLibraryPaths(QStringList() << pluginRoot);
    }
    configureBundledCertificates();

    MainWindow window;
    window.show();
    return app.exec();
}

#include "main.moc"
