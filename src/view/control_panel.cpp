#include "view/control_panel.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>

#include <algorithm>

#include "domain/board_limits.h"
#include "domain/rule.h"
#include "model/app_settings.h"
#include "model/life_model.h"
#include "view/rule_catalog.h"
#include "view/speed_scale.h"
#include "view/theme.h"

namespace lifegame::view
{

namespace
{

QWidget* makeStat(const QString& caption, QLabel** value, QWidget* parent)
{
    auto* container = new QWidget(parent);
    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(1);

    auto* label = new QLabel(QStringLiteral("0"), container);
    label->setObjectName(QStringLiteral("StatValue"));
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    auto* captionLabel = new QLabel(caption, container);
    captionLabel->setObjectName(QStringLiteral("StatCaption"));

    layout->addWidget(label);
    layout->addWidget(captionLabel);
    *value = label;
    return container;
}

} // namespace

ControlPanel::ControlPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("SidePanel"));
    setMinimumWidth(270);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setupUi();
}

void ControlPanel::setupUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16, 16, 16, 12);
    root->setSpacing(9);

    auto* header = new QHBoxLayout();
    header->setSpacing(10);
    auto* logo = new QLabel(this);
    logo->setPixmap(theme::appIcon().pixmap(36, 36));
    logo->setFixedSize(36, 36);
    logo->setToolTip(tr("生命游戏 · Conway's Game of Life"));

    auto* titles = new QVBoxLayout();
    titles->setSpacing(0);
    auto* title = new QLabel(tr("生命游戏"), this);
    title->setObjectName(QStringLiteral("Title"));
    auto* subtitle = new QLabel(QStringLiteral("CONWAY'S GAME OF LIFE"), this);
    subtitle->setObjectName(QStringLiteral("Subtitle"));
    titles->addWidget(title);
    titles->addWidget(subtitle);

    header->addWidget(logo);
    header->addLayout(titles, 1);
    root->addLayout(header);

    auto* stats = new QFrame(this);
    stats->setObjectName(QStringLiteral("Card"));
    auto* statsLayout = new QHBoxLayout(stats);
    statsLayout->setContentsMargins(12, 10, 12, 10);
    statsLayout->setSpacing(8);
    statsLayout->addWidget(makeStat(tr("代数"), &m_generationValue, stats));
    statsLayout->addWidget(makeStat(tr("存活"), &m_populationValue, stats));
    statsLayout->addWidget(makeStat(tr("密度"), &m_densityValue, stats));
    m_densityValue->setText(QStringLiteral("0.0%"));
    root->addWidget(stats);

    m_playButton = new QPushButton(tr("开始"), this);
    m_playButton->setObjectName(QStringLiteral("PrimaryButton"));
    m_playButton->setMinimumHeight(40);
    m_playButton->setIcon(QIcon(QStringLiteral(":/icons/play.svg")));
    m_playButton->setAccessibleName(tr("开始或暂停"));
    m_playButton->setToolTip(tr("开始或暂停模拟（空格）"));
    connect(m_playButton, &QPushButton::clicked, this, &ControlPanel::playPauseRequested);
    root->addWidget(m_playButton);

    auto* actionRow = new QHBoxLayout();
    actionRow->setSpacing(8);
    m_stepButton = new QPushButton(tr("单步"), this);
    m_stepButton->setIcon(QIcon(QStringLiteral(":/icons/step-forward.svg")));
    m_stepButton->setToolTip(tr("只演化一代（F6）"));
    connect(m_stepButton, &QPushButton::clicked, this, &ControlPanel::stepRequested);

    m_clearButton = new QPushButton(tr("清空"), this);
    m_clearButton->setIcon(QIcon(QStringLiteral(":/icons/trash-2.svg")));
    m_clearButton->setToolTip(tr("清空棋盘并停止模拟"));
    connect(m_clearButton, &QPushButton::clicked, this, &ControlPanel::clearRequested);

    actionRow->addWidget(m_stepButton);
    actionRow->addWidget(m_clearButton);
    root->addLayout(actionRow);

    auto* randomRow = new QHBoxLayout();
    randomRow->setSpacing(8);
    m_randomButton = new QPushButton(tr("随机填充"), this);
    m_randomButton->setIcon(QIcon(QStringLiteral(":/icons/dices.svg")));
    m_randomButton->setToolTip(tr("按右侧密度随机撒点"));
    m_densitySpin = new QSpinBox(this);
    m_densitySpin->setRange(model::limits::kDensityMin, model::limits::kDensityMax);
    m_densitySpin->setValue(model::AppSettings{}.density);
    m_densitySpin->setSuffix(tr(" %"));
    m_densitySpin->setFixedWidth(72);
    m_densitySpin->setToolTip(tr("随机填充的存活概率"));
    connect(m_randomButton, &QPushButton::clicked, this, [this] {
        emit randomizeRequested(m_densitySpin->value());
    });
    randomRow->addWidget(m_randomButton, 1);
    randomRow->addWidget(m_densitySpin);
    root->addLayout(randomRow);

    auto* speedRow = new QHBoxLayout();
    speedRow->setSpacing(8);
    auto* speedTitle = new QLabel(tr("速度"), this);
    speedTitle->setObjectName(QStringLiteral("Muted"));
    m_speedSlider = new QSlider(Qt::Horizontal, this);
    m_speedSlider->setRange(0, kSpeedSliderSteps);
    m_speedSlider->setSingleStep(1);
    m_speedSlider->setPageStep(10);
    m_speedSlider->setValue(sliderPositionForSpeed(m_speed));
    m_speedSlider->setToolTip(tr("模拟速度：每秒演化代数（最高 %1）").arg(model::limits::kSpeedMax));
    m_speedValue = new QLabel(this);
    m_speedValue->setObjectName(QStringLiteral("ValueLabel"));
    m_speedValue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_speedValue->setMinimumWidth(76);
    connect(m_speedSlider, &QSlider::valueChanged, this, [this](int position) {
        const int speed = speedForSliderPosition(position);
        m_speedValue->setText(tr("%1 代/秒").arg(speed));
        if (speed != m_speed)
        {
            m_speed = speed;
            emit speedChanged(speed);
        }
    });
    speedRow->addWidget(speedTitle);
    speedRow->addWidget(m_speedSlider, 1);
    speedRow->addWidget(m_speedValue);
    root->addLayout(speedRow);
    setSpeed(m_speed);

    auto* jumpRow = new QHBoxLayout();
    jumpRow->setSpacing(6);
    auto* jumpTitle = new QLabel(tr("跳转"), this);
    jumpTitle->setObjectName(QStringLiteral("Muted"));

    m_jumpSpin = new QSpinBox(this);
    m_jumpSpin->setObjectName(QStringLiteral("JumpSpin"));
    m_jumpSpin->setRange(m_jumpStep, 10000);
    m_jumpSpin->setSingleStep(m_jumpStep);
    m_jumpSpin->setValue(m_jumpStep);
    m_jumpSpin->setSuffix(tr(" 代"));
    m_jumpSpin->setFixedWidth(90);
    m_jumpSpin->setKeyboardTracking(false);
    m_jumpSpin->setToolTip(tr("一次回退或快进的代数"));
    connect(m_jumpSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int) {
        updateJumpState();
    });

    m_rewindButton = new QPushButton(QIcon(QStringLiteral(":/icons/rewind.svg")), tr("回退"), this);
    m_rewindButton->setMinimumWidth(0);
    connect(m_rewindButton, &QPushButton::clicked, this, [this] {
        emit rewindRequested(m_jumpSpin->value());
    });

    m_forwardButton = new QPushButton(QIcon(QStringLiteral(":/icons/fast-forward.svg")), tr("快进"), this);
    m_forwardButton->setMinimumWidth(0);
    connect(m_forwardButton, &QPushButton::clicked, this, [this] {
        emit fastForwardRequested(m_jumpSpin->value());
    });

    jumpRow->addWidget(jumpTitle);
    jumpRow->addWidget(m_jumpSpin);
    jumpRow->addWidget(m_rewindButton);
    jumpRow->addWidget(m_forwardButton);
    root->addLayout(jumpRow);
    updateJumpState();

    auto* patternGroup = new QGroupBox(tr("图案"), this);
    auto* patternLayout = new QVBoxLayout(patternGroup);
    patternLayout->setContentsMargins(12, 10, 12, 12);
    patternLayout->setSpacing(6);
    m_patternCombo = new QComboBox(patternGroup);
    m_patternCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_patternCombo->setMinimumContentsLength(8);
    connect(m_patternCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        emit patternChanged(m_patternCombo->currentData().toInt());
    });
    patternLayout->addWidget(m_patternCombo);

    m_patternText = new QLabel(patternGroup);
    m_patternText->setObjectName(QStringLiteral("Muted"));
    m_patternText->setWordWrap(true);
    m_patternText->setMinimumHeight(40);
    m_patternText->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    patternLayout->addWidget(m_patternText);
    root->addWidget(patternGroup);

    auto* ruleGroup = new QGroupBox(tr("规则与边界"), this);
    auto* ruleLayout = new QHBoxLayout(ruleGroup);
    ruleLayout->setContentsMargins(12, 10, 12, 12);
    ruleLayout->setSpacing(10);

    auto* ruleColumn = new QVBoxLayout();
    ruleColumn->setSpacing(3);
    auto* ruleLabel = new QLabel(tr("规则"), ruleGroup);
    ruleLabel->setObjectName(QStringLiteral("Muted"));
    m_ruleCombo = new QComboBox(ruleGroup);
    m_ruleCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_ruleCombo->setMinimumContentsLength(4);
    connect(m_ruleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (!m_syncing)
        {
            emit ruleChanged(index);
        }
    });
    ruleColumn->addWidget(ruleLabel);
    ruleColumn->addWidget(m_ruleCombo);

    auto* boundaryColumn = new QVBoxLayout();
    boundaryColumn->setSpacing(3);
    auto* boundaryLabel = new QLabel(tr("边界"), ruleGroup);
    boundaryLabel->setObjectName(QStringLiteral("Muted"));
    m_boundaryCombo = new QComboBox(ruleGroup);
    m_boundaryCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_boundaryCombo->setMinimumContentsLength(4);
    m_boundaryCombo->setToolTip(tr("死亡边界：越界视为空；环绕边界：上下左右相连"));
    connect(m_boundaryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (!m_syncing)
        {
            emit boundaryChanged(index);
        }
    });
    boundaryColumn->addWidget(boundaryLabel);
    boundaryColumn->addWidget(m_boundaryCombo);

    ruleLayout->addLayout(ruleColumn, 1);
    ruleLayout->addLayout(boundaryColumn, 1);
    root->addWidget(ruleGroup);

    auto* sizeGroup = new QGroupBox(tr("棋盘尺寸"), this);
    auto* sizeLayout = new QHBoxLayout(sizeGroup);
    sizeLayout->setContentsMargins(12, 10, 12, 12);
    sizeLayout->setSpacing(6);

    m_widthSpin = new QSpinBox(sizeGroup);
    m_widthSpin->setObjectName(QStringLiteral("WidthSpin"));
    m_widthSpin->setRange(domain::kMinBoardExtent, domain::kMaxBoardExtent);
    m_widthSpin->setSingleStep(5);
    m_widthSpin->setValue(domain::kDefaultBoardWidth);
    m_widthSpin->setFixedWidth(62);
    m_widthSpin->setKeyboardTracking(false);
    m_widthSpin->setToolTip(tr("列数（20 - 400）"));

    m_heightSpin = new QSpinBox(sizeGroup);
    m_heightSpin->setObjectName(QStringLiteral("HeightSpin"));
    m_heightSpin->setRange(domain::kMinBoardExtent, domain::kMaxBoardExtent);
    m_heightSpin->setSingleStep(5);
    m_heightSpin->setValue(domain::kDefaultBoardHeight);
    m_heightSpin->setFixedWidth(62);
    m_heightSpin->setKeyboardTracking(false);
    m_heightSpin->setToolTip(tr("行数（20 - 400）"));

    auto* times = new QLabel(QStringLiteral("×"), sizeGroup);
    times->setObjectName(QStringLiteral("Muted"));

    auto* applyButton = new QPushButton(tr("应用"), sizeGroup);
    applyButton->setFixedWidth(52);
    applyButton->setToolTip(tr("按新尺寸重建空棋盘"));
    connect(applyButton, &QPushButton::clicked, this, [this] {
        emit resizeRequested(m_widthSpin->value(), m_heightSpin->value());
    });

    sizeLayout->addWidget(m_widthSpin, 1);
    sizeLayout->addWidget(times);
    sizeLayout->addWidget(m_heightSpin, 1);
    sizeLayout->addWidget(applyButton);
    root->addWidget(sizeGroup);

    m_gridCheck = new QCheckBox(tr("显示网格线"), this);
    m_gridCheck->setChecked(true);
    connect(m_gridCheck, &QCheckBox::toggled, this, &ControlPanel::gridVisibilityChanged);
    root->addWidget(m_gridCheck);

    root->addStretch(1);

    onRunningChanged(false);
}

