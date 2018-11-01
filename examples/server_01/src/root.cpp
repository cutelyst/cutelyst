#include "root.h"
#include <QFile>
#include <QDir>
#include <QString>
#include <QDebug>
#include <sstream>

using namespace Cutelyst;

Root::Root(QObject *parent) : Controller(parent)
{
}

Root::~Root()
{
}

void Root::index(Context *c)
{
  QString fname = tr("leaflet-template.html");
  QFile file(fname);
  if (!file.open(QIODevice::ReadOnly))
  {
    qDebug() << fname;
    return;
  }
  QTextStream in(&file);
  QString body = in.readAll();
  file.close();
  std::ostringstream strm;
  strm
    << "<script>"
    << "var layer_base = L.tileLayer(\n"
    << "'http://cartodb-basemaps-{s}.global.ssl.fastly.net/light_all/{z}/{x}/{y}@2x.png',{\n"
    << "opacity: 1\n"
    << "});\n"
    << "var map = new L.Map('map', {\n"
    << "center: new L.LatLng(38.9072, -77.0369),\n"
    << "zoom: 13,\n"
    << "layers: [layer_base]\n"
    << "});\n"
    << "var circle = L.circle([38.9072, -77.0369], {"
    << "color: '#ff0000',"
    << "stroke: false,"
    << "radius : 500"
    << "}).addTo(map);"
    << "</script>";

  body += tr(strm.str().c_str());
  qDebug() << body;
  c->response()->body() = body.toUtf8();
}

void Root::defaultPage(Context *c)
{
  c->response()->body() = "Page not found!";
  c->response()->setStatus(404);
}

