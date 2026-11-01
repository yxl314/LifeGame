#pragma once

/**
 * @file jump_progress_dialog.h
 * @brief QProgressDialog implementation of the jump progress port.
 */

#include <QPointer>
#include <QWidget>

#include <functional>
#include <memory>

#include "view/interfaces.h"

class QProgressDialog;

namespace lifegame::view
{

/// QProgressDialog-backed implementation of JumpProgressView.
class JumpProgressDialog : public JumpProgressView
{
public:
    /// @param parent Widget parent.
    explicit JumpProgressDialog(QWidget* parent);
    ~JumpProgressDialog() override;

    /// @param info Jump parameters.
    void show(const shared::JumpInfo& info) override;
    /// @param done Replayed generations.
    void setProgress(int done) override;
    /// @brief Programmatically close without treating it as user cancel.
    void close() override;
    /// @param handler Callback invoked on user cancellation.
    void setCancelHandler(std::function<void()> handler) override;

private:
    void closeDialog();

    QWidget* m_parent = nullptr;
    QPointer<QProgressDialog> m_dialog;
    std::function<void()> m_cancelHandler;
};

} // namespace lifegame::view
