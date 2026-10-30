#include <QSignalSpy>
#include <QtTest>

#include <string>

#include "domain/pattern_library.h"
#include "domain/rle.h"
#include "model/life_model.h"

namespace
{

bool waitFor(QSignalSpy& spy, int timeoutMs = 5000)
{
    return spy.count() > 0 || spy.wait(timeoutMs);
}

QString rle(const lifegame::model::LifeModel& model)
{
    return QString::fromStdString(model.toRle());
}

void place(lifegame::model::LifeModel* model, const std::string& id, int row, int col)
{
    const auto* pattern = lifegame::domain::findBuiltinPattern(id);
    QVERIFY(pattern != nullptr);
    QVERIFY(model->placePattern(*pattern, row, col));
}

} // namespace

class ModelTests : public QObject
{
    Q_OBJECT

private slots:
    void defaults();
    void setCellUpdatesStatsAndSignals();
    void editingPausesSimulation();
    void emptyBoardCannotStart();
    void synchronousEvolution();
    void randomizeProducesVariation();
    void resizeAndLoadDocument();
    void ruleAndBoundary();
    void asyncStep();
    void largeAsyncStep();
    void jumpForwardAndBackward();
    void jumpCanBeCancelled();
    void loadWhileJumpingIsRejected();
    void snapshotInterval();
};

void ModelTests::defaults()
{
    lifegame::model::LifeModel model;
    QCOMPARE(model.width(), lifegame::domain::kDefaultBoardWidth);
    QCOMPARE(model.height(), lifegame::domain::kDefaultBoardHeight);
    QCOMPARE(model.stats().generation, 0);
    QCOMPARE(model.stats().population, 0);
    QCOMPARE(model.stats().density, 0.0);
    QCOMPARE(model.rule().notation(), std::string("B3/S23"));
    QCOMPARE(model.boundary(), lifegame::domain::Boundary::Dead);
    QCOMPARE(model.isRunning(), false);
    QCOMPARE(model.isJumping(), false);
    QCOMPARE(model.oldestGeneration(), 0);
    QCOMPARE(model.snapshotInterval(), 100);
}

void ModelTests::setCellUpdatesStatsAndSignals()
{
    lifegame::model::LifeModel model;
    QVERIFY(model.resizeBoard(20, 20));

    QSignalSpy boardSpy(&model, &lifegame::model::LifeModel::boardChanged);
    QSignalSpy statusSpy(&model, &lifegame::model::LifeModel::statusChanged);

    QVERIFY(model.setCell(10, 10, true));
    QCOMPARE(model.stats().population, 1);
    QCOMPARE(model.stats().density, 0.3); // 1 / 400 * 100 = 0.25 -> 0.3
    QCOMPARE(boardSpy.count(), 1);
    QCOMPARE(statusSpy.count(), 1);

    // Same value is a no-op and emits nothing.
    QVERIFY(!model.setCell(10, 10, true));
    QCOMPARE(boardSpy.count(), 1);

    // Out of bounds is rejected.
    QVERIFY(!model.setCell(-1, 0, true));
    QVERIFY(!model.setCell(20, 20, true));

    QVERIFY(model.toggleCell(10, 10)); // mutation happened: alive -> dead
    QCOMPARE(model.stats().population, 0);
    QVERIFY(model.toggleCell(10, 10)); // mutation happened: dead -> alive
    QCOMPARE(model.stats().population, 1);

    QVERIFY(model.clear());
    QCOMPARE(model.stats().population, 0);
    QCOMPARE(model.stats().generation, 0);
}

void ModelTests::editingPausesSimulation()
{
    lifegame::model::LifeModel model;
    QVERIFY(model.resizeBoard(20, 20));
    QVERIFY(model.setCell(5, 5, true));
    QVERIFY(model.setRunning(true));
    QCOMPARE(model.isRunning(), true);

    QSignalSpy runningSpy(&model, &lifegame::model::LifeModel::runningChanged);
    QVERIFY(model.setCell(6, 6, true));
    QCOMPARE(model.isRunning(), false);
    QCOMPARE(runningSpy.count(), 1);
}

void ModelTests::emptyBoardCannotStart()
{
    lifegame::model::LifeModel model;
    QVERIFY(model.clear());
    QSignalSpy stalledSpy(&model, &lifegame::model::LifeModel::simulationStalled);
    QVERIFY(!model.setRunning(true));
    QCOMPARE(model.isRunning(), false);
    QCOMPARE(stalledSpy.count(), 1);
    if (!stalledSpy.isEmpty())
    {
        QCOMPARE(stalledSpy.first().first().value<lifegame::model::StallReason>(),
                 lifegame::model::StallReason::EmptyBoard);
    }
}

