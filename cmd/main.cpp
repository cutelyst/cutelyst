
#include <QCoreApplication>
#include <QCommandLineParser>

#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include <QTextStream>

#include <QRegularExpression>
#include <QStringBuilder>
#include <QDir>

#include <wsgi/wsgi.h>

#include <iostream>

#ifdef Q_OS_UNIX
#include <utime.h>
#endif

#include "config.h"
#include "helper.h"

#define OUT_EXISTS  "  exists "
#define OUT_CREATED " created "

bool buildControllerHeader(const QString &filename, const QString &controllerName, bool helpers);
bool buildControllerImplementation(const QString &filename, const QString &controllerName, bool helpers);

bool createController(const QString &controllerName)
{
    if (controllerName.contains(QRegularExpression(QStringLiteral("\\W"))) || controllerName.contains(QRegularExpression(QStringLiteral("^\\d")))) {
        std::cerr << "Error: Invalid Controller name." << std::endl;
        return false;
    }

    QDir projectDir;
    if (!Helper::findProjectDir(QDir::current(), &projectDir)) {
        std::cerr << "Error: failed to find project" << std::endl;
        return false;
    }

    if (!buildControllerHeader(projectDir.absoluteFilePath(QStringLiteral("src/")) % controllerName.toLower() % QLatin1String(".h"),
                               controllerName,
                               false)) {
        return false;
    }

    if (!buildControllerImplementation(projectDir.absoluteFilePath(QStringLiteral("src/")) % controllerName.toLower() % QLatin1String(".cpp"),
                                       controllerName,
                                       false)) {
        return false;
    }

#ifdef Q_OS_UNIX
    // Change the modification time of CMakeLists.txt to force FILE_GLOB to be updated
    utime(projectDir.absoluteFilePath(QStringLiteral("CMakeLists.txt")).toLatin1().data(), NULL);
#endif

    std::cout << "Now, on your application class include and instantiate the controller." << std::endl;

    return true;
}

bool buildApplicationImplementation(const QString &filename, const QString &appName)
{
    QFile data(filename);
    if (data.exists()) {
        std::cerr << OUT_EXISTS << qPrintable(filename) << std::endl;
        return true;
    }

    if (data.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&data);
        QFileInfo fileInfo(filename);
        out << "#include \"" << fileInfo.baseName() << ".h\"" << "\n";
        out << "\n";
        out << "#include <Cutelyst/Plugins/StaticSimple/staticsimple.h>" << "\n";
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
        out << "    new StaticSimple(this);" << "\n";
        out << "\n";
        out << "    return true;" << "\n";
        out << "}" << "\n";
        out << "\n";

        std::cout << OUT_CREATED << qPrintable(filename) << std::endl;

        return true;
    }
    std::cerr << "Error: failed to create file" << qPrintable(filename) << std::endl;

    return false;
}

bool buildApplicationHeader(const QString &filename, const QString &appName)
{
    QFile data(filename);
    if (data.exists()) {
        std::cout << OUT_EXISTS << qPrintable(filename) << std::endl;
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
        out << "    CUTELYST_APPLICATION(IID \""<< appName << "\")" << "\n";
        out << "public:" << "\n";
        out << "    Q_INVOKABLE explicit " << appName << "(QObject *parent = 0);" << "\n";
        out << "    ~" << appName << "();" << "\n";
        out << "\n";
        out << "    bool init();" << "\n";
        out << "};" << "\n";
        out << "\n";
        out << "#endif //" << appName.toUpper() << "_H" << "\n";
        out << "\n";

        std::cout << OUT_CREATED << qPrintable(filename) << std::endl;

        return true;
    }
    std::cerr << "Error: failed to create file" << qPrintable(filename) << std::endl;

    return false;
}

bool buildControllerImplementation(const QString &filename, const QString &controllerName, bool helpers)
{
    QFile data(filename);
    if (data.exists()) {
        std::cout << OUT_EXISTS << qPrintable(filename) << std::endl;
        return true;
    }

    if (data.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&data);
        QFileInfo fileInfo(filename);
        out << "#include \"" << fileInfo.baseName() << ".h\"" << "\n";
        out << "\n";
        out << "using namespace Cutelyst;" << "\n";
        out << "\n";
        out << controllerName << "::" << controllerName << "(QObject *parent) : Controller(parent)" << "\n";
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
            out << "    c->response()->body() = \"Matched Controller::" << controllerName << " in " << controllerName << ".\";" << "\n";
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

        std::cout << OUT_CREATED << qPrintable(filename) << std::endl;

        return true;
    }
    std::cerr << "Error: failed to create file" << qPrintable(filename) << std::endl;

    return false;
}

