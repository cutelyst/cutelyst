#ifndef _TEST_COVERAGE_OBJECT_H
#define _TEST_COVERAGE_OBJECT_H
#include <QObject>
#include <Cutelyst/Engine>
class CoverageObject : public QObject
{
  Q_OBJECT
  public:
    CoverageObject(QObject *parent = nullptr) : QObject(parent) {}
    virtual void initTest() {}
    virtual void cleanupTest() {}
  protected Q_SLOTS:
    void init() ;
    void cleanup();
  private:
    void saveCoverageData();
    QString generateTestName() const;
};

class TestEngine : public Cutelyst::Engine
{
    Q_OBJECT
public:
    explicit TestEngine(const QVariantMap &opts, QObject *parent = nullptr);

    virtual int workerId() const override;

    virtual int workerCore() const override;

    QByteArray createRequest(const QString &method, const QString &path, const QByteArray &query, const Cutelyst::Headers &headers, QByteArray *body);

protected:
    virtual qint64 doWrite(Cutelyst::Context *c, const char *data, qint64 len, void *engineData);
    virtual bool init();

private:
    QByteArray m_responseData;
};

#endif
