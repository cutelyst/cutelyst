
#include <QCoreApplication>
#include <wsgi/wsgi.h>
#include "hello.h"

int main(int argc, char *argv[])
{

    QCoreApplication app(argc, argv);
    auto myapp = new HelloWorld();
    auto wsgi = new CWSGI::WSGI;
    wsgi->setHttpSocket({
                            { QStringLiteral(":3000") },
                        });
    wsgi->setUpgradeH2c(true);
    wsgi->setBufferSize(16393);
    wsgi->setHttp2Socket({
                             { QStringLiteral(":3001") },
                         });
    wsgi->setFastcgiSocket({
                             { QStringLiteral(":3002") },
                         });
    wsgi->exec(myapp);
}
