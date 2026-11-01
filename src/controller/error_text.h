#pragma once

/**
 * @file error_text.h
 * @brief Error-code to localized-message mapping.
 */

#include <QString>

#include "model/command_result.h"
#include "service/file_error.h"
#include "service/settings_error.h"

namespace lifegame::controller
{

/// @param result Model command result.
/// @return One actionable localized message.
QString commandResultText(model::CommandResult result);

/// @param error File error.
/// @param detail Optional system detail.
/// @return One actionable localized message.
QString fileErrorText(service::FileError error, const QString& detail = {});

/// @param error Settings error.
/// @param detail Optional system detail.
/// @return One actionable localized message.
QString settingsErrorText(service::SettingsError error, const QString& detail = {});

} // namespace lifegame::controller