void ControlPanel::setModel(const model::LifeModel* model)
{
    if (m_model == model)
    {
        return;
    }

    if (m_model != nullptr)
    {
        disconnect(m_model, nullptr, this, nullptr);
    }

    m_model = model;
    if (m_model != nullptr)
    {
        connect(m_model, &model::LifeModel::statusChanged, this, &ControlPanel::onStatusChanged);
        connect(m_model, &model::LifeModel::runningChanged, this, &ControlPanel::onRunningChanged);
        connect(m_model, &model::LifeModel::boardResized, this, &ControlPanel::onBoardResized);
        connect(m_model, &model::LifeModel::configurationChanged, this, &ControlPanel::onConfigurationChanged);

        onStatusChanged(m_model->stats());
        onRunningChanged(m_model->isRunning());
        onBoardResized(m_model->width(), m_model->height());
        syncRuleSelection();
        syncBoundarySelection();
    }
}

void ControlPanel::setRuleCatalog(const RuleCatalog* catalog)
{
    m_ruleCatalog = catalog;
    syncRuleSelection();
}

void ControlPanel::setPatternOptions(const QVector<PatternOption>& options)
{
    const QSignalBlocker blocker(m_patternCombo);
    m_patternCombo->clear();
    for (const PatternOption& option : options)
    {
        m_patternCombo->addItem(option.text, option.id);
    }

    const int index = m_patternCombo->findData(-1);
    m_patternCombo->setCurrentIndex(index >= 0 ? index : 0);
}

