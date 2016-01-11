#ifndef _TEST_COVERAGE_OBJECT_H
#define _TEST_COVERAGE_OBJECT_H
#include <QObject>
class CoverageObject : public QObject
{
  Q_OBJECT
  public:
    CoverageObject(QObject *p=NULL) : QObject(p) {}
    virtual void initTest() {}
    virtual void cleanupTest() {}
  protected Q_SLOTS:
    void init() ;
    void cleanup();
  private:
    void saveCoverageData();
    QString generateTestName() const;
};
#endif
