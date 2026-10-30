#include <QApplication>
#include <QSettings>
#include <QTemporaryDir>
#include <QtTest>

#include <algorithm>
#include <functional>
#include <memory>
#include <vector>

#include "controller/board_controller.h"
#include "controller/jump_controller.h"
#include "controller/line_raster.h"
#include "controller/session_controller.h"
#include "controller/simulation_controller.h"
#include "domain/pattern_library.h"
#include "domain/rle.h"
#include "model/life_model.h"
#include "service/rle_file_service.h"
#include "service/session_state.h"
#include "service/settings_store.h"
#include "view/board_view.h"
#include "view/control_panel.h"
#include "view/interfaces.h"
#include "view/pattern_catalog.h"

namespace
{

class FakeMessages : public lifegame::view::MessageSink
{
public:
    void showMessage(const QString& text, int timeoutMs) override
    {
        Q_UNUSED(timeoutMs)
        last = text;
        ++count;
    }

    QString last;
    int count = 0;
};

struct JumpState
{
    bool shown = false;
    bool closed = false;
    int progress = 0;
    lifegame::model::JumpInfo info;
    std::function<void()> cancel;
};

class FakeJumpProgress : public lifegame::view::JumpProgressView
{
public:
    explicit FakeJumpProgress(JumpState* state)
        : m_state(state)
    {
    }

    void show(const lifegame::model::JumpInfo& info) override
    {
        m_state->shown = true;
        m_state->info = info;
    }

    void setProgress(int done) override
    {
        m_state->progress = done;
    }

    void close() override
    {
        m_state->closed = true;
    }

    void setCancelHandler(std::function<void()> handler) override
    {
        m_state->cancel = std::move(handler);
    }

private:
    JumpState* m_state = nullptr;
};

void place(lifegame::model::LifeModel* model, const std::string& id, int row, int col)
{
    const auto* pattern = lifegame::domain::findBuiltinPattern(id);
    QVERIFY(pattern != nullptr);
    QVERIFY(model->placePattern(*pattern, row, col));
}

} // namespace

class ControllerTests : public QObject
{
    Q_OBJECT

private slots:
    void lineRaster();
    void boardControllerEditsAndStamps();
    void simulationControllerDrivesModel();
    void jumpControllerReportsProgressAndCancel();
    void settingsStoreRoundTrip();
    void rleFileService();
    void sessionControllerRoundTrip();
};

void ControllerTests::lineRaster()
{
    using lifegame::controller::CellPosition;
    using lifegame::controller::rasterLine;

    const auto horizontal = rasterLine(2, 2, 2, 5);
    QCOMPARE(horizontal.size(), std::size_t{4});
    QCOMPARE(horizontal.front().row, 2);
    QCOMPARE(horizontal.front().col, 2);
    QCOMPARE(horizontal.back().col, 5);

    const auto diagonal = rasterLine(0, 0, 4, 4);
    QCOMPARE(diagonal.size(), std::size_t{5});
    for (std::size_t i = 0; i < diagonal.size(); ++i)
    {
        QCOMPARE(diagonal[i].row, diagonal[i].col);
    }

    const auto reverse = rasterLine(5, 5, 2, 2);
    QCOMPARE(reverse.front().row, 5);
    QCOMPARE(reverse.back().row, 2);
    QCOMPARE(reverse.size(), std::size_t{4});

    QCOMPARE(rasterLine(1, 1, 1, 1).size(), std::size_t{1});
}