void ControlPanel::setPatternDescription(const QString& text)
{
    m_patternText->setText(text);
}

void ControlPanel::setSpeed(int speed)
{
    const int value = std::clamp(speed, model::limits::kSpeedMin, model::limits::kSpeedMax);
    m_speed = value;
    {
        const QSignalBlocker blocker(m_speedSlider);
        m_speedSlider->setValue(sliderPositionForSpeed(value));
    }
    updateSpeedLabel();
}

int ControlPanel::speed() const
{
    return m_speed;
}

int ControlPanel::randomDensity() const
{
    return m_densitySpin->value();
}

void ControlPanel::setRandomDensity(int density)
{
    const QSignalBlocker blocker(m_densitySpin);
    m_densitySpin->setValue(std::clamp(density, model::limits::kDensityMin, model::limits::kDensityMax));
}

bool ControlPanel::isGridVisible() const
{
    return m_gridCheck->isChecked();
}

void ControlPanel::setGridVisible(bool visible)
{
    const QSignalBlocker blocker(m_gridCheck);
    m_gridCheck->setChecked(visible);
}

void ControlPanel::setJumpStep(int step)
{
    m_jumpStep = std::max(1, step);
    m_jumpSpin->setMinimum(m_jumpStep);
    m_jumpSpin->setSingleStep(m_jumpStep);

    const int rounded = ((m_jumpSpin->value() + m_jumpStep / 2) / m_jumpStep) * m_jumpStep;
    const int aligned = std::max(m_jumpStep, std::min(m_jumpSpin->maximum(), rounded));
    {
        const QSignalBlocker blocker(m_jumpSpin);
        m_jumpSpin->setValue(aligned);
    }
    updateJumpState();
}

