/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <Cutelyst/Server/server.h>
#include <iostream>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QLibraryInfo>
#include <QLocale>
#include <QRegularExpression>
#include <QStringBuilder>
#include <QTextStream>
#include <QTranslator>

#ifdef Q_OS_UNIX
#    include <utime.h>
#endif

#include "config.h"
#include "helper.h"

#define OUT_EXISTS "  exists "
#define OUT_CREATED " created "

using namespace Qt::Literals::StringLiterals;

bool buildControllerHeader(const QString &filename, const QString &controllerName, bool helpers);
bool buildControllerImplementation(const QString &filename,
                                   const QString &controllerName,
                                   bool helpers);

bool createController(const QString &controllerName)
{
    const static QRegularExpression nonWordRE(u"\\W"_s);
    const static QRegularExpression nonDigitRE(u"^\\d"_s);
    if (controllerName.contains(nonWordRE) || controllerName.contains(nonDigitRE)) {
        //% "Error: invalid Controller name."
        std::cerr << qUtf8Printable(qtTrId("cutelystcmd-err-inv-cont-name")) << '\n';
        return false;
    }

    QDir projectDir;
    if (!Helper::findProjectDir(QDir::current(), &projectDir)) {
        //% "Error: failed to find project."
        std::cerr << qUtf8Printable(qtTrId("cutelystcmd-err-fail-find-proj")) << '\n';
        return false;
    }

    if (!buildControllerHeader(projectDir.absoluteFilePath(u"src/"_s) % controllerName.toLower() %
                                   QLatin1String(".h"),
                               controllerName,
                               false)) {
        return false;
    }

    if (!buildControllerImplementation(projectDir.absoluteFilePath(u"src/"_s) %
                                           controllerName.toLower() % QLatin1String(".cpp"),
                                       controllerName,
                                       false)) {
        return false;
    }

#ifdef Q_OS_UNIX
    // Change the modification time of CMakeLists.txt to force FILE_GLOB to be updated
    utime(projectDir.absoluteFilePath(u"CMakeLists.txt"_s).toLatin1().data(), nullptr);
#endif

    //% "Now, on your application class include and instantiate the controller."
    std::cout << qUtf8Printable(qtTrId("cutelystcmd-info-instantiate-controller")) << '\n';

    return true;
}

bool buildApplicationImplementation(const QString &filename, const QString &appName)
{
    QFile data(filename);
    if (data.exists()) {
        std::cerr << OUT_EXISTS << qPrintable(filename) << '\n';
        return true;
    }

    if (data.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&data);
        QFileInfo fileInfo(filename);
        out << "#include \"" << fileInfo.baseName() << ".h\"" << "\n";
        out << "\n";
        out << "#include \"root.h\"" << "\n";
        out << "\n";
        out << "using namespace Cutelyst;" << "\n";
        out << "\n";
        out << appName << "::" << appName << "(QObject *parent) : Application(parent)" << "\n";
        out << "{" << "\n";
        out << "}" << "\n";
        out << "\n";
        out << appName << "::~" << appName << "()" << "\n";
        out << "{" << "\n";
        out << "}" << "\n";
        out << "\n";
        out << "bool " << appName << "::init" << "()" << "\n";
        out << "{" << "\n";
        out << "    new Root(this);" << "\n";
        out << "\n";
        out << "    return true;" << "\n";
        out << "}" << "\n";
        out << "\n";

        std::cout << OUT_CREATED << qPrintable(filename) << '\n';

        return true;
    }
    //% "Error: failed to create file:"
    std::cerr << qUtf8Printable(qtTrId("cutelytcmd-err-failed-create-file")) << qPrintable(filename)
              << '\n';

    return false;
}

