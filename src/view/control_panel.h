#pragma once

#include <QString>
#include <QVector>
#include <QWidget>

#include "shared/app_settings.h"
#include "shared/life_stats.h"
#include "view/view_state.h"

class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QSlider;
class QSpinBox;

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
 * Passive side panel: controllers push display state into it and it emits
 * user intents. It has no model or controller dependency.
 */
class ControlPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ControlPanel(QWidget* parent = nullptr);

    void setRuleCatalog(const RuleCatalog* catalog);
    void applyState(const PanelState& state);
    void setStats(const shared::LifeStats& stats);
    void setRunning(bool running);
    void setBoardSize(int width, int height);
    void setRuleNotation(const QString& notation);
    void setBoundaryIndex(int index);
    void setOldestGeneration(int generation);
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

private:
    void setupUi();
    void syncRuleSelection();
    void syncBoundarySelection();
    void updateJumpState();
    void updateSpeedLabel();

    const RuleCatalog* m_ruleCatalog = nullptr;
    QString m_ruleNotation = QStringLiteral("B3/S23");
    int m_boundaryIndex = 0;

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
    int m_speed = shared::limits::kDefaultSpeed;

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
    int m_population = 0;
    double m_density = -1.0;
    int m_oldestGeneration = 0;
    int m_panelWidth = 0;
    int m_panelHeight = 0;
    bool m_running = false;
    bool m_syncing = false;
};

} // namespace lifegame::view
