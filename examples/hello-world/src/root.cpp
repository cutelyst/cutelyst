/***************************************************************************
 *   Copyright (C) 2014 Daniel Nicoletti <dantti12@gmail.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#include "root.h"

#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Session/Session>

#include <QDebug>

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

Root::Root(QObject *app) : Controller(app)
{
}

Root::~Root()
{
}

void Root::hello(Context *c)
{
    c->response()->setBody(QByteArrayLiteral("Hello World! \n"));
}

void Root::json(Context *c)
{
    c->res()->setJsonObjectBody({
                                    {QStringLiteral("message"), QStringLiteral("Hello, World!")}
                                });
}

void Root::echo(Context *c)
{
  QUrl websocket_url = c->uriFor(actionFor(QStringLiteral("ws"))).toString();
    websocket_url.setScheme(QStringLiteral("ws"));
    c->response()->setBody(
                QStringLiteral("<!DOCTYPE html>\n"
                               "<html lang=\"en\">\n"
                               "<head>\n"
                               "  <title>WebSocket Echo Client</title>\n"
                               "  <meta charset=\"UTF-8\" />\n"
                               "  <script>\n"
                               "    \"use strict\";\n"
                               "    window.addEventListener(\"load\", function(event) {\n"
                               "      var status = document.getElementById(\"status\");\n"
                               "      var url = document.getElementById(\"url\");\n"
                               "      var open = document.getElementById(\"open\");\n"
                               "      var close = document.getElementById(\"close\");\n"
                               "      var send = document.getElementById(\"send\");\n"
                               "      var text = document.getElementById(\"text\");\n"
                               "      var message = document.getElementById(\"message\");\n"
                               "      var socket;\n"
                               " \n"
                               "      status.textContent = \"Not Connected\";\n"
                               "      url.value = \"") + websocket_url.toString() +
                QStringLiteral("\";\n"
                               "      close.disabled = true;\n"
                               "      send.disabled = true;\n"
                               " \n"
                               "      // Create a new connection when the Connect button is clicked\n"
                               "      open.addEventListener(\"click\", function(event) {\n"
                               "        open.disabled = true;\n"
                               "        socket = new WebSocket(url.value);\n"
                               " \n"
                               "        socket.addEventListener(\"open\", function(event) {\n"
                               "          close.disabled = false;\n"
                               "          send.disabled = false;\n"
                               "          status.textContent = \"Connected\";\n"
                               "        });\n"
                               " \n"
                               "        // Display messages received from the server\n"
                               "        socket.addEventListener(\"message\", function(event) {\n"
                               "          message.textContent = \"Server Says: \" + event.data;\n"
                               "        });\n"
                               " \n"
                               "        // Display any errors that occur\n"
                               "        socket.addEventListener(\"error\", function(event) {\n"
                               "          message.textContent = \"Error: \" + event;\n"
                               "          console.log(\"my object: %o\", event);\n"
                               "        });\n"
                               " \n"
                               "        socket.addEventListener(\"close\", function(event) {\n"
                               "          open.disabled = false;\n"
                               "          close.disabled = true;\n"
                               "          send.disabled = true;\n"
                               "          status.textContent = \"Not Connected\"\n;"
                               "          message.textContent = \"\";\n"
                               "        });\n"
                               "      });\n"
                               " \n"
                               "      // Close the connection when the Disconnect button is clicked\n"
                               "      close.addEventListener(\"click\", function(event) {\n"
                               "        open.disabled = false;\n"
                               "        close.disabled = true;\n"
                               "        send.disabled = true;\n"
                               "        message.textContent = \"\";\n"
                               "        socket.close();\n"
                               "      });\n"
                               " \n"
                               "      // Send text to the server when the Send button is clicked\n"
                               "      send.addEventListener(\"click\", function(event) {\n"
                               "        socket.send(text.value);\n"
                               "        text.value = \"\";\n"
                               "      });\n"
                               "    });\n"
                               "  </script>\n"
                               "</head>\n"
                               "<body>\n"
                               "  Status: <span id=\"status\"></span><br />\n"
                               "  URL: <input id=\"url\" /><br />\n"
                               "  <input id=\"open\" type=\"button\" value=\"Connect\" />&nbsp;\n"
                               "  <input id=\"close\" type=\"button\" value=\"Disconnect\" /><br />\n"
                               "  <input id=\"send\" type=\"button\" value=\"Send\" />&nbsp;\n"
                               "  <textarea rows=\"4\" cols=\"50\" id=\"text\"></textarea><br />\n"
                               "  <span id=\"message\"></span>\n"
                               "</body>\n"
                               "</html>\n"));
}

void Root::ws(Context *c)
{
    Response *response = c->response();

    if (response->webSocketHandshake()) {
        Request *req = c->req();
        connect(req, &Request::webSocketTextFrame, [=] (const QString &msg, bool isLastFrame) {
//            qDebug() << "Got text frame" << isLastFrame << msg.size() << msg.left(25) << c->actionName();
//            response->webSocketTextMessage(msg);
        });

        connect(req, &Request::webSocketTextMessage, [=] (const QString &msg) {
//            qDebug() << "Got text msg" << msg.size() << msg.left(25) << c->actionName();
            response->webSocketTextMessage(msg);
        });

        connect(req, &Request::webSocketBinaryFrame, [=] (const QByteArray &msg, bool isLastFrame) {
//            qDebug() << "Got binary frame" << isLastFrame << msg.size() << msg.left(25) << c->actionName();
//            response->webSocketBinaryMessage(msg);
        });

        connect(req, &Request::webSocketBinaryMessage, [=] (const QByteArray &msg) {
//            qDebug() << "Got binary msg" << msg.size() << msg.left(25) << c->actionName();
            response->webSocketBinaryMessage(msg);
        });

        connect(req, &Request::webSocketPong, [=] (const QByteArray &payload) {
            qDebug() << "Got pong" << payload;
        });

        connect(req, &Request::webSocketClosed, [=] (quint16 code, const QString &reason) {
            qDebug() << "Got close" << code << reason;
        });
    }
}

void Root::session(Context *c)
{
    QString foo = Session::value(c, QStringLiteral("foo")).toString();

    c->response()->setBody(QLatin1String("Foo: ") + foo + QLatin1Char('\n'));

    Session::setValue(c, QStringLiteral("foo"), QStringLiteral("bar"));
}

void Root::read_session(Context *c)
{
    QString foo = Session::value(c, QStringLiteral("foo")).toString();
    c->response()->setBody(QLatin1String("Foo: ") + foo + QLatin1Char('\n'));
}