bool buildApplicationHeader(const QString &filename, const QString &appName)
{
    QFile data(filename);
    if (data.exists()) {
        std::cout << OUT_EXISTS << qPrintable(filename) << '\n';
        return true;
    }

    if (data.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&data);
        out << "#ifndef " << appName.toUpper() << "_H" << "\n";
        out << "#define " << appName.toUpper() << "_H" << "\n";
        out << "\n";
        out << "#include <Cutelyst/Application>" << "\n";
        out << "\n";
        out << "using namespace Cutelyst;" << "\n";
        out << "\n";
        out << "class " << appName << " : public Application" << "\n";
        out << "{" << "\n";
        out << "    Q_OBJECT" << "\n";
        out << "    CUTELYST_APPLICATION(IID \"" << appName << "\")" << "\n";
        out << "public:" << "\n";
        out << "    Q_INVOKABLE explicit " << appName << "(QObject *parent = nullptr);" << "\n";
        out << "    ~" << appName << "();" << "\n";
        out << "\n";
        out << "    bool init();" << "\n";
        out << "};" << "\n";
        out << "\n";
        out << "#endif //" << appName.toUpper() << "_H" << "\n";
        out << "\n";

        std::cout << OUT_CREATED << qPrintable(filename) << '\n';

        return true;
    }

    std::cerr << qUtf8Printable(qtTrId("cutelytcmd-err-failed-create-file")) << qPrintable(filename)
              << '\n';

    return false;
}

bool buildControllerImplementation(const QString &filename,
                                   const QString &controllerName,
                                   bool helpers)
{
    QFile data(filename);
    if (data.exists()) {
        std::cout << OUT_EXISTS << qPrintable(filename) << '\n';
        return true;
    }

    if (data.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&data);
        QFileInfo fileInfo(filename);
        out << "#include \"" << fileInfo.baseName() << ".h\"" << "\n";
        out << "\n";
        out << "using namespace Cutelyst;" << "\n";
        out << "\n";
        out << controllerName << "::" << controllerName << "(QObject *parent) : Controller(parent)"
            << "\n";
        out << "{" << "\n";
        out << "}" << "\n";
        out << "\n";
        out << controllerName << "::~" << controllerName << "()" << "\n";
        out << "{" << "\n";
        out << "}" << "\n";
        out << "\n";
        out << "void " << controllerName << "::index" << "(Context *c)" << "\n";
        out << "{" << "\n";
        if (helpers) {
            out << "    c->response()->body() = \"Welcome to Cutelyst!\";" << "\n";
        } else {
            out << "    c->response()->body() = \"Matched Controller::" << controllerName << " in "
                << controllerName << ".\";" << "\n";
        }
        out << "}" << "\n";
        out << "\n";
        if (helpers) {
            out << "void " << controllerName << "::defaultPage" << "(Context *c)" << "\n";
            out << "{" << "\n";
            out << "    c->response()->body() = \"Page not found!\";" << "\n";
            out << "    c->response()->setStatus(404);" << "\n";
            out << "}" << "\n";
            out << "\n";
        }

        std::cout << OUT_CREATED << qPrintable(filename) << '\n';

        return true;
    }

    std::cerr << qUtf8Printable(qtTrId("cutelytcmd-err-failed-create-file")) << qPrintable(filename)
              << '\n';

    return false;
}

bool buildControllerHeader(const QString &filename, const QString &controllerName, bool helpers)
{
    QFile data(filename);
    if (data.exists()) {
        std::cout << OUT_EXISTS << qPrintable(filename) << '\n';
        return true;
    }

    if (data.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&data);
        out << "#ifndef " << controllerName.toUpper() << "_H" << "\n";
        out << "#define " << controllerName.toUpper() << "_H" << "\n";
        out << "\n";
        out << "#include <Cutelyst/Controller>" << "\n";
        out << "\n";
        out << "using namespace Cutelyst;" << "\n";
        out << "\n";
        out << "class " << controllerName << " : public Controller" << "\n";
        out << "{" << "\n";
        out << "    Q_OBJECT" << "\n";
        if (helpers) {
            out << "    C_NAMESPACE(\"\")" << "\n";
        }
        out << "public:" << "\n";
        out << "    explicit " << controllerName << "(QObject *parent = nullptr);" << "\n";
        out << "    ~" << controllerName << "();" << "\n";
        out << "\n";
        out << "    C_ATTR(index, :Path :AutoArgs)" << "\n";
        out << "    void index(Context *c);" << "\n";
        if (helpers) {
            out << "\n";
            out << "    C_ATTR(defaultPage, :Path)" << "\n";
            out << "    void defaultPage(Context *c);" << "\n";
            out << "\n";
            out << "private:\n";
            out << "    C_ATTR(End, :ActionClass(\"RenderView\"))" << "\n";
            out << "    void End(Context *c) { Q_UNUSED(c); }" << "\n";
        }
        out << "};" << "\n";
        out << "\n";
        out << "#endif //" << controllerName.toUpper() << "_H" << "\n";
        out << "\n";

        std::cout << OUT_CREATED << qPrintable(filename) << '\n';

        return true;
    }

    std::cerr << qUtf8Printable(qtTrId("cutelytcmd-err-failed-create-file")) << qPrintable(filename)
              << '\n';

    return false;
}

