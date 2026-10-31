#pragma once

#include <QString>

#include "model/command_result.h"
#include "service/file_error.h"
#include "service/settings_error.h"

namespace lifegame::controller
{

/// One actionable Chinese message per model command result.
QString commandResultText(model::CommandResult result);
QString fileErrorText(service::FileError error, const QString& detail = {});
QString settingsErrorText(service::SettingsError error, const QString& detail = {});

} // namespace lifegame::controller
