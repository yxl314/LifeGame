#pragma once

#include <QMetaType>

namespace lifegame::model
{

/// Parameters of a rewind/fast-forward operation.
struct JumpInfo
{
    int fromGeneration = 0;
    int toGeneration = 0;
    int replayGenerations = 0;
};

} // namespace lifegame::model

Q_DECLARE_METATYPE(lifegame::model::JumpInfo)