void ModelTests::synchronousEvolution()
{
    lifegame::model::LifeModel model;
    QVERIFY(model.resizeBoard(20, 20));
    place(&model, "blinker", 10, 10);
    QCOMPARE(model.stats().population, 3);

    QSignalSpy boardSpy(&model, &lifegame::model::LifeModel::boardChanged);
    model.step();
    QCOMPARE(model.stats().generation, 1);
    QCOMPARE(boardSpy.count(), 1);
    QCOMPARE(model.board().cell(9, 10), true);
    QCOMPARE(model.board().cell(10, 10), true);
    QCOMPARE(model.board().cell(11, 10), true);
    QCOMPARE(model.stats().population, 3);

    model.step();
    QCOMPARE(model.stats().generation, 2);
    QCOMPARE(model.board().cell(10, 9), true);
    QCOMPARE(model.board().cell(10, 10), true);
    QCOMPARE(model.board().cell(10, 11), true);
}

void ModelTests::randomizeProducesVariation()
{
    lifegame::model::LifeModel model;
    QVERIFY(model.resizeBoard(30, 30));
    QVERIFY(model.randomize(35));
    const QString first = rle(model);
    QVERIFY(model.stats().population > 0);
    QCOMPARE(model.stats().generation, 0);

    QVERIFY(model.randomize(35));
    const QString second = rle(model);
    QVERIFY(first != second);
}

void ModelTests::resizeAndLoadDocument()
{
    lifegame::model::LifeModel model;
    QVERIFY(model.resizeBoard(35, 25));
    place(&model, "glider", 10, 10);
    const QString saved = rle(model);

    const auto decoded = lifegame::domain::decodeRle(saved.toStdString());
    QVERIFY(decoded.document.has_value());

    lifegame::model::LifeModel loaded;
    QSignalSpy resizedSpy(&loaded, &lifegame::model::LifeModel::boardResized);
    QVERIFY(loaded.loadDocument(*decoded.document));
    QCOMPARE(loaded.width(), 35);
    QCOMPARE(loaded.height(), 25);
    QCOMPARE(loaded.stats().population, model.stats().population);
    QCOMPARE(loaded.stats().generation, 0);
    QCOMPARE(resizedSpy.count(), 1);

    // Small RLE patterns are padded to the minimum board size.
    lifegame::model::LifeModel padded;
    const auto glider = lifegame::domain::decodeRle("x = 3, y = 3\nbob$2bo$3o!");
    QVERIFY(glider.document.has_value());
    QVERIFY(padded.loadDocument(*glider.document));
    QCOMPARE(padded.width(), lifegame::domain::kMinBoardExtent);
    QCOMPARE(padded.height(), lifegame::domain::kMinBoardExtent);
    QCOMPARE(padded.stats().population, 5);

    // Invalid rule in the document is rejected and leaves the model untouched.
    lifegame::domain::DecodedDocument invalid;
    invalid.pattern = lifegame::domain::Pattern::fromAscii("bad", {"O"});
    invalid.pattern.width = 1;
    invalid.rule = "not-a-rule";
    QVERIFY(!padded.loadDocument(invalid));
    QCOMPARE(padded.stats().population, 5);
}

void ModelTests::ruleAndBoundary()
{
    lifegame::model::LifeModel model;
    QVERIFY(model.resizeBoard(20, 20));
    QSignalSpy configSpy(&model, &lifegame::model::LifeModel::configurationChanged);

    QVERIFY(model.setRuleByNotation("B36/S23"));
    QCOMPARE(model.rule().notation(), std::string("B36/S23"));
    QCOMPARE(configSpy.count(), 1);
    QVERIFY(!model.setRuleByNotation("bad"));
    QCOMPARE(configSpy.count(), 1);

    QVERIFY(model.setBoundary(lifegame::domain::Boundary::Wrap));
    QCOMPARE(model.boundary(), lifegame::domain::Boundary::Wrap);
    QCOMPARE(configSpy.count(), 2);
    QVERIFY(!model.setBoundary(lifegame::domain::Boundary::Wrap));
    QCOMPARE(configSpy.count(), 2);
}

void ModelTests::asyncStep()
{
    lifegame::model::LifeModel model;
    QVERIFY(model.resizeBoard(20, 20));
    place(&model, "glider", 10, 10);
    QVERIFY(model.setRunning(true));

    QSignalSpy boardSpy(&model, &lifegame::model::LifeModel::boardChanged);
    model.requestStep();
    QVERIFY(waitFor(boardSpy));
    QTRY_COMPARE(model.stats().generation, 1);
    QCOMPARE(model.isRunning(), true);

    QVERIFY(model.setRunning(false));
    const int generation = model.stats().generation;
    model.requestStep();
    QTest::qWait(50);
    QCOMPARE(model.stats().generation, generation);
}

void ModelTests::largeAsyncStep()
{
    lifegame::model::LifeModel model;
    QVERIFY(model.resizeBoard(400, 400));
    QVERIFY(model.randomize(30));
    QVERIFY(model.setRunning(true));

    QSignalSpy boardSpy(&model, &lifegame::model::LifeModel::boardChanged);
    model.requestStep();
    QVERIFY(waitFor(boardSpy, 8000));
    QTRY_COMPARE(model.stats().generation, 1);
    QCOMPARE(model.width(), 400);
    QCOMPARE(model.height(), 400);
    QVERIFY(model.stats().population > 0);
}