bool buildSrcCMakeLists(const QString &name, const QString &appName)
{
    QFile data(name);
    if (data.exists()) {
        std::cout << OUT_EXISTS << qPrintable(name) << '\n';
        return true;
    }

    if (data.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&data);
        out << "file(GLOB_RECURSE " << appName << "_SRCS *.cpp *.h)" << "\n";
        out << "\n";
        out << "set(" << appName << "_SRCS" << "\n";
        out << "    ${" << appName << "_SRCS}" << "\n";
        out << "    ${TEMPLATES_SRC}" << "\n";
        out << ")" << "\n";
        out << "\n";
        out << "# Create the application" << "\n";
        out << "add_library(" << appName << " SHARED ${" << appName << "_SRCS})" << "\n";
        out << "\n";
        out << "# Link to Cutelyst" << "\n";
        out << "target_link_libraries(" << appName << "\n";
        out << "    Cutelyst::Core" << "\n";
        out << "    Qt::Core" << "\n";
        out << "    Qt::Network" << "\n";
        out << ")" << "\n";
        out << "\n";

        std::cout << OUT_CREATED << qPrintable(name) << '\n';

        return true;
    }

    std::cerr << qUtf8Printable(qtTrId("cutelytcmd-err-failed-create-file")) << qPrintable(name)
              << '\n';

    return false;
}

bool buildProjectCMakeLists(const QString &name, const QString &appName)
{
    QFile data(name);
    if (data.exists()) {
        std::cout << OUT_EXISTS << qPrintable(name) << '\n';
        return true;
    }

    if (data.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&data);
        out << "cmake_minimum_required(VERSION 3.16 FATAL_ERROR)" << "\n";
        out << "\n";
        out << "project(" << appName << ")" << "\n";
        out << "\n";
        out << "if(WIN32)\n";
        out << "  if(MSVC)\n";
        out << "    add_definitions(-D_SCL_SECURE_NO_WARNINGS)\n";
        out << "    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)\n";
        out << "  endif()\n";
        out << "endif()\n\n";
        out << "find_package(Qt" << QT_VERSION_MAJOR << " COMPONENTS Core Network REQUIRED)"
            << "\n";
        out << "find_package(Cutelyst" << CUTELYST_VERSION_MAJOR << "Qt" << QT_VERSION_MAJOR
            << " REQUIRED)" << "\n";
        out << "\n";
        out << "# Auto generate moc files" << "\n";
        out << "set(CMAKE_AUTOMOC ON)" << "\n";
        out << "\n";
        out << "# As moc files are generated in the binary dir, tell CMake" << "\n";
        out << "# to always look for includes there:" << "\n";
        out << "set(CMAKE_INCLUDE_CURRENT_DIR ON)" << "\n";
        out << "\n";
        out << "file(GLOB_RECURSE TEMPLATES_SRC root/*)" << "\n";
        out << "\n";
        out << "add_subdirectory(src)" << "\n";

        std::cout << OUT_CREATED << qPrintable(name) << '\n';

        return true;
    }

    std::cerr << qUtf8Printable(qtTrId("cutelytcmd-err-failed-create-file")) << qPrintable(name)
              << '\n';

    return false;
}

