#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDebug>

#include <Cutelyst/Application>
#include <Cutelyst/enginehttp.h>

#include "root.h"
#include "users.h"

#include <Cutelyst/Plugin/staticsimple.h>
#include <Cutelyst/Plugin/session.h>

using namespace std;

void registerPlugins(Context *ctx)
{

}

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("Cecilet Ti");
    QCoreApplication::setOrganizationDomain("ceciletti.com.br");
    QCoreApplication::setApplicationName("CuteWeb");
    QCoreApplication::setApplicationVersion("0.0.1");

    QCoreApplication app(argc, argv);
    Application *server = new Application;
    server->registerController(new Root);
    server->registerController(new Users);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    QCoreApplication::installTranslator(&qtTranslator);

    QObject::connect(server, &Application::registerPlugins,
                [=](Context *ctx) {
        ctx->registerPlugin(new Plugin::StaticSimple);
        ctx->registerPlugin(new Plugin::Session);
    });

    EngineHttp *engine = new EngineHttp;
    if (engine->setupApplication(server)) {
        return app.exec();
    }
    return 1;
}
