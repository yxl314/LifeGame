#include <QApplication>
#include <QLabel>
#include <QSignalSpy>
#include <QSlider>
#include <QtTest>

#include <algorithm>
#include <vector>

#include "domain/grid.h"
#include "domain/pattern_library.h"
#include "view/board_view.h"
#include "view/jump_progress_dialog.h"
#include "view/control_panel.h"
#include "view/pattern_catalog.h"
#include "view/rule_catalog.h"
#include "view/settings_dialog.h"
#include "view/speed_scale.h"

namespace
{

std::vector<QLabel*> statLabels(QWidget& panel)
{
    std::vector<QLabel*> labels;
    for (QLabel* label : panel.findChildren<QLabel*>(QStringLiteral("StatValue")))
    {
        labels.push_back(label);
    }
    std::sort(labels.begin(), labels.end(), [&panel](QLabel* left, QLabel* right) {
        return left->mapTo(&panel, QPoint(0, 0)).x() < right->mapTo(&panel, QPoint(0, 0)).x();
    });
    return labels;
}

} // namespace

class ViewTests : public QObject
{
    Q_OBJECT

private slots:
    void speedScale();
    void speedSliderDoesNotWriteBack();
    void controlPanelDisplaysPushedState();
    void boardViewRendersPushedStateAndCapsExport();
    void catalogs();
    void settingsDialogRoundTrip();
    void progressDialogProgrammaticCloseDoesNotCancel();
};

void ViewTests::speedScale()
{
    QCOMPARE(lifegame::view::speedForSliderPosition(0), 1);
    QCOMPARE(lifegame::view::speedForSliderPosition(100), 200);
    QCOMPARE(lifegame::view::sliderPositionForSpeed(1), 0);
    QCOMPARE(lifegame::view::sliderPositionForSpeed(200), 100);
}

void ViewTests::speedSliderDoesNotWriteBack()
{
    lifegame::view::ControlPanel panel;
    panel.setSpeed(10);

    auto* slider = panel.findChild<QSlider*>();
    QVERIFY(slider != nullptr);
    QCOMPARE(slider->value(), lifegame::view::sliderPositionForSpeed(10));

    QSignalSpy speedSpy(&panel, &lifegame::view::ControlPanel::speedChanged);
    slider->setValue(8);
    QCOMPARE(speedSpy.count(), 1);
    QCOMPARE(speedSpy.first().first().toInt(), 2);

    // The panel must not snap the thumb back while the user is dragging.
    QCOMPARE(slider->value(), 8);
    QCOMPARE(panel.speed(), 2);
}

void ViewTests::controlPanelDisplaysPushedState()
{
    lifegame::view::ControlPanel panel;
    lifegame::view::RuleCatalog catalog;
    panel.setRuleCatalog(&catalog);
    panel.setBoardSize(20, 20);

    const auto labels = statLabels(panel);
    QCOMPARE(labels.size(), 3);
    panel.setStats({0, 0, 0.0});
    QCOMPARE(labels[0]->text(), QStringLiteral("0"));
    QCOMPARE(labels[1]->text(), QStringLiteral("0"));

    panel.setStats({0, 1, 0.25});
    QCOMPARE(labels[0]->text(), QStringLiteral("0"));
    QCOMPARE(labels[1]->text(), QStringLiteral("1"));
    QCOMPARE(labels[2]->text(), QStringLiteral("0.3%"));

    panel.setStats({1, 0, 0.0});
    QCOMPARE(labels[0]->text(), QStringLiteral("1"));
    QCOMPARE(labels[1]->text(), QStringLiteral("0"));
}

void ViewTests::boardViewRendersPushedStateAndCapsExport()
{
    lifegame::domain::Grid grid(20, 20);
    grid.setCell(5, 5, true);

    lifegame::view::BoardView view;
    view.setBoard(&grid);
    view.setPopulation(grid.population());
    view.resize(640, 480);
    view.refresh();

    const QImage image = view.renderBoardImage();
    QVERIFY(!image.isNull());
    QVERIFY(image.width() > 0);
    QVERIFY(image.height() > 0);

    lifegame::domain::Grid largeGrid(400, 400);
    largeGrid.setCell(0, 0, true);
    view.setBoard(&largeGrid);
    view.setPopulation(largeGrid.population());
    const QImage large = view.renderBoardImage();
    QVERIFY(!large.isNull());
    QVERIFY(large.width() <= 4096);
    QVERIFY(large.height() <= 4096);
}

void ViewTests::catalogs()
{
    lifegame::view::RuleCatalog rules;
    QCOMPARE(rules.presets().size(), std::size_t{5});
    QCOMPARE(rules.names().size(), 5);
    QCOMPARE(rules.nameForNotation(QStringLiteral("B3/S23")), QStringLiteral("康威生命"));
    QVERIFY(rules.notationAt(0) == QStringLiteral("B3/S23"));
    QVERIFY(rules.notationAt(-1).isEmpty());

    lifegame::view::PatternCatalog patterns;
    QCOMPARE(patterns.all().size(), std::size_t{13});
    const auto* glider = patterns.find("glider");
    QVERIFY(glider != nullptr);
    QCOMPARE(glider->name, QStringLiteral("滑翔机"));
    QVERIFY(patterns.displayLabel(lifegame::domain::builtinPatterns().front()).contains(QStringLiteral("滑翔机")));
}

void ViewTests::settingsDialogRoundTrip()
{
    lifegame::view::SettingsDialog dialog;
    lifegame::shared::AppSettings values;
    values.jumpStep = 500;
    values.sizeStep = 10;
    values.snapshotInterval = 250;
    values.density = 55;
    values.speed = 42;
    values.gridVisible = false;
    dialog.setValues(values);

    const lifegame::shared::AppSettings result = dialog.values();
    QCOMPARE(result.jumpStep, 500);
    QCOMPARE(result.sizeStep, 10);
    QCOMPARE(result.snapshotInterval, 250);
    // Fields the dialog does not edit must survive the round-trip.
    QCOMPARE(result.density, 55);
    QCOMPARE(result.speed, 42);
    QCOMPARE(result.gridVisible, false);
}

void ViewTests::progressDialogProgrammaticCloseDoesNotCancel()
{
    bool cancelled = false;
    QWidget parent;
    lifegame::view::JumpProgressDialog dialog(&parent);
    dialog.setCancelHandler([&cancelled] { cancelled = true; });

    lifegame::shared::JumpInfo info;
    info.fromGeneration = 0;
    info.toGeneration = 100;
    info.replayGenerations = 100;
    dialog.show(info);

    // Simulates JumpController::closeProgress() during shutdown/finish.
    dialog.close();
    QVERIFY(!cancelled);
}

QTEST_MAIN(ViewTests)

#include "view_tests.moc"
