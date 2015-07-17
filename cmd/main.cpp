
#include <QCoreApplication>
#include <QCommandLineParser>

#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDebug>

#include <QRegularExpression>
#include <QStringBuilder>
#include <QProcess>
#include <QDirIterator>
#include <QDir>

#include <QMimeDatabase>

#include <ostream>
#include <utime.h>

#define OUT_EXISTS  "  exists"
#define OUT_CREATED " created"

bool buildControllerHeader(const QString &filename, const QString &controllerName, bool helpers);
bool buildControllerImplementation(const QString &filename, const QString &controllerName, bool helpers);
bool findProjectDir(const QDir &dir, QDir *projectDir);

QString findApplication(const QDir &projectDir)
{
    QMimeDatabase m_db;

    QDirIterator it(projectDir.absolutePath(), QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString file = it.next();
        QMimeType mime = m_db.mimeTypeForFile(file);
        if (mime.inherits(QStringLiteral("application/x-sharedlib"))) {
            return file;
        }
    }
    return QString();
}

int runServer(const QString &appFilename, int port, bool restart)
{
    QDir projectDir;
    if (!findProjectDir(QDir::current(), &projectDir)) {
        qDebug() << "Error: failed to find project";
        return false;
    }

    QString localFilename = appFilename;
    if (localFilename.isEmpty()) {
        localFilename = findApplication(projectDir);
    }

    QFileInfo fileInfo(localFilename);
    if (!fileInfo.exists()) {
        qDebug() << "Error: Application file not found";
        return false;
    }

    QStringList args;
    args.append(QStringLiteral("--http-socket"));
    args.append(QLatin1String(":") % QString::number(port));

    args.append(QStringLiteral("--chdir"));
    args.append(projectDir.absolutePath());

    args.append(QStringLiteral("-M"));

    args.append(QStringLiteral("--plugin"));
    args.append(QStringLiteral("cutelyst"));

    args.append(QStringLiteral("--cutelyst-app"));
    args.append(localFilename);

    if (restart) {
        args.append(QStringLiteral("--cutelyst-reload"));
    }

    qDebug() << "Running: uwsgi" << args.join(" ").toLatin1().data();

    return QProcess::execute("uwsgi", args);
}

bool findProjectDir(const QDir &dir, QDir *projectDir)
{
    QFile cmake(dir.absoluteFilePath(QStringLiteral("CMakeLists.txt")));
    if (cmake.exists()) {
        if (cmake.open(QFile::ReadOnly | QFile::Text)) {
            while (!cmake.atEnd()) {
                QByteArray line = cmake.readLine();
                if (line.toLower().startsWith(QByteArrayLiteral("project"))) {
                    *projectDir = dir;
                    return true;
                }
            }
        }
    }

    QDir localDir = dir;
    if (localDir.cdUp()) {
        return findProjectDir(localDir, projectDir);
    }
    return false;
}

bool createController(const QString &controllerName)
{
    if (controllerName.contains(QRegularExpression("\\W")) || controllerName.contains(QRegularExpression("^\\d"))) {
        qDebug() << "Error: Invalid Controller name.";
        return false;
    }

    QDir projectDir;
    if (!findProjectDir(QDir::current(), &projectDir)) {
        qDebug() << "Error: failed to find project";
        return false;
    }

    if (!buildControllerHeader(projectDir.absoluteFilePath("src/") % controllerName.toLower() % QLatin1String(".h"),
                               controllerName,
                               false)) {
        return false;
    }

    if (!buildControllerImplementation(projectDir.absoluteFilePath("src/") % controllerName.toLower() % QLatin1String(".cpp"),
                                       controllerName,
                                       false)) {
        return false;
    }

    // Change the modification time of CMakeLists.txt to force FILE_GLOB to be updated
    utime(projectDir.absoluteFilePath(QStringLiteral("CMakeLists.txt")).toLatin1().data(), NULL);

    qDebug() << "Now, on your application class include and instantiate the controller.";

    return true;
}

bool buildApplicationImplementation(const QString &filename, const QString &appName)
{
    QFile data(filename);
    if (data.exists()) {
        qDebug() << OUT_EXISTS << filename;
        return true;
    }

    if (data.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&data);
        QFileInfo fileInfo(filename);
        out << "#include \"" << fileInfo.baseName() << ".h\"" << "\n";
        out << "\n";
        out << "#include <Cutelyst/Plugins/StaticSimple>" << "\n";
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
        out << "    new Root(this));" << "\n";
        out << "\n";
        out << "    new StaticSimple(this);" << "\n";
        out << "\n";
        out << "    return true;" << "\n";
        out << "}" << "\n";
        out << "\n";

        qDebug() << OUT_CREATED << filename;

        return true;
    }
    qDebug() << "Error: failed to create file" << filename;

    return false;
}

bool buildApplicationHeader(const QString &filename, const QString &appName)
{
    QFile data(filename);
    if (data.exists()) {
        qDebug() << OUT_EXISTS << filename;
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

        qDebug() << OUT_CREATED << filename;

        return true;
    }
    qDebug() << "Error: failed to create file" << filename;

    return false;
}