bool buildControllerHeader(const QString &filename, const QString &controllerName, bool helpers)
{
    QFile data(filename);
    if (data.exists()) {
        std::cout << OUT_EXISTS << qPrintable(filename) << std::endl;
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
        out << "    explicit " << controllerName << "(QObject *parent = 0);" << "\n";
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

        std::cout << OUT_CREATED << qPrintable(filename) << std::endl;

        return true;
    }
    std::cerr << "Error: failed to create file" << qPrintable(filename) << std::endl;

    return false;
}

bool buildSrcCMakeLists(const QString &name, const QString &appName)
{
    QFile data(name);
    if (data.exists()) {
        std::cout << OUT_EXISTS << qPrintable(name) << std::endl;
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
        out << "    Cutelyst::StaticSimple" << "\n";
        out << "    Qt5::Core" << "\n";
        out << "    Qt5::Network" << "\n";
        out << ")" << "\n";
        out << "\n";

        std::cout << OUT_CREATED << qPrintable(name) << std::endl;

        return true;
    }
    std::cerr << "Error: failed to create file" << qPrintable(name) << std::endl;

    return false;
}

bool buildProjectCMakeLists(const QString &name, const QString &appName)
{
    QFile data(name);
    if (data.exists()) {
        std::cout << OUT_EXISTS << qPrintable(name) << std::endl;
        return true;
    }

    if (data.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&data);
        out << "project(" <<  appName << ")" << "\n";
        out << "\n";
        out << "cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)" << "\n";
        out << "if (POLICY CMP0043)" << "\n";
        out << "  cmake_policy(SET CMP0043 NEW)" << "\n";
        out << "endif()" << "\n";
        out << "\n";
        out << "if(WIN32)\n";
        out << "  if(MSVC)\n";
        out << "    add_definitions(-D_SCL_SECURE_NO_WARNINGS)\n";
        out << "    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)\n";
        out << "  endif()\n";
        out << "endif()\n\n";
        out << "find_package(Qt5 COMPONENTS Core Network REQUIRED)" << "\n";
        out << "find_package(CutelystQt5 REQUIRED)" << "\n";
        out << "\n";
        out << "# Auto generate moc files" << "\n";
        out << "set(CMAKE_AUTOMOC ON)" << "\n";
        out << "\n";
        out << "# As moc files are generated in the binary dir, tell CMake" << "\n";
        out << "# to always look for includes there:" << "\n";
        out << "set(CMAKE_INCLUDE_CURRENT_DIR ON)" << "\n";
        out << "\n";
        out << "# Enable C++11 features" << "\n";
        out << "add_definitions(-std=c++11)" << "\n";
        out << "\n";
        out << "include_directories(" << "\n";
        out << "    ${CMAKE_SOURCE_DIR}" << "\n";
        out << "    ${CMAKE_CURRENT_BINARY_DIR}" << "\n";
        out << "    ${CutelystQt5_INCLUDE_DIR}" << "\n";
        out << ")" << "\n";
        out << "\n";
        out << "file(GLOB_RECURSE TEMPLATES_SRC root/*)" << "\n";
        out << "\n";
        out << "add_subdirectory(src)" << "\n";

        std::cout << OUT_CREATED << qPrintable(name) << std::endl;

        return true;
    }
    std::cerr << "Error: failed to create file" << qPrintable(name) << std::endl;

    return false;
}

bool createDir(const QDir &parentDir, const QString &name)
{
    const QString newDir = parentDir.relativeFilePath(name);
    if (parentDir.exists(name)) {
        std::cout << OUT_EXISTS << qPrintable(newDir) << std::endl;
        return true;
    } else if (parentDir.mkdir(name)) {
        std::cout << OUT_CREATED << qPrintable(newDir) << std::endl;
        return true;
    }

    std::cerr << "Error: failed to create directory:" << qPrintable(newDir) << std::endl;
    return false;
}

