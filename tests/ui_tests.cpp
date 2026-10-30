#include <QAction>
#include <QApplication>
#include <QDir>
#include <QLabel>
#include <QSettings>
#include <QTemporaryDir>
#include <QtTest>

#include <algorithm>
#include <vector>

#include "app/main_window.h"
#include "view/theme.h"

namespace
{

QAction* findAction(QWidget& window, const QString& text)
{
    for (QAction* action : window.findChildren<QAction*>())
    {
        if (action->text().contains(text))
        {
            return action;
        }
    }
    return nullptr;
}

std::vector<QLabel*> statLabels(QWidget& window)
{
    std::vector<QLabel*> labels;
    for (QLabel* label : window.findChildren<QLabel*>(QStringLiteral("StatValue")))
    {
        labels.push_back(label);
    }
    std::sort(labels.begin(), labels.end(), [&window](QLabel* left, QLabel* right) {
        return left->mapTo(&window, QPoint(0, 0)).x() < right->mapTo(&window, QPoint(0, 0)).x();
    });
    return labels;
}

} // namespace

class UiTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void startPlayClear();

private:
    QTemporaryDir m_settingsDir;
};

void UiTests::initTestCase()
{
    QVERIFY(m_settingsDir.isValid());
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_settingsDir.path());
    QCoreApplication::setOrganizationName(QStringLiteral("life-game-test"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("life-game.local"));
    QCoreApplication::setApplicationName(QStringLiteral("ui-test"));

    QSettings settings;
    settings.clear();
    settings.sync();

    lifegame::view::theme::apply(*qApp);
}

void UiTests::startPlayClear()
{
    lifegame::app::MainWindow window;
    window.resize(1180, 820);
    window.show();
    QTest::qWait(100);

    const std::vector<QLabel*> labels = statLabels(window);
    QVERIFY(labels.size() >= 3);
    QLabel* generation = labels[0];
    QLabel* population = labels[1];
    QVERIFY(generation != nullptr);
    QVERIFY(population != nullptr);

    const QString before = generation->text();
    QAction* play = findAction(window, QStringLiteral("开始"));
    QVERIFY(play != nullptr);
    play->trigger();
    QTRY_VERIFY(generation->text() != before);
    QVERIFY(population->text() != QStringLiteral("0"));
    QVERIFY(window.grab().save(QDir::tempPath() + QStringLiteral("/lifegame-ui-tests.png")));

    QAction* clear = findAction(window, QStringLiteral("清空"));
    QVERIFY(clear != nullptr);
    clear->trigger();
    QTRY_COMPARE(population->text(), QStringLiteral("0"));

    window.close();
}

QTEST_MAIN(UiTests)

#include "ui_tests.moc"
