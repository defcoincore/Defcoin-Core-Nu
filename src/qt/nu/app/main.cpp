#include <QApplication>
#include <QFont>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QDir>
#include <QQuickWindow>
#include <QTimer>
#include <QUrl>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("Defcoin Core Nu");
    QApplication::setApplicationDisplayName("Defcoin Core Nu");
    QApplication::setOrganizationName("Defcoin Core");
    QApplication::setOrganizationDomain("defcoincore.org");
    QApplication::setFont(QFont("Helvetica Neue"));

    QQuickStyle::setStyle("Basic");

    QQmlApplicationEngine engine;
    const QString resourceRoot = QDir(QCoreApplication::applicationDirPath()).filePath("../Resources/nu");
    engine.addImportPath(resourceRoot + "/qml");
    engine.addImportPath(resourceRoot);

    const QUrl mainUrl = QUrl::fromLocalFile(resourceRoot + "/qml/Main.qml");
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, [] {
        QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(mainUrl);

    if (!engine.rootObjects().isEmpty()) {
        if (auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().constFirst())) {
            window->show();
            window->raise();
            window->requestActivate();
        }
    }

    if (app.arguments().contains("--smoke-test")) {
        QTimer::singleShot(1000, &app, &QCoreApplication::quit);
    }

    return app.exec();
}
