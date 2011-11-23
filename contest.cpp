/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Oleksandr Shneyder

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "contest.h"
#include "x2gologdebug.h"
#include <QTimer>
#include "httpbrokerclient.h"
#include <QPushButton>

ConTest::ConTest(HttpBrokerClient* broker, QUrl url, QWidget* parent, Qt::WindowFlags f): QDialog(parent, f)
{
    socket=0l;
    setupUi(this);
    this->broker=broker;
    brokerUrl=url;
    timer=new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(slotTimer()));
    connect(broker,SIGNAL(connectionTime(int,int)),this, SLOT(slotConSpeed(int,int)));
    start();
}

ConTest::~ConTest()
{

}

void ConTest::resetSocket()
{
    if (socket)
    {
        socket->disconnectFromHost();
        socket->close();
        delete socket;
        socket=0l;
    }
}


void ConTest::reset()
{
    timer->stop();
    lhttps->setText("");
    lssh->setText("");
    lspeed->setText("");
    prhttps->setValue(0);
    prspeed->setValue(0);
    prssh->setValue(0);
    httpsOk=false;
    resetSocket();
    buttonBox->button(QDialogButtonBox::Retry)->setEnabled(false);
}

void ConTest::start()
{
    reset();
    testConnection(HTTPS);
}

void ConTest::testConnection(tests test)
{
    time=0;
    timer->start(100);
    resetSocket();
    currentTest=test;
    if (test==SPEED)
    {
        if (!httpsOk)
        {
            slotConSpeed(1,0);
            return;
        }
        broker->testConnection();
        return;
    }
    socket=new QTcpSocket(this);
    socket->connectToHost(brokerUrl.host(),test);
    connect( socket,SIGNAL(connected()),this,SLOT(slotConnected()));
    connect( socket, SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(slotError(QAbstractSocket::SocketError)));
}


void ConTest::slotConnected()
{
    x2goDebug<<"connected\n";
    timer->stop();
    QPalette pal=lhttps->palette();
    pal.setColor(QPalette::WindowText, Qt::green);
    switch (currentTest)
    {
    case HTTPS:
        prhttps->setValue(100);
        lhttps->setText(tr("OK"));
        lhttps->setPalette(pal);
        httpsOk=true;
        testConnection(SSH);
        break;
    case SSH:
        prssh->setValue(100);
        lssh->setText(tr("OK"));
        lssh->setPalette(pal);
        testConnection(SPEED);
        break;
    default:
        break;
    }
}

void ConTest::slotConSpeed(int msecElapsed, int bytesRecived)
{
    timer->stop();
    prspeed->setValue(100);
    double sec=msecElapsed/1000.;
    int KB=bytesRecived/1024;
    int Kbsec=(int)(KB/sec)*8;

    QPalette pal=lspeed->palette();
    pal.setColor(QPalette::WindowText, Qt::green);

    if (Kbsec<1000)
        pal.setColor(QPalette::WindowText, Qt::yellow);
    if (Kbsec<512)
        pal.setColor(QPalette::WindowText, Qt::red);

    lspeed->setPalette(pal);
    lspeed->setText(QString::number(Kbsec)+" Kb/s");
    buttonBox->button(QDialogButtonBox::Retry)->setEnabled(true);
}


void ConTest::slotError(QAbstractSocket::SocketError socketError)
{
    QString error;
    if (socketError==QAbstractSocket::SocketTimeoutError)
        error=tr("Socket operation timed out");
    else
        error=socket->errorString();

    x2goDebug<<"Error: "<<error<<endl;
    timer->stop();
    QPalette pal=lhttps->palette();
    pal.setColor(QPalette::WindowText, Qt::red);
    switch (currentTest)
    {
    case HTTPS:
        prhttps->setValue(100);
        lhttps->setText(tr("Failed: ")+error);
        lhttps->setPalette(pal);
        testConnection(SSH);
        break;
    case SSH:
        prssh->setValue(100);
        lssh->setText(tr("Failed: ")+error);
        lssh->setPalette(pal);
        testConnection(SPEED);
        break;
    default:
        break;
    }
}


void ConTest::slotTimer()
{
    time++;
    if (time>150)
    {
        if (currentTest==SSH || currentTest==HTTPS)
        {
            socket->close();
            slotError(QAbstractSocket::SocketTimeoutError);
        }
    }
    QProgressBar* bar=0l;
    switch (currentTest)
    {
    case SSH:
        bar=prssh;
        break;
    case HTTPS:
        bar=prhttps;
        break;
    case SPEED:
        bar=prspeed;
        break;
    }
    if (bar->value()==100)
        bar->setValue(0);
    else
        bar->setValue(bar->value()+10);

}
