#pragma once

#include <QDialog>

#include "model/app_settings.h"

class QSpinBox;

namespace lifegame::view
{

/// Pure view: reads/writes AppSettings, owns no persistence and no model.
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);

    void setValues(const model::AppSettings& values);
    model::AppSettings values() const;

private:
    QSpinBox* m_jumpStepSpin = nullptr;
    QSpinBox* m_sizeStepSpin = nullptr;
    QSpinBox* m_snapshotSpin = nullptr;
    model::AppSettings m_base;
};

} // namespace lifegame::view
