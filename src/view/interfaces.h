#pragma once

#include <QString>

#include <functional>
#include <memory>

#include "shared/jump_info.h"

namespace lifegame::view
{

/// Narrow interface for destructive-action confirmation, so BoardController is
/// testable without a modal QMessageBox.
class ConfirmSink
{
public:
    virtual ~ConfirmSink() = default;
    virtual bool confirm(const QString& title, const QString& text) = 0;
};

/// Narrow interface used by controllers to report transient status text.
class MessageSink
{
public:
    virtual ~MessageSink() = default;
    virtual void showMessage(const QString& text, int timeoutMs) = 0;
};

/// Narrow interface for the jump progress presentation, so JumpController can
/// be unit-tested with a fake.
class JumpProgressView
{
public:
    virtual ~JumpProgressView() = default;

    virtual void show(const shared::JumpInfo& info) = 0;
    virtual void setProgress(int done) = 0;
    virtual void close() = 0;
    virtual void setCancelHandler(std::function<void()> handler) = 0;
};

using JumpProgressFactory = std::function<std::unique_ptr<JumpProgressView>()>;

} // namespace lifegame::view
