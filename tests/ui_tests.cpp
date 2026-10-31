#include <QAction>
#include <QApplication>
#include <QDir>
#include <QLabel>
#include <QCheckBox>
#include <QSettings>
#include <QSpinBox>
#include <QTemporaryDir>
#include <QtTest>

#include <algorithm>
#include <vector>

#include "app/main_window.h"
#include "view/board_view.h"
#include "view/interfaces.h"
#include "view/theme.h"

namespace
{

class AcceptConfirm : public lifegame::view::ConfirmSink
{
public:
    bool confirm(const QString&, const QString&) override
    {
        return true;
    }
};

class RejectConfirm : public lifegame::view::ConfirmSink
{
public:
    bool confirm(const QString&, const QString&) override
    {
        return false;
    }
};

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
    void clearRequiresConfirmation();
    void spaceDoesNotStealFocusedControl();
    void gridActionMirrorsPanel();

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
    AcceptConfirm confirm;
    lifegame::app::MainWindow window(nullptr, &confirm);
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

void UiTests::clearRequiresConfirmation()
{
    RejectConfirm reject;
    lifegame::app::MainWindow window(nullptr, &reject);
    window.resize(1180, 820);
    window.show();
    QTest::qWait(100);

    const std::vector<QLabel*> labels = statLabels(window);
    QVERIFY(labels.size() >= 2);
    const QString before = labels[1]->text();
    QVERIFY(before != QStringLiteral("0"));

    QAction* clear = findAction(window, QStringLiteral("清空"));
    QVERIFY(clear != nullptr);
    clear->trigger();
    QTest::qWait(50);
    QCOMPARE(labels[1]->text(), before); // rejected confirmation leaves the board intact
    window.close();
}

void UiTests::spaceDoesNotStealFocusedControl()
{
    AcceptConfirm confirm;
    lifegame::app::MainWindow window(nullptr, &confirm);
    window.resize(1180, 820);
    window.show();
    QTest::qWait(100);

    QAction* play = findAction(window, QStringLiteral("开始"));
    QVERIFY(play != nullptr);

    QSpinBox* density = nullptr;
    for (QSpinBox* spin : window.findChildren<QSpinBox*>())
    {
        if (spin->accessibleName().contains(QStringLiteral("密度")))
        {
            density = spin;
            break;
        }
    }
    QVERIFY(density != nullptr);
    density->setFocus();
    QTest::keyClick(density, Qt::Key_Space);
    QTest::qWait(50);
    QVERIFY(play->text().contains(QStringLiteral("开始")));

    auto* board = window.findChild<lifegame::view::BoardView*>();
    QVERIFY(board != nullptr);
    board->setFocus();
    QTest::keyClick(board, Qt::Key_Space);
    QTRY_VERIFY(play->text().contains(QStringLiteral("暂停")));
    play->trigger();
    window.close();
}

void UiTests::gridActionMirrorsPanel()
{
    AcceptConfirm confirm;
    lifegame::app::MainWindow window(nullptr, &confirm);
    window.show();
    QTest::qWait(100);

    QAction* gridAction = findAction(window, QStringLiteral("显示网格线"));
    QVERIFY(gridAction != nullptr);
    QVERIFY(gridAction->isCheckable());

    QCheckBox* gridCheck = nullptr;
    for (QCheckBox* check : window.findChildren<QCheckBox*>())
    {
        if (check->accessibleName().contains(QStringLiteral("网格")))
        {
            gridCheck = check;
            break;
        }
    }
    QVERIFY(gridCheck != nullptr);

    const bool before = gridAction->isChecked();
    gridCheck->setChecked(!before);
    QTest::qWait(50);
    QCOMPARE(gridAction->isChecked(), !before);
    window.close();
}

QTEST_MAIN(UiTests)

#include "ui_tests.moc"