void ModelTests::jumpForwardAndBackward()
{
    lifegame::model::LifeModel reference;
    QVERIFY(reference.resizeBoard(60, 40));
    place(&reference, "glider", 20, 20);
    for (int i = 0; i < 250; ++i)
    {
        reference.step();
    }

    lifegame::model::LifeModel model;
    QVERIFY(model.resizeBoard(60, 40));
    place(&model, "glider", 20, 20);
    QVERIFY(model.setRunning(true));

    QSignalSpy startedSpy(&model, &lifegame::model::LifeModel::jumpStarted);
    QSignalSpy finishedSpy(&model, &lifegame::model::LifeModel::jumpFinished);
    QSignalSpy progressSpy(&model, &lifegame::model::LifeModel::jumpProgress);

    QVERIFY(model.startJump(250));
    QCOMPARE(model.isRunning(), false);
    QCOMPARE(model.isJumping(), true);
    QVERIFY(waitFor(finishedSpy, 8000));
    QCOMPARE(model.isJumping(), false);
    QCOMPARE(model.stats().generation, 250);
    QCOMPARE(rle(model), rle(reference));
    QCOMPARE(startedSpy.count(), 1);
    QVERIFY(progressSpy.count() >= 1);

    finishedSpy.clear();
    QVERIFY(model.startJump(120));
    QVERIFY(waitFor(finishedSpy, 5000));
    QCOMPARE(model.stats().generation, 120);

    lifegame::model::LifeModel reference120;
    QVERIFY(reference120.resizeBoard(60, 40));
    place(&reference120, "glider", 20, 20);
    for (int i = 0; i < 120; ++i)
    {
        reference120.step();
    }
    QCOMPARE(rle(model), rle(reference120));

    finishedSpy.clear();
    QVERIFY(model.startJump(0));
    QVERIFY(waitFor(finishedSpy, 5000));
    QCOMPARE(model.stats().generation, 0);
    QCOMPARE(model.oldestGeneration(), 0);

    QVERIFY(!model.startJump(-1));
    QVERIFY(!model.startJump(0));
}

void ModelTests::jumpCanBeCancelled()
{
    lifegame::model::LifeModel model;
    QVERIFY(model.resizeBoard(60, 40));
    place(&model, "glider", 20, 20);

    QSignalSpy cancelledSpy(&model, &lifegame::model::LifeModel::jumpCancelled);
    QSignalSpy finishedSpy(&model, &lifegame::model::LifeModel::jumpFinished);
    const QString before = rle(model);
    QVERIFY(model.startJump(200000));
    model.cancelJump();
    QVERIFY(waitFor(cancelledSpy, 8000));
    QCOMPARE(model.isJumping(), false);
    QCOMPARE(model.stats().generation, 0);
    QCOMPARE(rle(model), before);
    QCOMPARE(finishedSpy.count(), 0);
}

void ModelTests::loadWhileJumpingIsRejected()
{
    lifegame::model::LifeModel model;
    QVERIFY(model.resizeBoard(60, 40));
    place(&model, "glider", 20, 20);

    const auto document = lifegame::domain::decodeRle("x = 3, y = 3, rule = B36/S23\nbob$2bo$3o!");
    QVERIFY(document.document.has_value());

    QSignalSpy finishedSpy(&model, &lifegame::model::LifeModel::jumpFinished);
    QVERIFY(model.startJump(5000));
    QCOMPARE(model.rule().notation(), std::string("B3/S23"));
    QVERIFY(!model.loadDocument(*document.document));
    QCOMPARE(model.rule().notation(), std::string("B3/S23"));
    QCOMPARE(model.stats().population, 5);

    QVERIFY(waitFor(finishedSpy, 8000));
    QCOMPARE(model.rule().notation(), std::string("B3/S23"));
    QCOMPARE(model.stats().generation, 5000);
}

void ModelTests::snapshotInterval()
{
    lifegame::model::LifeModel model;
    QVERIFY(model.resizeBoard(20, 20));

    QVERIFY(model.setSnapshotInterval(50));
    QCOMPARE(model.snapshotInterval(), 50);
    QVERIFY(!model.setSnapshotInterval(50));
    QVERIFY(model.setSnapshotInterval(1)); // clamped to the minimum
    QCOMPARE(model.snapshotInterval(), lifegame::model::limits::kSnapshotIntervalMin);
    QVERIFY(model.setSnapshotInterval(5000)); // clamped to the maximum
    QCOMPARE(model.snapshotInterval(), lifegame::model::limits::kSnapshotIntervalMax);
}

QTEST_GUILESS_MAIN(ModelTests)

#include "model_tests.moc"
