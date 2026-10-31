#pragma once

#include <QObject>
#include <QString>

#include "domain/rle.h"
#include "model/command_result.h"
#include "service/file_error.h"

class QWidget;

namespace lifegame::model
{
class LifeModel;
}

namespace lifegame::service
{
class RleFileService;
class ImageFileService;
}

namespace lifegame::view
{
class BoardView;
class MessageSink;
}

namespace lifegame::controller
{

/// Open/save/export input and file dialogs.
class FileController : public QObject
{
    Q_OBJECT

public:
    struct OpenResult
    {
        service::FileError fileError = service::FileError::None;
        domain::RleErrorInfo decodeError;
        model::CommandResult modelResult = model::CommandResult::Success;
        QString detail;
    };

    FileController(model::LifeModel* model,
                   view::BoardView* view,
                   view::MessageSink* messages,
                   QWidget* dialogParent,
                   service::RleFileService* rleService,
                   service::ImageFileService* imageService,
                   QObject* parent = nullptr);

    /// Path-based operations used by the dialogs and by tests.
    OpenResult openPath(const QString& path);
    service::FileError savePath(const QString& path, QString* detail = nullptr);
    service::FileError exportPath(const QString& path, QString* detail = nullptr);

public slots:
    void openRle();
    void saveRle();
    void exportImage();

private:
    QString rleErrorMessage(const domain::RleErrorInfo& error) const;

    model::LifeModel* m_model = nullptr;
    view::BoardView* m_view = nullptr;
    view::MessageSink* m_messages = nullptr;
    QWidget* m_dialogParent = nullptr;
    service::RleFileService* m_rle = nullptr;
    service::ImageFileService* m_image = nullptr;
    QString m_lastDir;
};

} // namespace lifegame::controller
