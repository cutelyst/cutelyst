#include "mainwindow.h"

#include "embeddedapp.h"
#include "ui_mainwindow.h"

#include <Cutelyst/Context>
#include <Cutelyst/Response>
#include <server/server.h>

#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_server(new Cutelyst::Server{this})
    , m_app(new EmbeddedApp{m_server})
    , m_nam(new QNetworkAccessManager{this})
    , m_serverReceivedHeaders(new QStandardItemModel{this})
    , m_clientReceivedHeaders(new QStandardItemModel{this})
{
    ui->setupUi(this);
    ui->serverReceivedHeaders->setModel(m_serverReceivedHeaders);
    ui->clientReceivedHeaders->setModel(m_clientReceivedHeaders);

    connect(ui->serverPortSB, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::updateUrl);
    connect(ui->clientSendPB, &QPushButton::clicked, this, &MainWindow::clientSend);
    connect(ui->serverListenPB, &QPushButton::clicked, this, &MainWindow::listenClicked);
    connect(ui->serverStopListenPB, &QPushButton::clicked, this, [=] {
        ui->serverStopListenPB->setEnabled(false);
        m_server->stop();
    });

    connect(m_server, &Cutelyst::Server::ready, this, [=] {
        ui->serverStopListenPB->setEnabled(true);
    });
    connect(m_server, &Cutelyst::Server::stopped, this, [=] {
        ui->serverListenPB->setEnabled(true);
    });
    connect(m_server, &Cutelyst::Server::errorOccured, this, [=](const QString &message) {
        ui->serverListenPB->setEnabled(true);
        ui->serverStopListenPB->setEnabled(false);
        QMessageBox::critical(this, tr("Failed to start server"), message);
    });

    connect(m_app, &EmbeddedApp::indexCalled, this, &MainWindow::indexCalled);
    updateUrl();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::clientSend()
{
    QString path = ui->clientPathLE->text();
    if (!path.startsWith(QLatin1Char('/'))) {
        path.prepend(QLatin1Char('/'));
    }
    QUrl url(QLatin1String("http://localhost:") + QString::number(ui->serverPortSB->value()) + path);
    QNetworkRequest request(url);

    QNetworkReply *reply = m_nam->sendCustomRequest(request, ui->clientMethodLE->currentText().toLatin1(), ui->clientBodyPTE->toPlainText().toUtf8());
    connect(reply, &QNetworkReply::finished, this, [=] {
        reply->deleteLater();

        ui->clientSendPB->setEnabled(true);

        const QByteArray body = reply->readAll();
        ui->clientResponsePTE->setPlainText(QString::fromUtf8(body));
        int status           = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QString statusReason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        qDebug() << "client finished" << body.size() << status << reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute);
        ui->clientResponseLE->setText(QString::number(status) + QLatin1String(" - ") + statusReason);

        m_clientReceivedHeaders->clear();
        for (auto &header : reply->rawHeaderPairs()) {
            qDebug() << "client " << header.first << header.second;
            auto keyItem   = new QStandardItem(QString::fromLatin1(header.first));
            auto valueItem = new QStandardItem(QString::fromLatin1(header.second));
            m_clientReceivedHeaders->appendRow({
                keyItem,
                valueItem,
            });
        }
    });
    ui->clientSendPB->setEnabled(false);
}

void MainWindow::listenClicked()
{
    ui->serverListenPB->setEnabled(false);

    const QString address = QLatin1String("localhost:") + QString::number(ui->serverPortSB->value());
    m_server->setHttpSocket({address});
    m_server->start(m_app);
}

void MainWindow::indexCalled(Cutelyst::Context *c)
{
    qDebug() << "indexCalled" << ui->serverResponsePTE->toPlainText().toUtf8().size();
    // Request
    ui->serverMethodLE->setText(c->request()->method());
    ui->serverPathLE->setText(c->request()->path());
    if (c->request()->body()) {
        ui->serverBodyPTE->setPlainText(QString::fromUtf8(c->request()->body()->readAll()));
    } else {
        ui->serverBodyPTE->clear();
    }

    m_serverReceivedHeaders->clear();
    const auto headersData = c->request()->headers().data();
    auto hIt               = headersData.begin();
    while (hIt != headersData.end()) {
        auto keyItem   = new QStandardItem(hIt.key());
        auto valueItem = new QStandardItem(hIt.value());
        m_serverReceivedHeaders->appendRow({
            keyItem,
            valueItem,
        });
        ++hIt;
    }

    // Response
    c->response()->setStatus(ui->serverStatusSB->value());
    c->response()->setBody(ui->serverResponsePTE->toPlainText().toUtf8());
}

void MainWindow::updateUrl()
{
    const QString url = QLatin1String("http://localhost:") + QString::number(ui->serverPortSB->value());
    ui->serverL->setText(tr("Server: <a href=\"%1\">%1</a>").arg(url));
}
