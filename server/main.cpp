#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDebug>

#include <CutelystApplication>

#include "root.h"
#include "users.h"

#include "Plugin/staticsimple.h"
#include "Plugin/session.h"

using namespace std;

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

    app.registerPlugin(new CutelystPlugin::StaticSimple);
    app.registerPlugin(new CutelystPlugin::Session);

    if (app.parseArgs() && app.setup()) {
        return app.exec();
    }
    return app.printError();
}
