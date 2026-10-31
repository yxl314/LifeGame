#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QtTest>

#include "app/logging.h"
#include "model/model_log.h"

class LoggingTests : public QObject
{
    Q_OBJECT

private slots:
    void writesCategoriesAndVerboseDebug();
    void rotatesOversizedFile();
};

void LoggingTests::writesCategoriesAndVerboseDebug()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.path() + QStringLiteral("/life-game.log");

    lifegame::app::logging::Options options;
    options.filePath = path;
    options.verbose = true;
    options.mirrorToStderr = false;
    QVERIFY(lifegame::app::logging::initialize(options));
    QCOMPARE(lifegame::app::logging::logFilePath(), path);

    qCInfo(lcApp) << "test-event-42";
    qCDebug(lifegame::model::lcModel) << "model-debug-event";
    lifegame::app::logging::shutdown();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = QString::fromUtf8(file.readAll());
    QVERIFY(content.contains(QStringLiteral("test-event-42")));
    QVERIFY(content.contains(QStringLiteral("lifegame.app")));
    QVERIFY(content.contains(QStringLiteral("[I]")));
    QVERIFY(content.contains(QStringLiteral("model-debug-event")));
}

void LoggingTests::rotatesOversizedFile()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.path() + QStringLiteral("/life-game.log");

    QFile old(path);
    QVERIFY(old.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QCOMPARE(old.write(QByteArray(2 * 1024 * 1024 + 10, 'x')), qint64(2 * 1024 * 1024 + 10));
    old.close();

    lifegame::app::logging::Options options;
    options.filePath = path;
    options.mirrorToStderr = false;
    QVERIFY(lifegame::app::logging::initialize(options));
    qCInfo(lcApp) << "after-rotation";
    lifegame::app::logging::shutdown();

    QVERIFY(QFileInfo::exists(path + QStringLiteral(".1")));
    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QVERIFY(QString::fromUtf8(file.readAll()).contains(QStringLiteral("after-rotation")));
}

QTEST_GUILESS_MAIN(LoggingTests)

#include "logging_tests.moc"
