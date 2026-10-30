#pragma once

#include <QObject>
#include <QString>

#include "service/image_file_service.h"
#include "service/rle_file_service.h"

class QWidget;

namespace lifegame::model
{
class LifeModel;
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
    FileController(model::LifeModel* model,
                   view::BoardView* view,
                   view::MessageSink* messages,
                   QWidget* dialogParent,
                   QObject* parent = nullptr);

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
    service::RleFileService m_rle;
    service::ImageFileService m_image;
    QString m_lastDir;
};

} // namespace lifegame::controller
