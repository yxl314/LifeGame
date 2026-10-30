#pragma once

#include <QObject>
#include <QString>

#include <string>
#include <string_view>
#include <vector>

#include "domain/pattern.h"

namespace lifegame::view
{

struct PatternInfo
{
    std::string id;
    QString name;
    QString category;
    QString description;
};

/// Presentation catalog: maps domain pattern IDs to names/descriptions.
class PatternCatalog : public QObject
{
    Q_OBJECT

public:
    explicit PatternCatalog(QObject* parent = nullptr);

    const std::vector<PatternInfo>& all() const;
    const PatternInfo* find(std::string_view id) const;
    QString displayLabel(const domain::Pattern& pattern) const;
    QString descriptionOf(std::string_view id) const;

private:
    std::vector<PatternInfo> m_patterns;
};

} // namespace lifegame::view
