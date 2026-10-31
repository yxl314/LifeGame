#include <QApplication>
#include <QLabel>
#include <QSignalSpy>
#include <QSlider>
#include <QtTest>

#include <algorithm>
#include <vector>

#include "domain/pattern_library.h"
#include "model/life_model.h"

#define VERIFY_OK(expression) QVERIFY(lifegame::model::succeeded(expression))
#define VERIFY_FAIL(expression) QVERIFY(lifegame::model::failed(expression))
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
    void controlPanelObservesModel();
    void boardViewObservesModelAndCapsExport();
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

void ViewTests::controlPanelObservesModel()
{
    lifegame::model::LifeModel model;
    VERIFY_OK(model.resizeBoard(20, 20));

    lifegame::view::ControlPanel panel;
    lifegame::view::RuleCatalog catalog;
    panel.setRuleCatalog(&catalog);
    panel.setModel(&model);

    const auto labels = statLabels(panel);
    QCOMPARE(labels.size(), 3);
    QCOMPARE(labels[0]->text(), QStringLiteral("0"));
    QCOMPARE(labels[1]->text(), QStringLiteral("0"));

    QVERIFY(model.setCell(5, 5, true));
    QCOMPARE(labels[0]->text(), QStringLiteral("0"));
    QCOMPARE(labels[1]->text(), QStringLiteral("1"));
    QCOMPARE(labels[2]->text(), QStringLiteral("0.3%"));

    model.step();
    QCOMPARE(labels[0]->text(), QStringLiteral("1"));
    QCOMPARE(labels[1]->text(), QStringLiteral("0"));
}

void ViewTests::boardViewObservesModelAndCapsExport()
{
    lifegame::model::LifeModel model;
    VERIFY_OK(model.resizeBoard(20, 20));

    lifegame::view::BoardView view;
    view.setModel(&model);
    view.resize(640, 480);
    QVERIFY(model.setCell(5, 5, true));

    const QImage image = view.renderBoardImage();
    QVERIFY(!image.isNull());
    QVERIFY(image.width() > 0);
    QVERIFY(image.height() > 0);

    VERIFY_OK(model.resizeBoard(400, 400));
    QVERIFY(model.setCell(0, 0, true));
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
    lifegame::model::AppSettings values;
    values.jumpStep = 500;
    values.sizeStep = 10;
    values.snapshotInterval = 250;
    values.density = 55;
    values.speed = 42;
    values.gridVisible = false;
    dialog.setValues(values);

    const lifegame::model::AppSettings result = dialog.values();
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

    lifegame::model::JumpInfo info;
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