int ControlPanel::jumpGenerations() const
{
    return m_jumpSpin->value();
}

void ControlPanel::setSizeStep(int step)
{
    const int value = std::max(1, step);
    m_widthSpin->setSingleStep(value);
    m_heightSpin->setSingleStep(value);
}

void ControlPanel::onStatusChanged(const model::LifeStats& stats)
{
    m_currentGeneration = std::max(0, stats.generation);
    m_generationValue->setText(QLocale().toString(m_currentGeneration));
    m_populationValue->setText(QLocale().toString(std::max(0, stats.population)));
    m_densityValue->setText(QStringLiteral("%1%").arg(QString::number(std::max(0.0, stats.density), 'f', 1)));
    if (m_model != nullptr)
    {
        m_oldestGeneration = m_model->oldestGeneration();
    }
    updateJumpState();
}

void ControlPanel::onRunningChanged(bool running)
{
    m_playButton->setText(running ? tr("暂停") : tr("开始"));
    m_playButton->setIcon(QIcon(running ? QStringLiteral(":/icons/pause.svg")
                                        : QStringLiteral(":/icons/play.svg")));
    m_playButton->setToolTip(running ? tr("暂停模拟（空格）") : tr("开始模拟（空格）"));
}

void ControlPanel::onBoardResized(int width, int height)
{
    const QSignalBlocker widthBlocker(m_widthSpin);
    const QSignalBlocker heightBlocker(m_heightSpin);
    m_widthSpin->setValue(std::clamp(width, m_widthSpin->minimum(), m_widthSpin->maximum()));
    m_heightSpin->setValue(std::clamp(height, m_heightSpin->minimum(), m_heightSpin->maximum()));
}

