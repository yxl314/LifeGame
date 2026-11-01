#include "controller/session_service.h"

#include "domain/rle.h"
#include "model/life_model.h"
#include "service/session_state.h"
#include "service/settings_store.h"

namespace lifegame::controller
{

SessionService::SessionService(model::LifeModel* model, service::SettingsStore* store)
    : m_model(model), m_store(store)
{
}

SessionRestoreResult SessionService::restore()
{
    SessionRestoreResult result;
    const service::SettingsLoadResult loadedState = m_store->load();
    const service::SessionState state = loadedState.state;
    result.settings = shared::normalized(state.settings);
    result.error = loadedState.error;
    result.detail = loadedState.detail;
    result.windowGeometry = state.windowGeometry;

    m_model->setSnapshotInterval(result.settings.snapshotInterval);
    m_model->setBoundary(state.boundary == 1 ? domain::Boundary::Wrap : domain::Boundary::Dead);
    m_model->setRuleByNotation(state.ruleNotation.toStdString());

    result.hadStoredBoard = !state.boardRle.isEmpty();
    if (result.hadStoredBoard)
    {
        const domain::RleDecodeResult decoded = domain::decodeRle(state.boardRle.toStdString());
        if (decoded.document.has_value())
        {
            result.loadResult = m_model->loadDocument(*decoded.document);
            if (model::succeeded(result.loadResult))
            {
                result.restored = true;
                if (state.generation > 0)
                {
                    (void)m_model->setGeneration(state.generation);
                }
            }
            else
            {
                result.loadFailed = true;
            }
        }
        else
        {
            result.decodeFailed = true;
        }
    }

    if (!result.restored)
    {
        (void)m_model->resizeBoard(domain::kDefaultBoardWidth, domain::kDefaultBoardHeight);
    }
    return result;
}

service::SettingsError SessionService::save(const shared::AppSettings& settings,
                                             const QByteArray& windowGeometry,
                                             QString* detail)
{
    service::SessionState state;
    state.settings = settings;
    state.boardRle = QString::fromStdString(m_model->toRle("life-game"));
    state.ruleNotation = QString::fromStdString(m_model->rule().notation());
    state.boundary = m_model->boundary() == domain::Boundary::Wrap ? 1 : 0;
    state.generation = m_model->stats().generation;
    state.windowGeometry = windowGeometry;
    return m_store->save(state, detail);
}

} // namespace lifegame::controller
