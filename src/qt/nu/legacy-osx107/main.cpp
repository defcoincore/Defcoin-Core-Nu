#include <QtWidgets>
#include <QtNetwork>

#ifndef DEFCOIN_NU_VERSION
#define DEFCOIN_NU_VERSION "26.3.1-lion"
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

QString defaultDataDir()
{
    const QString home = QDir::homePath();
    return QDir(home).filePath("Library/Application Support/Defcoin");
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
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow()
    {
        setWindowTitle("Defcoin Core Nu Legacy");
        resize(920, 640);

        QWidget* central = new QWidget(this);
        QVBoxLayout* root = new QVBoxLayout(central);

        QLabel* title = new QLabel("Defcoin Core Nu Legacy", central);
        QFont titleFont = title->font();
        titleFont.setPointSize(22);
        titleFont.setBold(true);
        title->setFont(titleFont);
        root->addWidget(title);

        QLabel* subtitle = new QLabel(
            QString("Version %1 for Mac OS X 10.7.5 / Qt 5.5.1").arg(DEFCOIN_NU_VERSION),
            central);
        root->addWidget(subtitle);

        m_status = new QLabel("Backend not started by this launcher yet.", central);
        m_status->setWordWrap(true);
        root->addWidget(m_status);

        QHBoxLayout* actions = new QHBoxLayout();
        QPushButton* startButton = new QPushButton("Start bundled defcoind", central);
        QPushButton* stopButton = new QPushButton("Stop launched backend", central);
        QPushButton* refreshButton = new QPushButton("Refresh log", central);
        actions->addWidget(startButton);
        actions->addWidget(stopButton);
        actions->addWidget(refreshButton);
        actions->addStretch();
        root->addLayout(actions);

        m_tabs = new QTabWidget(central);
        m_overview = new QTextEdit(central);
        m_overview->setReadOnly(true);
        m_log = new QTextEdit(central);
        m_log->setReadOnly(true);
        m_tabs->addTab(m_overview, "Overview");
        m_tabs->addTab(m_log, "Debug Log");
        root->addWidget(m_tabs, 1);

        setCentralWidget(central);

        connect(startButton, SIGNAL(clicked()), this, SLOT(startBackend()));
        connect(stopButton, SIGNAL(clicked()), this, SLOT(stopBackend()));
        connect(refreshButton, SIGNAL(clicked()), this, SLOT(refreshLog()));

        m_process = new QProcess(this);
        connect(m_process, SIGNAL(started()), this, SLOT(onBackendStarted()));
        connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onBackendError(QProcess::ProcessError)));
        connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onBackendFinished(int,QProcess::ExitStatus)));

        refreshOverview();
        refreshLog();
    }

private slots:
    void startBackend()
    {
        if (m_process->state() != QProcess::NotRunning) {
            m_status->setText("Backend is already running from this launcher.");
            return;
        }
        const QString binary = backendPath();
        if (binary.isEmpty()) {
            m_status->setText("No bundled defcoind found at Resources/nu/bin/defcoind. Build or stage the backend before using this launcher.");
            return;
        }
        QStringList args;
        args << "-server";
        m_process->start(binary, args);
    }

    void stopBackend()
    {
        if (m_process->state() == QProcess::NotRunning) {
            m_status->setText("No backend process was launched by this window.");
            return;
        }
        m_process->terminate();
        if (!m_process->waitForFinished(5000)) {
            m_process->kill();
        }
    }

    void refreshLog()
    {
        m_log->setPlainText(tailFile(debugLogPath(), 240));
    }

    void onBackendStarted()
    {
        m_status->setText("Started bundled defcoind.");
    }

    void onBackendError(QProcess::ProcessError)
    {
        m_status->setText(QString("Backend launch error: %1").arg(m_process->errorString()));
    }

    void onBackendFinished(int code, QProcess::ExitStatus status)
    {
        m_status->setText(QString("Backend exited: code %1, status %2").arg(code).arg(status == QProcess::NormalExit ? "normal" : "crashed"));
        refreshLog();
    }

private:
    void refreshOverview()
    {
        QStringList lines;
        lines << "This branch is the Mac OS X 10.7.5 compatibility build path for Defcoin Core Nu.";
        lines << "";
        lines << "Why it exists:";
        lines << "- Qt 6.10 targets modern macOS and cannot run on this host.";
        lines << "- Qt 5.5.1 is the last suitable Qt line for OS X 10.7.";
        lines << "- The main Qt6/QML app remains on the normal release branch.";
        lines << "";
        lines << "Local paths:";
        lines << QString("- Resource root: %1").arg(appResourceRoot());
        lines << QString("- Data directory: %1").arg(defaultDataDir());
        lines << QString("- Debug log: %1").arg(debugLogPath());
        lines << QString("- Bundled backend: %1").arg(backendPath().isEmpty() ? "not found" : backendPath());
        m_overview->setPlainText(lines.join("\n"));
    }

    QLabel* m_status = 0;
    QTabWidget* m_tabs = 0;
    QTextEdit* m_overview = 0;
    QTextEdit* m_log = 0;
    QProcess* m_process = 0;
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("Defcoin Core Nu Legacy");
    QApplication::setOrganizationName("Defcoin Core");
    QApplication::setOrganizationDomain("defcoincore.org");

    MainWindow window;
    window.show();
    return app.exec();
}

#include "main.moc"