bool buildControllerImplementation(const QString &filename, const QString &controllerName, bool helpers)
{
    QFile data(filename);
    if (data.exists()) {
        qDebug() << OUT_EXISTS << filename;
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
            out << "    c->response()->body() = c->welcomeMessage();" << "\n";
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

        qDebug() << OUT_CREATED << filename;

        return true;
    }
    qDebug() << "Error: failed to create file" << filename;

    return false;
}

bool buildControllerHeader(const QString &filename, const QString &controllerName, bool helpers)
{
    QFile data(filename);
    if (data.exists()) {
        qDebug() << OUT_EXISTS << filename;
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
        out << "    C_ATTR(index, :Path :Args(0))" << "\n";
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

        qDebug() << OUT_CREATED << filename;

        return true;
    }
    qDebug() << "Error: failed to create file" << filename;

    return false;
}

bool buildSrcCMakeLists(const QString &name, const QString &appName)
{
    QFile data(name);
    if (data.exists()) {
        qDebug() << OUT_EXISTS << name;
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
        out << "    Cutelyst::cutelyst-qt5" << "\n";
        out << "    Qt5::Core" << "\n";
        out << "    Qt5::Network" << "\n";
        out << ")" << "\n";
        out << "\n";

        qDebug() << OUT_CREATED << name;

        return true;
    }
    qDebug() << "Error: failed to create file" << name;

    return false;
}

bool buildProjectCMakeLists(const QString &name, const QString &appName)
{
    QFile data(name);
    if (data.exists()) {
        qDebug() << OUT_EXISTS << name;
        return true;
    }

    if (data.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&data);
        out << "project(" <<  appName << ")" << "\n";
        out << "\n";
        out << "cmake_minimum_required(VERSION 2.8.6 FATAL_ERROR)" << "\n";
        out << "\n";
        out << "find_package(Qt5 COMPONENTS Core Network )" << "\n";
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

        qDebug() << OUT_CREATED << name;

        return true;
    }
    qDebug() << "Error: failed to create file" << name;

    return false;
}

bool createDir(const QDir &parentDir, const QString &name)
{
    QString newDir = parentDir.relativeFilePath(name);
    if (parentDir.exists(name)) {
        qDebug() << OUT_EXISTS << newDir;
        return true;
    } else if (parentDir.mkdir(name)) {
        qDebug() << OUT_CREATED << newDir;
        return true;
    }

    qDebug() << "Error: failed to create directory:" << newDir;
    return false;
}

bool createApplication(const QString &name)
{
    if (name.contains(QRegularExpression("\\W")) || name.contains(QRegularExpression("^\\d"))) {
        qDebug() << "Error: Invalid Application name.";
        return false;
    }

    QDir currentDir = QDir::current();

    if (!createDir(currentDir, name)) {
        return false;
    }

    if (!buildProjectCMakeLists(name % QStringLiteral("/CMakeLists.txt"), name)) {
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

    if (!buildSrcCMakeLists(name % QLatin1String("/src/CMakeLists.txt"), name)) {
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

    if (!buildApplicationHeader(name % QLatin1String("/src/") % name.toLower() % QLatin1String(".h"), name)) {
        return false;
    }

    if (!buildApplicationImplementation(name % QLatin1String("/src/") % name.toLower() % QLatin1String(".cpp"), name)) {
        return false;
    }

    qDebug() << "Change to application directory, then build directory and Run \"cmake ..\" to make sure your install is complete";

    return true;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Cutelyst");
    QCoreApplication::setOrganizationDomain("cutelyst.org");
    QCoreApplication::setApplicationName("cutelyst");
    QCoreApplication::setApplicationVersion("0.0.1");

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    QCoreApplication::installTranslator(&qtTranslator);


    QCommandLineParser parser;
    parser.setApplicationDescription("Creates a skeleton for a new application, and for controllers");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption appName = QCommandLineOption("create-app",
                                                    "Creates a new Cutelyst application",
                                                    "app_name");
    parser.addOption(appName);
    QCommandLineOption controller = QCommandLineOption("controller",
                                                       "Name of the Controller application to create",
                                                       "controller_name");

    parser.addOption(controller);
    QCommandLineOption server = QCommandLineOption("server",
                                                   "Development server (requires uWSGI)");
    parser.addOption(server);
    QCommandLineOption appFile = QCommandLineOption("app-file",
                                                    "Application file of to use with the server (usually in build/src/lib*.so),"
                                                    " if not set it will try to auto-detect",
                                                    "file_name");
    parser.addOption(appFile);
    QCommandLineOption serverPort = QCommandLineOption({ "server-port", "p" },
                                                       "Development server port",
                                                       "port");
    parser.addOption(serverPort);
    QCommandLineOption restart = QCommandLineOption({ "restart", "r" },
                                                    "Restarts the development server when the application file changes");
    parser.addOption(restart);

    // Process the actual command line arguments given by the user
    parser.process(app);

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
        QString filename = parser.value(appFile);
        int port = 3000;
        if (parser.isSet(serverPort)) {
            port = parser.value(serverPort).toInt();
        }
        return runServer(filename, port, parser.isSet(restart));
    } else {
        parser.showHelp(1);
    }

    return 0;
}
