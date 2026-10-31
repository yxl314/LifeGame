#pragma once

#include <QDialog>

#include "shared/app_settings.h"

class QSpinBox;

namespace lifegame::view
{

/// Pure view: reads/writes AppSettings, owns no persistence and no model.
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);

    void setValues(const shared::AppSettings& values);
    shared::AppSettings values() const;

private:
    QSpinBox* m_jumpStepSpin = nullptr;
    QSpinBox* m_sizeStepSpin = nullptr;
    QSpinBox* m_snapshotSpin = nullptr;
    shared::AppSettings m_base;
};

} // namespace lifegame::view
