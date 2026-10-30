#include <QApplication>
#include <QCoreApplication>

#include "app/main_window.h"
#include "view/theme.h"

int main(int argc, char* argv[])
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication application(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("life-game"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("life-game.local"));
    QCoreApplication::setApplicationName(QStringLiteral("life-game"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0.0"));
    QApplication::setApplicationDisplayName(QStringLiteral("生命游戏"));

    lifegame::view::theme::apply(application);

    lifegame::app::MainWindow window;
    window.show();

    return application.exec();
}
