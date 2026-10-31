#pragma once

#include <QObject>
#include <QString>

class QActionGroup;

namespace lifegame::model
{
class LifeModel;
}

namespace lifegame::view
{
class ControlPanel;
class MessageSink;
class RuleCatalog;
}

namespace lifegame::controller
{

/// Rule/boundary input from the panel or the menu.
class ConfigurationController : public QObject
{
    Q_OBJECT

public:
    ConfigurationController(model::LifeModel* model,
                            view::ControlPanel* panel,
                            const view::RuleCatalog* catalog,
                            view::MessageSink* messages,
                            QObject* parent = nullptr);

public slots:
    void setRuleIndex(int index);
    void setRuleByNotation(const QString& notation);
    void setBoundaryIndex(int index);

    void setRuleActionGroup(QActionGroup* group);

private:
    void syncRuleActions();
    void syncPanel();

    model::LifeModel* m_model = nullptr;
    view::ControlPanel* m_panel = nullptr;
    const view::RuleCatalog* m_catalog = nullptr;
    view::MessageSink* m_messages = nullptr;
    QActionGroup* m_ruleGroup = nullptr;
};

} // namespace lifegame::controller