void ControllerTests::boardControllerEditsAndStamps()
{
    lifegame::model::LifeModel model;
    QVERIFY(model.resizeBoard(20, 20));
    lifegame::view::BoardView view;
    view.setModel(&model);
    lifegame::view::ControlPanel panel;
    lifegame::view::PatternCatalog catalog;
    FakeMessages messages;

    lifegame::controller::BoardController controller(&model, &view, &panel, &catalog, &messages, nullptr);

    emit view.cellPressed(5, 5);
    QVERIFY(model.board().cell(5, 5));
    QCOMPARE(model.stats().population, 1);

    emit view.cellDragged(5, 10);
    for (int col = 5; col <= 10; ++col)
    {
        QVERIFY(model.board().cell(5, col));
    }
    QCOMPARE(model.stats().population, 6);
    emit view.cellReleased(5, 10);

    controller.clearBoard();
    QCOMPARE(model.stats().population, 0);

    emit panel.patternChanged(0); // glider
    emit view.cellPressed(10, 10);
    QCOMPARE(model.stats().population, 5);

    controller.setGridVisible(false);
    QCOMPARE(view.gridVisible(), false);
    QCOMPARE(panel.isGridVisible(), false);
    QCOMPARE(controller.gridVisible(), false);

    controller.loadDemo();
    QVERIFY(model.stats().population > 0);
}

void ControllerTests::simulationControllerDrivesModel()
{
    lifegame::model::LifeModel model;
    QVERIFY(model.resizeBoard(20, 20));
    QVERIFY(model.setCell(5, 5, true));
    lifegame::view::ControlPanel panel;
    FakeMessages messages;
    lifegame::controller::SimulationController controller(&model, &panel, &messages, nullptr);

    controller.stepOnce();
    QCOMPARE(model.stats().generation, 1);
    QCOMPARE(model.isRunning(), false);

    // The single cell died in the step; give the simulation something to run.
    QVERIFY(model.setCell(6, 6, true));
    controller.toggleRunning();
    QCOMPARE(model.isRunning(), true);
    controller.setSpeedExternal(200);
    QTRY_VERIFY(model.stats().generation > 1);
    controller.toggleRunning();
    QCOMPARE(model.isRunning(), false);

    model.clear();
    controller.toggleRunning();
    QCOMPARE(model.isRunning(), false);
    QVERIFY(messages.last.contains(QStringLiteral("棋盘为空")));

    controller.setSpeedExternal(42);
    QCOMPARE(controller.speed(), 42);
    QCOMPARE(panel.speed(), 42);
}

void ControllerTests::jumpControllerReportsProgressAndCancel()
{
    lifegame::model::LifeModel model;
    QVERIFY(model.resizeBoard(60, 40));
    place(&model, "glider", 20, 20);

    lifegame::view::ControlPanel panel;
    FakeMessages messages;
    JumpState state;

    auto factory = [&state]() -> std::unique_ptr<lifegame::view::JumpProgressView> {
        return std::make_unique<FakeJumpProgress>(&state);
    };
    lifegame::controller::JumpController controller(&model, &panel, &messages, factory, nullptr);

    controller.jumpBy(120);
    QTRY_VERIFY(!model.isJumping());
    QCOMPARE(model.stats().generation, 120);
    QVERIFY(state.shown);
    QVERIFY(state.closed);

    controller.jumpBy(100000);
    QVERIFY(state.shown);
    QVERIFY(state.cancel);
    state.cancel();
    QTRY_VERIFY(!model.isJumping());
    QCOMPARE(model.stats().generation, 120);
    QVERIFY(state.closed);
}

void ControllerTests::settingsStoreRoundTrip()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.path() + QStringLiteral("/settings.ini");

    lifegame::service::SessionState state;
    state.settings.jumpStep = 500;
    state.settings.sizeStep = 10;
    state.settings.snapshotInterval = 250;
    state.settings.density = 55;
    state.settings.speed = 42;
    state.settings.gridVisible = false;
    state.boardRle = QStringLiteral("x = 3, y = 2, rule = B3/S23\nbo$2bo$3o!\n");
    state.ruleNotation = QStringLiteral("B36/S23");
    state.boundary = 1;
    state.generation = 77;
    state.windowGeometry = QByteArrayLiteral("geometry");

    {
        auto settings = std::make_unique<QSettings>(path, QSettings::IniFormat);
        lifegame::service::SettingsStore store(std::move(settings));
        store.save(state);
    }

    auto settings = std::make_unique<QSettings>(path, QSettings::IniFormat);
    lifegame::service::SettingsStore store(std::move(settings));
    const lifegame::service::SessionState loaded = store.load();

    QCOMPARE(loaded.settings.jumpStep, 500);
    QCOMPARE(loaded.settings.sizeStep, 10);
    QCOMPARE(loaded.settings.snapshotInterval, 250);
    QCOMPARE(loaded.settings.density, 55);
    QCOMPARE(loaded.settings.speed, 42);
    QCOMPARE(loaded.settings.gridVisible, false);
    QCOMPARE(loaded.boardRle, state.boardRle);
    QCOMPARE(loaded.ruleNotation, state.ruleNotation);
    QCOMPARE(loaded.boundary, 1);
    QCOMPARE(loaded.generation, 77);
    QCOMPARE(loaded.windowGeometry, state.windowGeometry);
}

