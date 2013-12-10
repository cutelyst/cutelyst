#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDebug>

#include <Cutelyst/Application>

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

    qRegisterMetaType<Root*>();
    qRegisterMetaType<Users*>();

    Application app(argc, argv);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    QCoreApplication::installTranslator(&qtTranslator);

    QObject::connect(&app, &Application::registerPlugins,
                [=](Context *ctx) {
        ctx->registerPlugin(new CutelystPlugin::StaticSimple);
        ctx->registerPlugin(new CutelystPlugin::Session);
    });

    if (app.parseArgs() && app.setup()) {
        return app.exec();
    }
    return app.printError();
}
