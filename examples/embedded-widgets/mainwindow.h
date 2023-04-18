#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>

namespace Ui {
class MainWindow;
}
namespace Cutelyst {
class Server;
class Context;
} // namespace Cutelyst
class EmbeddedApp;
class QNetworkAccessManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void clientSend();
    void listenClicked();

    void indexCalled(Cutelyst::Context *c);

private:
    void updateUrl();
    Ui::MainWindow *ui;

    Cutelyst::Server *m_server;
    EmbeddedApp *m_app;
    QNetworkAccessManager *m_nam;

    QStandardItemModel *m_serverReceivedHeaders;
    QStandardItemModel *m_clientReceivedHeaders;
};

#endif // MAINWINDOW_H
