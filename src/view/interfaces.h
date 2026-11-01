#pragma once

/**
 * @file interfaces.h
 * @brief Narrow view output ports for controllers.
 */

#include <QString>

#include <functional>
#include <memory>

#include "shared/jump_info.h"

namespace lifegame::view
{

/// @brief Output port for destructive-action confirmation.
class ConfirmSink
{
public:
    virtual ~ConfirmSink() = default;

    /// @param title Dialog title.
    /// @param text Confirmation body.
    /// @return True when the user accepted.
    virtual bool confirm(const QString& title, const QString& text) = 0;
};

/// @brief Output port for transient status text.
class MessageSink
{
public:
    virtual ~MessageSink() = default;

    /// @param text Message to show.
    /// @param timeoutMs Auto-hide timeout in milliseconds; 0 = sticky.
    virtual void showMessage(const QString& text, int timeoutMs) = 0;
};

/// @brief Output port for jump progress presentation.
class JumpProgressView
{
public:
    virtual ~JumpProgressView() = default;

    /// @param info Jump parameters.
    virtual void show(const shared::JumpInfo& info) = 0;

    /// @param done Replayed generations.
    virtual void setProgress(int done) = 0;

    /// @brief Close the progress view.
    virtual void close() = 0;

    /// @param handler Callback invoked on user cancellation.
    virtual void setCancelHandler(std::function<void()> handler) = 0;
};

/// @brief Factory used to create a jump progress view.
using JumpProgressFactory = std::function<std::unique_ptr<JumpProgressView>()>;

} // namespace lifegame::view