void ControlPanel::onConfigurationChanged()
{
    syncRuleSelection();
    syncBoundarySelection();
}

void ControlPanel::syncRuleSelection()
{
    if (m_model == nullptr || m_ruleCatalog == nullptr)
    {
        return;
    }

    QStringList names = m_ruleCatalog->names();
    int index = m_ruleCatalog->indexOf(m_model->rule());
    if (index < 0)
    {
        names.append(tr("自定义 · %1").arg(QString::fromStdString(m_model->rule().notation())));
        index = names.size() - 1;
    }

    m_syncing = true;
    {
        const QSignalBlocker blocker(m_ruleCombo);
        m_ruleCombo->clear();
        m_ruleCombo->addItems(names);
        m_ruleCombo->setCurrentIndex(index);
    }
    m_syncing = false;
}

void ControlPanel::syncBoundarySelection()
{
    if (m_model == nullptr)
    {
        return;
    }

    m_syncing = true;
    {
        const QSignalBlocker blocker(m_boundaryCombo);
        m_boundaryCombo->clear();
        m_boundaryCombo->addItems({tr("死亡边界"), tr("环绕边界")});
        m_boundaryCombo->setCurrentIndex(m_model->boundary() == domain::Boundary::Wrap ? 1 : 0);
    }
    m_syncing = false;
}

void ControlPanel::updateJumpState()
{
    if (m_rewindButton == nullptr || m_forwardButton == nullptr || m_jumpSpin == nullptr)
    {
        return;
    }

    const int generations = m_jumpSpin->value();
    const bool canRewind = m_currentGeneration > m_oldestGeneration;
    m_rewindButton->setEnabled(canRewind);
    if (canRewind)
    {
        m_rewindButton->setToolTip(tr("回退 %1 代（最早到第 %2 代）")
                                       .arg(QLocale().toString(generations))
                                       .arg(QLocale().toString(m_oldestGeneration)));
    }
    else
    {
        m_rewindButton->setToolTip(
            tr("已是可回退的最早一代（第 %1 代）").arg(QLocale().toString(m_oldestGeneration)));
    }
    m_forwardButton->setToolTip(tr("快进 %1 代（不逐代刷新界面）").arg(QLocale().toString(generations)));
}

void ControlPanel::updateSpeedLabel()
{
    m_speedValue->setText(tr("%1 代/秒").arg(m_speed));
}

} // namespace lifegame::view
