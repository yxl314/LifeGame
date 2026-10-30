#include "view/settings_dialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace lifegame::view
{

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("设置"));
    setModal(true);
    setMinimumWidth(430);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(18, 16, 18, 14);
    root->setSpacing(12);

    auto* form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(10);

    m_jumpStepSpin = new QSpinBox(this);
    m_jumpStepSpin->setObjectName(QStringLiteral("JumpStepSpin"));
    m_jumpStepSpin->setRange(model::limits::kJumpStepMin, model::limits::kJumpStepMax);
    m_jumpStepSpin->setSingleStep(100);
    m_jumpStepSpin->setSuffix(tr(" 代"));
    m_jumpStepSpin->setFixedWidth(120);
    m_jumpStepSpin->setAccessibleName(tr("跳转粒度"));
    m_jumpStepSpin->setToolTip(tr("「回退 / 快进」每次调整的代数，也是最小跳转代数"));
    form->addRow(tr("跳转粒度"), m_jumpStepSpin);

    m_sizeStepSpin = new QSpinBox(this);
    m_sizeStepSpin->setObjectName(QStringLiteral("SizeStepSpin"));
    m_sizeStepSpin->setRange(model::limits::kSizeStepMin, model::limits::kSizeStepMax);
    m_sizeStepSpin->setSingleStep(5);
    m_sizeStepSpin->setSuffix(tr(" 格"));
    m_sizeStepSpin->setFixedWidth(120);
    m_sizeStepSpin->setAccessibleName(tr("尺寸粒度"));
    m_sizeStepSpin->setToolTip(tr("棋盘宽高输入框的步进"));
    form->addRow(tr("尺寸粒度"), m_sizeStepSpin);

    m_snapshotSpin = new QSpinBox(this);
    m_snapshotSpin->setObjectName(QStringLiteral("SnapshotSpin"));
    m_snapshotSpin->setRange(model::limits::kSnapshotIntervalMin, model::limits::kSnapshotIntervalMax);
    m_snapshotSpin->setSingleStep(50);
    m_snapshotSpin->setSuffix(tr(" 代"));
    m_snapshotSpin->setFixedWidth(120);
    m_snapshotSpin->setAccessibleName(tr("快照间隔"));
    m_snapshotSpin->setToolTip(tr("每隔多少代保存一张回退快照"));
    form->addRow(tr("快照间隔"), m_snapshotSpin);

    root->addLayout(form);

    auto* hint = new QLabel(
        tr("快照间隔越小，回退越快；但可快速回退的代数范围也越小（保留 64 张快照，约 10 MB）。"
           "修改快照间隔会重置当前的回退历史。"),
        this);
    hint->setObjectName(QStringLiteral("Muted"));
    hint->setWordWrap(true);
    root->addWidget(hint);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("保存"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("取消"));
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    root->addWidget(buttons);
}

void SettingsDialog::setValues(const model::AppSettings& values)
{
    m_jumpStepSpin->setValue(values.jumpStep);
    m_sizeStepSpin->setValue(values.sizeStep);
    m_snapshotSpin->setValue(values.snapshotInterval);
}

model::AppSettings SettingsDialog::values() const
{
    model::AppSettings result;
    result.jumpStep = m_jumpStepSpin->value();
    result.sizeStep = m_sizeStepSpin->value();
    result.snapshotInterval = m_snapshotSpin->value();
    return result;
}

} // namespace lifegame::view
