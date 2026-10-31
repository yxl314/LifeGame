#pragma once

#include <QString>
#include <QVector>
#include <QWidget>

#include "model/life_stats.h"

class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QSlider;
class QSpinBox;

namespace lifegame::model
{
class LifeModel;
}

namespace lifegame::view
{

class RuleCatalog;

/// One entry in the pattern combo box; id -1 means "brush".
struct PatternOption
{
    QString text;
    int id = -1;
};

/**
 * Side panel: observes the model for statistics/configuration and emits
 * intents for the controllers. It never includes or mutates the model.
 */
class ControlPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ControlPanel(QWidget* parent = nullptr);

    void setModel(const model::LifeModel* model);
    void setRuleCatalog(const RuleCatalog* catalog);
    void setPatternOptions(const QVector<PatternOption>& options);
    void setPatternDescription(const QString& text);

    void setSpeed(int speed);
    int speed() const;
    int randomDensity() const;
    void setRandomDensity(int density);
    bool isGridVisible() const;
    void setGridVisible(bool visible);
    void setJumpStep(int step);
    int jumpGenerations() const;
    void setSizeStep(int step);

signals:
    void playPauseRequested();
    void stepRequested();
    void clearRequested();
    void randomizeRequested(int density);
    void speedChanged(int speed);
    void rewindRequested(int generations);
    void fastForwardRequested(int generations);
    void patternChanged(int patternId);
    void ruleChanged(int index);
    void boundaryChanged(int index);
    void resizeRequested(int width, int height);
    void gridVisibilityChanged(bool visible);

private slots:
    void onStatusChanged(const model::LifeStats& stats);
    void onRunningChanged(bool running);
    void onBoardResized(int width, int height);
    void onConfigurationChanged();

private:
    void setupUi();
    void syncRuleSelection();
    void syncBoundarySelection();
    void updateJumpState();
    void updateSpeedLabel();

    const model::LifeModel* m_model = nullptr;
    const RuleCatalog* m_ruleCatalog = nullptr;

    QLabel* m_generationValue = nullptr;
    QLabel* m_populationValue = nullptr;
    QLabel* m_densityValue = nullptr;

    QPushButton* m_playButton = nullptr;
    QPushButton* m_stepButton = nullptr;
    QPushButton* m_clearButton = nullptr;
    QPushButton* m_randomButton = nullptr;
    QSpinBox* m_densitySpin = nullptr;

    QSlider* m_speedSlider = nullptr;
    QLabel* m_speedValue = nullptr;
    int m_speed = 10;

    QSpinBox* m_jumpSpin = nullptr;
    int m_jumpStep = 100;
    QPushButton* m_rewindButton = nullptr;
    QPushButton* m_forwardButton = nullptr;
    QPushButton* m_applySizeButton = nullptr;

    QComboBox* m_patternCombo = nullptr;
    QLabel* m_patternText = nullptr;

    QComboBox* m_ruleCombo = nullptr;
    QComboBox* m_boundaryCombo = nullptr;

    QSpinBox* m_widthSpin = nullptr;
    QSpinBox* m_heightSpin = nullptr;
    QCheckBox* m_gridCheck = nullptr;

    int m_currentGeneration = 0;
    int m_oldestGeneration = 0;
    bool m_syncing = false;
};

} // namespace lifegame::view