void ControllerTests::rleFileService()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.path() + QStringLiteral("/glider.rle");
    const QString text = QStringLiteral("#N Glider\nx = 3, y = 3, rule = B3/S23\nbob$2bo$3o!\n");

    lifegame::service::RleFileService service;
    QString error;
    QVERIFY(service.write(path, text, &error));
    QVERIFY(error.isEmpty());

    const lifegame::service::RleReadResult result = service.read(path);
    QVERIFY(result.systemError.isEmpty());
    QCOMPARE(result.decodeError.code, lifegame::domain::RleError::None);
    QVERIFY(result.document.has_value());
    if (result.document)
    {
        QCOMPARE(result.document->pattern.population(), 5);
        QCOMPARE(result.document->rule, std::string("B3/S23"));
    }

    const lifegame::service::RleReadResult missing = service.read(directory.path() + QStringLiteral("/nope.rle"));
    QVERIFY(!missing.systemError.isEmpty());
    QVERIFY(!missing.document.has_value());
}

void ControllerTests::sessionControllerRoundTrip()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.path() + QStringLiteral("/session.ini");

    {
        lifegame::model::LifeModel model;
        lifegame::view::ControlPanel panel;
        lifegame::view::BoardView view;
        lifegame::view::PatternCatalog catalog;
        FakeMessages messages;
        panel.setModel(&model);
        view.setModel(&model);
        lifegame::controller::BoardController board(&model, &view, &panel, &catalog, &messages, nullptr);
        lifegame::controller::SimulationController simulation(&model, &panel, &messages, nullptr);

        auto settings = std::make_unique<QSettings>(path, QSettings::IniFormat);
        lifegame::service::SettingsStore store(std::move(settings));
        lifegame::controller::SessionController session(
            &model, &panel, &simulation, &board, &store, nullptr, &messages, nullptr);
        QVERIFY(!session.restore()); // nothing persisted yet

        QVERIFY(model.setCell(5, 5, true));
        panel.setRandomDensity(55);
        board.setGridVisible(false);
        simulation.setSpeedExternal(42);
        session.save();
    }

    {
        lifegame::model::LifeModel model;
        lifegame::view::ControlPanel panel;
        lifegame::view::BoardView view;
        lifegame::view::PatternCatalog catalog;
        FakeMessages messages;
        panel.setModel(&model);
        view.setModel(&model);
        lifegame::controller::BoardController board(&model, &view, &panel, &catalog, &messages, nullptr);
        lifegame::controller::SimulationController simulation(&model, &panel, &messages, nullptr);

        auto settings = std::make_unique<QSettings>(path, QSettings::IniFormat);
        lifegame::service::SettingsStore store(std::move(settings));
        lifegame::controller::SessionController session(
            &model, &panel, &simulation, &board, &store, nullptr, &messages, nullptr);
        QVERIFY(session.restore());

        QCOMPARE(model.width(), 250);
        QCOMPARE(model.height(), 200);
        QVERIFY(model.board().cell(5, 5));
        QCOMPARE(panel.randomDensity(), 55);
        QCOMPARE(simulation.speed(), 42);
        QCOMPARE(panel.speed(), 42);
        QCOMPARE(board.gridVisible(), false);
    }
}

QTEST_MAIN(ControllerTests)

#include "controller_tests.moc"