bool createDir(const QDir &parentDir, const QString &name)
{
    const QString newDir = parentDir.relativeFilePath(name);
    if (parentDir.exists(name)) {
        std::cout << OUT_EXISTS << qPrintable(newDir) << '\n';
        return true;
    } else if (parentDir.mkdir(name)) {
        std::cout << OUT_CREATED << qPrintable(newDir) << '\n';
        return true;
    }

    //% "Error: failed to create directory:"
    std::cerr << qUtf8Printable(qtTrId("cutelytcmd-err-failed-create-dir")) << qPrintable(newDir)
              << '\n';

    return false;
}

bool createApplication(const QString &name)
{
    QString nameWithUnderscore = name;
    nameWithUnderscore.replace(u'-', u'_');

    const static QRegularExpression nonWordRE(u"\\W"_s);
    const static QRegularExpression nonDigitRE(u"^\\d"_s);

    if (nameWithUnderscore.contains(nonWordRE) || nameWithUnderscore.contains(nonDigitRE)) {
        //% "Error: invalid application name."
        std::cerr << qUtf8Printable(qtTrId("cutelystcmd-err-invalid-app-name"));
        return false;
    }

    const QDir currentDir = QDir::current();

    if (!createDir(currentDir, name)) {
        return false;
    }

    if (!buildProjectCMakeLists(name % u"/CMakeLists.txt"_s, nameWithUnderscore)) {
        return false;
    }

    if (!createDir(currentDir, name % u"/build"_s)) {
        return false;
    }

    if (!createDir(currentDir, name % u"/root"_s)) {
        return false;
    }

    if (!createDir(currentDir, name % QLatin1String("/src"))) {
        return false;
    }

    if (!buildSrcCMakeLists(name % QLatin1String("/src/CMakeLists.txt"), nameWithUnderscore)) {
        return false;
    }

    if (!buildControllerHeader(name % QLatin1String("/src/root.h"), u"Root"_s, true)) {
        return false;
    }

    if (!buildControllerImplementation(name % QLatin1String("/src/root.cpp"), u"Root"_s, true)) {
        return false;
    }

    if (!buildApplicationHeader(name % QLatin1String("/src/") % name.toLower() %
                                    QLatin1String(".h"),
                                nameWithUnderscore)) {
        return false;
    }

    if (!buildApplicationImplementation(name % QLatin1String("/src/") % name.toLower() %
                                            QLatin1String(".cpp"),
                                        nameWithUnderscore)) {
        return false;
    }

    //% "Change to the build directory inside your application directory and run \"cmake ..\" "
    //% "to make sure your install is complete."
    std::cout << qUtf8Printable(qtTrId("cutelystcmd-info-build-steps")) << '\n';

    return true;
}

