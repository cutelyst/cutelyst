
#include <QCoreApplication>
#include <wsgi/wsgi.h>
#include "map.h"

int main(int argc, char *argv[])
{

  QCoreApplication app(argc, argv);
  auto myapp = new Map();
  auto wsgi = new CWSGI::WSGI;
  wsgi->setHttpSocket({
    { QStringLiteral(":3000") },
  });
  wsgi->exec(myapp);
}
