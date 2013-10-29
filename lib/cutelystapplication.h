#ifndef CUTELYSTAPPLICATION_H
#define CUTELYSTAPPLICATION_H

#include <QCoreApplication>

class CutelystApplicationPrivate;
class CutelystApplication : public QCoreApplication
{
public:
    CutelystApplication(int &argc, char **argv);
    ~CutelystApplication();

    bool parseArgs();
    int printError();

protected:
    CutelystApplicationPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(CutelystApplication)

    void onNewConnection();
};

#endif // CUTELYSTAPPLICATION_H