bool createApplication(const QString &name)
{
    QString nameWithUnderscore = name;
    nameWithUnderscore.replace(QLatin1Char('-'), QLatin1Char('_'));
    if (nameWithUnderscore.contains(QRegularExpression(QStringLiteral("\\W"))) || nameWithUnderscore.contains(QRegularExpression(QStringLiteral("^\\d")))) {
        std::cerr << "Error: Invalid Application name." << std::endl;
        return false;
    }

    const QDir currentDir = QDir::current();

    if (!createDir(currentDir, name)) {
        return false;
    }

    if (!buildProjectCMakeLists(name % QStringLiteral("/CMakeLists.txt"), nameWithUnderscore)) {
        return false;
    }

    if (!createDir(currentDir, name % QStringLiteral("/build"))) {
        return false;
    }

    if (!createDir(currentDir, name % QStringLiteral("/root"))) {
        return false;
    }

    if (!createDir(currentDir, name % QLatin1String("/src"))) {
        return false;
    }

    if (!buildSrcCMakeLists(name % QLatin1String("/src/CMakeLists.txt"), nameWithUnderscore)) {
        return false;
    }

    if (!buildControllerHeader(name % QLatin1String("/src/root.h"),
                               QStringLiteral("Root"),
                               true)) {
        return false;
    }

    if (!buildControllerImplementation(name % QLatin1String("/src/root.cpp"),
                                       QStringLiteral("Root"),
                                       true)) {
        return false;
    }

    if (!buildApplicationHeader(name % QLatin1String("/src/") % name.toLower() % QLatin1String(".h"),
                                nameWithUnderscore)) {
        return false;
    }

    if (!buildApplicationImplementation(name % QLatin1String("/src/") % name.toLower() % QLatin1String(".cpp"),
                                        nameWithUnderscore)) {
        return false;
    }

    std::cout << "Change to application directory, then build directory and Run \"cmake ..\" to make sure your install is complete" << std::endl;

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
    QCoreApplication::setOrganizationName(QStringLiteral("Cutelyst"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("cutelyst.org"));
    QCoreApplication::setApplicationName(QStringLiteral("cutelyst"));
    QCoreApplication::setApplicationVersion(QStringLiteral(VERSION));

    QTranslator qtTranslator;
    qtTranslator.load(QLatin1String("qt_") % QLocale::system().name(),
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    QCoreApplication::installTranslator(&qtTranslator);


    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Cutelyst DEVELOPER helper, it can create a skeleton for a new application, controllers and start your application"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption appName(QStringLiteral("create-app"),
                               QStringLiteral("Creates a new Cutelyst application"),
                               QStringLiteral("app_name"));
    parser.addOption(appName);

    QCommandLineOption controller(QStringLiteral("controller"),
                                  QStringLiteral("Name of the Controller application to create"),
                                  QStringLiteral("controller_name"));

    parser.addOption(controller);

    QCommandLineOption server(QStringLiteral("server"),
                              QStringLiteral("Starts a HTTP server"));
    parser.addOption(server);

    QCommandLineOption appFile(QStringLiteral("app-file"),
                               QStringLiteral("Application file of to use with the server (usually in build/src/lib*.so), if not set it will try to auto-detect"),
                               QStringLiteral("file_name"));
    parser.addOption(appFile);

    QCommandLineOption serverPort({ QStringLiteral("server-port"), QStringLiteral("p") },
                                  QStringLiteral("Development server port"),
                                  QStringLiteral("port"));
    parser.addOption(serverPort);

    QCommandLineOption restartOpt({ QStringLiteral("restart"), QStringLiteral("r") },
                                  QStringLiteral("Restarts the development server when the application file changes"));
    parser.addOption(restartOpt);

    const QStringList arguments = app.arguments();
    QStringList argsBeforeDashDash;
    QStringList argsAfterDashDash = arguments.mid(0, 1);

    int pos = arguments.indexOf(QStringLiteral("--"));
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
    } else if (parser.isSet(server)) {
        int port = 3000;
        if (parser.isSet(serverPort)) {
            port = parser.value(serverPort).toInt();
        }

        CWSGI::WSGI wsgi;

        wsgi.parseCommandLine(argsAfterDashDash);

        wsgi.setHttpSocket({ QLatin1Char(':') + QString::number(port) });

        bool restart = parser.isSet(restartOpt);
        wsgi.setMaster(restart);
        wsgi.setAutoReload(restart);
        wsgi.setLazy(restart);

        QDir projectDir;
        if (!Helper::findProjectDir(QDir::current(), &projectDir)) {
            std::cerr << "Error: failed to find project" << std::endl;
            return false;
        }
        wsgi.setChdir2(projectDir.absolutePath());

        QString localFilename = parser.value(appFile);
        if (localFilename.isEmpty()) {
            localFilename = Helper::findApplication(projectDir);
            if (!QFile::exists(localFilename)) {
                std::cerr << "Error: Application file not found" << std::endl;
                return 1;
            }
        }
        wsgi.setApplication(localFilename);

        return wsgi.exec();
    } else {
        parser.showHelp(1);
    }

    return 0;
}
