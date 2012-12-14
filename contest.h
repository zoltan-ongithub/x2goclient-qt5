/**************************************************************************
*   Copyright (C) 2005-2012 by Oleksandr Shneyder                         *
*   o.shneyder@phoca-gmbh.de                                              *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
***************************************************************************/

#ifndef CONTEST_H
#define CONTEST_H

#include <QDialog>
#include "ui_contest.h"
#include <QUrl>
#include <QTcpSocket>

class HttpBrokerClient;
class QTcpSocket;
class QTimer;

class ConTest : public QDialog, public Ui_ConTest
{
    Q_OBJECT
public:
    ConTest(HttpBrokerClient* broker, QUrl url, QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~ConTest();
private:
    enum tests {SSH=22,HTTPS=443,SPEED} currentTest;
    void reset();
    void testConnection(tests test);
    void resetSocket();
private slots:
    void slotConnected();
    void slotError(QAbstractSocket::SocketError socketError);
    void slotTimer();
    void slotConSpeed(int msecElapsed, int bytesRecived);
private:
    HttpBrokerClient* broker;
    QUrl brokerUrl;
    QTcpSocket* socket;
    QTimer* timer;
    int time;
    bool httpsOk;
public slots:
    void start();
};

#endif // CONTEST_H