int main(int argc, char *argv[])
{
    QByteArray logging = qgetenv("QT_LOGGING_RULES");
    if (!logging.isEmpty()) {
        logging.append(';');
    }
    logging.append("cutelyst.*=true");
    qputenv("QT_LOGGING_RULES", logging);

    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName(u"Cutelyst"_s);
    QCoreApplication::setOrganizationDomain(u"cutelyst.org"_s);
    QCoreApplication::setApplicationName(u"cutelyst"_s);
    QCoreApplication::setApplicationVersion(QStringLiteral(CUTELYST_VERSION));

    QTranslator qtTranslator;
    if (qtTranslator.load(QLatin1String("qt_") % QLocale::system().name(),
                          QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        QCoreApplication::installTranslator(&qtTranslator);
    } else {
        std::cerr << "Error: can not load Qt translations" << '\n';
    }

    QTranslator appTranslator;
    if (appTranslator.load(
            QLocale(), u"cutelystcmd"_s, u"."_s, QStringLiteral(CUTELYST_I18N_DIR))) {
        QCoreApplication::installTranslator(&appTranslator);
    } else {
        std::cerr << "Error: can not load app translations" << '\n';
    }

    QCommandLineParser parser;
    parser.setApplicationDescription(
        //% "Cutelyst DEVELOPER helper, it can create a skeleton for new "
        //% "applications and controllers and can start your application."
        qtTrId("cutelystcmd-app-desc"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption appName(u"create-app"_s,
                               //: CLI option description
                               //% "Create a new Cutelyst application with the given name."
                               qtTrId("cutelystcmd-opt-create-app-desc"),
                               //: CLI option value name
                               //% "name"
                               qtTrId("cutelystcmd-opt-value-name"));
    parser.addOption(appName);

    QCommandLineOption controller(u"controller"_s,
                                  //: CLI option description
                                  //% "Create a new Cutelyst controller with the given name."
                                  qtTrId("cutelystcmd-opt-controller-desc"),
                                  qtTrId("cutelystcmd-opt-value-name"));
    parser.addOption(controller);

    QCommandLineOption serverOption(u"server"_s,
                                    //: CLI option description
                                    //% "Start a HTTP server."
                                    qtTrId("cutelystcmd-opt-server-desc"));
    parser.addOption(serverOption);

    QCommandLineOption appFile(u"app-file"_s,
                               //: CLI option description
                               //% "Application file to use with the server (usually in "
                               //% "build/src/lib*.so). If not set it will try to auto-detect."
                               qtTrId("cutelystcmd-opt-app-file-desc"),
                               //: CLI option value name
                               //% "file"
                               qtTrId("cutelystcmd-opt-app-file-value"));
    parser.addOption(appFile);

    QCommandLineOption serverPort({u"server-port"_s, u"p"_s},
                                  //: CLI option description
                                  //% "Development server port. Default: 3000"
                                  qtTrId("cutelystcmd-opt-server-port-desc"),
                                  //: CLI option value name
                                  //% "port"
                                  qtTrId("cutelystcmd-opt-server-port-value"),
                                  u"3000"_s);
    parser.addOption(serverPort);

    QCommandLineOption restartOpt(
        {u"restart"_s, u"r"_s},
        //: CLI option description
        //% "Restarts the development server when the applicatoin file changes."
        qtTrId("cutelystcmd-opt-restart-desc"));
    parser.addOption(restartOpt);

    const QStringList arguments = QCoreApplication::arguments();
    QStringList argsBeforeDashDash;
    QStringList argsAfterDashDash = arguments.mid(0, 1);

    int pos = arguments.indexOf(u"--"_s);
    if (pos != -1) {
        argsBeforeDashDash = arguments.mid(0, pos);
        argsAfterDashDash.append(arguments.mid(pos + 1));
    } else {
        argsBeforeDashDash = arguments;
    }

    // Process the actual command line arguments given by the user
    parser.process(argsBeforeDashDash);

    if (parser.isSet(appName)) {
        QString name = parser.value(appName);
        if (!createApplication(name)) {
            parser.showHelp(2);
        }
    } else if (parser.isSet(controller)) {
        QString name = parser.value(controller);
        if (!createController(name)) {
            parser.showHelp(3);
        }
    } else if (parser.isSet(serverOption)) {
        bool portOk = false;
        int port    = parser.value(serverPort).toInt(&portOk);
        if (!portOk) {
            //% "Error: no valid port specified."
            std::cerr << qUtf8Printable(qtTrId("cutelystcmd-err-invalid-port")) << '\n';
            return 1;
        }

        Cutelyst::Server server;

        server.parseCommandLine(argsAfterDashDash);

        server.setHttpSocket({QLatin1Char(':') + QString::number(port)});

        bool restart = parser.isSet(restartOpt);
        server.setMaster(restart);
        server.setAutoReload(restart);
        server.setLazy(restart);

        QDir projectDir;
        bool hasProjectDir = Helper::findProjectDir(QDir::current(), &projectDir);
        if (!hasProjectDir) {
            std::cerr << qUtf8Printable(qtTrId("cutelystcmd-err-fail-find-proj")) << '\n';
            return 1;
        } else {
            server.setChdir2(projectDir.absolutePath());
        }

        QString localFilename = parser.value(appFile);
        if (localFilename.isEmpty()) {
            localFilename = Helper::findApplication(projectDir);
            if (!QFile::exists(localFilename)) {
                //% "Error: application file not found."
                std::cerr << qUtf8Printable(qtTrId("cutelystcmd-err-fail-find-app")) << '\n';
                return 1;
            }
        }
        server.setApplication(localFilename);

        return server.exec();
    } else {
        parser.showHelp(1);
    }

    return 0;
}
