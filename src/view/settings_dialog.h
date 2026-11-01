#pragma once

/**
 * @file settings_dialog.h
 * @brief Settings value editor view.
 */

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

    /// @param values Current settings to display; unedited fields are retained.
    void setValues(const shared::AppSettings& values);

    /// @return Edited settings merged with retained fields.
    shared::AppSettings values() const;

private:
    QSpinBox* m_jumpStepSpin = nullptr;
    QSpinBox* m_sizeStepSpin = nullptr;
    QSpinBox* m_snapshotSpin = nullptr;
    shared::AppSettings m_base;
};

} // namespace lifegame::view
