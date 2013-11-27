#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDebug>

#include <CutelystApplication>

#include "root.h"
#include "users.h"

#include "Plugins/cstaticsimple.h"
#include "Plugins/csession.h"

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("Cecilet Ti");
    QCoreApplication::setOrganizationDomain("ceciletti.com.br");
    QCoreApplication::setApplicationName("CuteWeb");
    QCoreApplication::setApplicationVersion("0.0.1");

    qRegisterMetaType<Root*>();
    qRegisterMetaType<Users*>();

    CutelystApplication app(argc, argv);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    QCoreApplication::installTranslator(&qtTranslator);

    app.registerPlugin(new CPStaticSimple);
    app.registerPlugin(new CutelystPluginSession);

    if (app.parseArgs() && app.setup()) {
        return app.exec();
    }
    return app.printError();
}
