#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

#include <string>
#include <string_view>
#include <vector>

#include "domain/rule.h"

namespace lifegame::view
{

struct RulePresetView
{
    std::string id;
    QString name;
    QString notation;
};

/// Presentation catalog: maps domain rule IDs/notations to localized names.
class RuleCatalog : public QObject
{
    Q_OBJECT

public:
    explicit RuleCatalog(QObject* parent = nullptr);

    const std::vector<RulePresetView>& presets() const;
    QStringList names() const;
    int indexOf(const domain::Rule& rule) const;
    QString notationAt(int index) const;
    QString nameForNotation(const QString& notation) const;

private:
    std::vector<RulePresetView> m_presets;
};

} // namespace lifegame::view
