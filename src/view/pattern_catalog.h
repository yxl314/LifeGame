#pragma once

/**
 * @file pattern_catalog.h
 * @brief Pattern ID to display-name presentation catalog.
 */

#include <QObject>
#include <QString>

#include <string>
#include <string_view>
#include <vector>

#include "domain/pattern.h"

namespace lifegame::view
{

/// @brief Display metadata for one pattern.
struct PatternInfo
{
    std::string id;      ///< Stable domain ID.
    QString name;        ///< Localized name.
    QString category;    ///< Localized category.
    QString description; ///< Localized description.
};

/// @brief Presentation catalog: maps pattern IDs to localized text.
class PatternCatalog : public QObject
{
    Q_OBJECT

public:
    explicit PatternCatalog(QObject* parent = nullptr);

    /// @return All pattern metadata.
    const std::vector<PatternInfo>& all() const;

    /// @param id Stable pattern ID.
    /// @return Matching metadata, or nullptr.
    const PatternInfo* find(std::string_view id) const;

    /// @param pattern Domain pattern.
    /// @return Localized display label.
    QString displayLabel(const domain::Pattern& pattern) const;

    /// @param id Stable pattern ID.
    /// @return Localized description, or the ID when unknown.
    QString descriptionOf(std::string_view id) const;

private:
    std::vector<PatternInfo> m_patterns;
};

} // namespace lifegame::view
