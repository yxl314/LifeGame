#include "controller/configuration_controller.h"

#include <QAction>
#include <QActionGroup>

#include "domain/boundary.h"
#include "domain/rule.h"
#include "model/life_model.h"
#include "view/control_panel.h"
#include "view/interfaces.h"
#include "view/rule_catalog.h"

#include <QSignalBlocker>

namespace lifegame::controller
{

ConfigurationController::ConfigurationController(model::LifeModel* model,
                                                 view::ControlPanel* panel,
                                                 const view::RuleCatalog* catalog,
                                                 view::MessageSink* messages,
                                                 QObject* parent)
    : QObject(parent), m_model(model), m_catalog(catalog), m_messages(messages)
{
    connect(panel, &view::ControlPanel::ruleChanged, this, &ConfigurationController::setRuleIndex);
    connect(panel, &view::ControlPanel::boundaryChanged, this, &ConfigurationController::setBoundaryIndex);
}

void ConfigurationController::setRuleIndex(int index)
{
    if (m_catalog == nullptr)
    {
        return;
    }
    const QString notation = m_catalog->notationAt(index);
    if (notation.isEmpty())
    {
        return; // custom entry, not selectable as a preset
    }
    setRuleByNotation(notation);
}

void ConfigurationController::setRuleByNotation(const QString& notation)
{
    if (m_model->isJumping())
    {
        if (m_messages != nullptr)
        {
            m_messages->showMessage(tr("跳转进行中，暂不能切换规则。"), 3000);
        }
        return;
    }

    const std::string target = notation.toStdString();
    if (!domain::Rule::fromNotation(target).has_value())
    {
        if (m_messages != nullptr)
        {
            m_messages->showMessage(tr("规则记号无效：%1").arg(notation), 4000);
        }
        return;
    }
    if (m_model->rule().notation() == target)
    {
        return;
    }

    if (m_model->setRuleByNotation(target) && m_messages != nullptr)
    {
        m_messages->showMessage(
            tr("规则已切换为 %1（%2）。").arg(m_catalog->nameForNotation(notation), notation), 4000);
    }
}

void ConfigurationController::setRuleActionGroup(QActionGroup* group)
{
    if (m_ruleGroup != group && group != nullptr)
    {
        disconnect(m_model, &model::LifeModel::configurationChanged, this, &ConfigurationController::syncRuleActions);
        connect(m_model, &model::LifeModel::configurationChanged, this, &ConfigurationController::syncRuleActions);
    }
    m_ruleGroup = group;
    syncRuleActions();
}

void ConfigurationController::syncRuleActions()
{
    if (m_ruleGroup == nullptr)
    {
        return;
    }

    const QString notation = QString::fromStdString(m_model->rule().notation());
    for (QAction* action : m_ruleGroup->actions())
    {
        const bool checked = action->data().toString() == notation;
        const QSignalBlocker blocker(action);
        action->setChecked(checked);
    }
}

void ConfigurationController::setBoundaryIndex(int index)
{
    if (index != 0 && index != 1)
    {
        return;
    }
    const auto boundary = index == 1 ? domain::Boundary::Wrap : domain::Boundary::Dead;
    if (m_model->boundary() == boundary)
    {
        return;
    }
    if (m_model->setBoundary(boundary) && m_messages != nullptr)
    {
        m_messages->showMessage(boundary == domain::Boundary::Wrap ? tr("已启用环绕边界。")
                                                                  : tr("已启用死亡边界。"),
                                3000);
    }
}

} // namespace lifegame::controller
