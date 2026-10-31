#pragma once

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
    explicit JumpProgressDialog(QWidget* parent);
    ~JumpProgressDialog() override;

    void show(const model::JumpInfo& info) override;
    void setProgress(int done) override;
    void close() override;
    void setCancelHandler(std::function<void()> handler) override;

private:
    void closeDialog();

    QWidget* m_parent = nullptr;
    QPointer<QProgressDialog> m_dialog;
    std::function<void()> m_cancelHandler;
};

} // namespace lifegame::view
