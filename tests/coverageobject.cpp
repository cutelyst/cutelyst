#include "coverageobject.h"
#include <QTest>
#include <QMetaObject>
#include <QDir>
#include <QString>
#include <QDebug>
#include <QtDebug>
#include <QLibrary>
#include <QtCore/QBuffer>

#include <Cutelyst/context.h>

#include "cutelyst_paths.h"

using namespace Cutelyst;

void CoverageObject::init()
{
  initTest();
}

QString CoverageObject::generateTestName() const
{
  QString test_name;
  test_name+=QString::fromLatin1(metaObject()->className());
  test_name+=QString::fromLatin1("/");
  test_name+=QString::fromLatin1(QTest::currentTestFunction());
  if (QTest::currentDataTag())
  {
    test_name+=QString::fromLatin1("/");
    test_name+=QString::fromLatin1(QTest::currentDataTag());
  }
  return test_name;
}

void CoverageObject::saveCoverageData()
{
#ifdef __COVERAGESCANNER__
  QString test_name;
  test_name += generateTestName();

  __coveragescanner_testname(test_name.toStdString().c_str());
  if (QTest::currentTestFailed())
    __coveragescanner_teststate("FAILED");
  else
    __coveragescanner_teststate("PASSED") ;
  __coveragescanner_save();
  __coveragescanner_testname("");
  __coveragescanner_clear();
#endif
}

void CoverageObject::cleanup()
{
  cleanupTest();
  saveCoverageData();
}

TestEngine::TestEngine(const QVariantMap &opts, QObject *parent) : Engine(opts, parent)
{

}

int TestEngine::workerId() const
{
    return 0;
}

int TestEngine::workerCore() const
{
    return 0;
}

QByteArray TestEngine::createRequest(const QString &method, const QString &path, const QByteArray &query, const Headers &headers, QByteArray *body)
{
    QBuffer buf(body);
    buf.open(QBuffer::ReadOnly);

    m_responseData = QByteArray();
    processRequest(method,
                   path,
                   query,
                   QStringLiteral("HTTP/1.1"),
                   false,
                   QStringLiteral("192.168.0.5"),
                   QStringLiteral("192.168.0.10"),
                   3203,
                   QString(), // RemoteUser
                   headers,
                   QDateTime::currentMSecsSinceEpoch(),
                   &buf,
                   0);

    return m_responseData;
}

qint64 TestEngine::doWrite(Context *c, const char *data, qint64 len, void *engineData)
{
    Q_UNUSED(c)
    Q_UNUSED(engineData)
    m_responseData.append(data, len);
}

bool TestEngine::init()
{
    return true;
}
