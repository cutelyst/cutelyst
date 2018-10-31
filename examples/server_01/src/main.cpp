#include <wsgi/wsgi.h>
#include "server_01.h"

int main(int argc, char *argv[])
{
  auto app = new server_01;
  auto wsgi = new CWSGI::WSGI;
  wsgi->setHttpSocket({
    { QStringLiteral(":3000") },
  });
  wsgi->exec(app);
}
