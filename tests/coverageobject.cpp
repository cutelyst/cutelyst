#include "coverageobject.h"
#include <QTest>
#include <QMetaObject>
#include <QDir>
#include <QString>
#include <QDebug>
#include <QtDebug>
#include <QLibrary>

#include "cutelyst_paths.h"

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
