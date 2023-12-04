%{Cpp:LicenseTemplate}\
#include "root.h"

#include <QCoreApplication>
#include <Cutelyst/Application>

using namespace Cutelyst;

Root::Root(QObject *parent) : Controller(parent)
{
}

Root::~Root()
{
}

void Root::index(Context *c)
{
    QString body;
    body = "<!DOCTYPE html PUBLIC \\"-//W3C//DTD XHTML 1.0 Transitional//EN\\"\\n"
           "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\\">\\n"
           "<html xmlns=\\"http://www.w3.org/1999/xhtml\\" xml:lang=\\"en\\" lang=\\"en\\">\\n"
           "    <head>\\n"
           "        <meta http-equiv=\\"Content-Language\\" content=\\"en\\" />\\n"
           "        <meta http-equiv=\\"Content-Type\\" content=\\"text/html; charset=utf-8\\" />\\n"
           "        <title>%1 on Cutelyst %2</title>\\n"
           "    </head>\\n"
           "    <body>\\n"
           "        <div id=\\"content\\">\\n"
           "            <div id=\\"topbar\\">\\n"
           "                <h1><span id=\\"appname\\">%1</span> on <a href=\\"http://cutelyst.org\\">Cutelyst</a>%2</h1>\\n"
           "            </div>\\n"
           "        </div>\\n"
           "    <body>\\n"
           "</html>\\n";
    body = body.arg(QCoreApplication::applicationName()).arg(Application::cutelystVersion());
    c->response()->setBody(body);
    c->response()->setContentType(QLatin1String("text/html; charset=utf-8"));
}

void Root::defaultPage(Context *c)
{
    c->response()->setBody(QStringLiteral("Page not found!"));
    c->response()->setStatus(404);
}

