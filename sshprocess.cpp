/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "x2goclientconfig.h"
#include "x2gologdebug.h"
#include "sshmasterconnection.h"
#include "sshprocess.h"
#include <QTimer>

#ifndef Q_OS_WIN
#include <arpa/inet.h>
#include <netinet/tcp.h>
#endif

#undef DEBUG
// #define DEBUG

SshProcess::SshProcess(SshMasterConnection* master, QObject* parent): QObject(parent)
{
    masterCon=master;
    serverSocket=0;
    connect(master,SIGNAL(stdErr(SshProcess*,QByteArray)),this,SLOT(slotStdErr(SshProcess*,QByteArray)));
    connect(master,SIGNAL(ioErr(SshProcess*,QString,QString)),this,SLOT(slotIOerr(SshProcess*,QString,QString)));
    tunnel=false;
    normalExited=true;
}

SshProcess::~SshProcess()
{
#ifdef DEBUG
    x2goDebug<<"ssh process destructor";
#endif
    if (serverSocket>0)
    {
#ifdef Q_OS_WIN
        closesocket(serverSocket);
        WSACleanup();

#else
        close(serverSocket);
#endif
    }
}


void SshProcess::slotCheckNewConnection()
{
    fd_set rfds;
    struct timeval tv;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&rfds);
    FD_SET(serverSocket, &rfds);

    if (select(serverSocket+1,&rfds,NULL,NULL,&tv)<=0)
        return;

#ifdef DEBUG
    x2goDebug<<"new tcp connection\n";
#endif
    int tcpSocket=accept(serverSocket, (struct sockaddr*)&address,&addrlen);

#ifdef DEBUG
    x2goDebug<<"new socket:"<<tcpSocket<<endl;
#endif
    masterCon->addChannelConnection(this, tcpSocket, forwardHost, forwardPort, localHost,
                                    ntohs(address.sin_port));
}


void SshProcess::tunnelLoop()
{

    serverSocket=socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket<=0)
    {
        QString err=tr("Error creating socket");
        x2goDebug<<err<<endl;
        emit sshFinished(false,err,this);
        return;
    }
#ifndef Q_OS_WIN
    const int y=1;
#else
    const char y=1;
#endif
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR,&y, sizeof(int));
    setsockopt(serverSocket, IPPROTO_TCP, TCP_NODELAY,&y, sizeof(int));

    address.sin_family=AF_INET;
    address.sin_addr.s_addr=INADDR_ANY;
    address.sin_port=htons(localPort);
    if (bind(serverSocket,(struct sockaddr*) &address,sizeof(address))!=0)
    {
        QString err=tr("Error binding ")+localHost+":"+QString::number(localPort);
        x2goDebug<<err<<endl;
        emit sshFinished(false,err,this);
        return;
    }
    listen(serverSocket,5);
    addrlen=sizeof(struct sockaddr_in);
    QTimer* timer=new QTimer();
    connect(timer,SIGNAL(timeout()),this,SLOT(slotCheckNewConnection()));
    timer->start(500);
    emit sshTunnelOk();
#ifdef DEBUG
    x2goDebug<<"Direct tunnel: waiting for connections on "<<localHost<<":"<<localPort<<endl;
#endif
}

void SshProcess::startNormal(const QString& cmd)
{
    masterCon->addChannelConnection(this, cmd);
    connect(masterCon,SIGNAL(stdOut(SshProcess*,QByteArray)),this,SLOT(slotStdOut(SshProcess*,QByteArray)));
    connect(masterCon,SIGNAL(channelClosed(SshProcess*)), this,SLOT(slotChannelClosed(SshProcess*)));
}

void SshProcess::start_cp(QString src, QString dst)
{
    connect(masterCon, SIGNAL(copyErr(SshProcess*,QString,QString)), this,
            SLOT(slotCopyErr(SshProcess*,QString,QString)));
    connect(masterCon, SIGNAL(copyOk(SshProcess*)), this,SLOT(slotCopyOk(SshProcess*)));
    scpSource=src;
    masterCon->addCopyRequest(this,src,dst);
}


void SshProcess::startTunnel(const QString& forwardHost, uint forwardPort, const QString& localHost,
                             uint localPort, bool reverse)
{
    this->forwardHost=forwardHost;
    this->forwardPort=forwardPort;
    this->localHost=localHost;
    this->localPort=localPort;
    tunnel=true;
    if (!reverse)
        tunnelLoop();
    else
    {
        connect(masterCon, SIGNAL(reverseListenOk(SshProcess*)), this, SLOT(slotReverseTunnelOk(SshProcess*)));
        tunnelConnection=masterCon->reverseTunnelConnection(this, forwardPort, localHost, localPort);
    }
}


void SshProcess::slotStdErr(SshProcess* creator, QByteArray data)
{
    if (creator!=this)
        return;
#ifdef DEBUG
    x2goDebug<<"new err data:"<<data<<endl;
#endif
    stdErrString+=data;
}

void SshProcess::slotStdOut(SshProcess* creator, QByteArray data)
{
    if (creator!=this)
        return;
//     x2goDebug<<"new data"<<data<<endl;
    stdOutString+=data;
}

void SshProcess::slotIOerr(SshProcess* creator, QString message, QString sshSessionErr)
{
    if (creator!=this)
        return;
#ifdef DEBUG
    x2goDebug<<"io error:"<<message<<" - "<<sshSessionErr<<endl;
#endif
    normalExited=false;
    abortString=message+" - "+sshSessionErr;
}

void SshProcess::slotCopyErr(SshProcess* creator, QString message, QString sshSessionErr)
{
    if (creator!=this)
        return;
    emit sshFinished(false, message+" - "+sshSessionErr, this);
}

void SshProcess::slotCopyOk(SshProcess* creator)
{
    if (creator!=this)
        return;
    emit sshFinished(true,"", this);
}

void SshProcess::slotReverseTunnelOk(SshProcess* creator)
{
    if (creator==this)
        emit sshTunnelOk();
}


void SshProcess::slotChannelClosed(SshProcess* creator)
{
    if (creator!=this)
        return;
    QString output;
    if (!normalExited)
    {
        output=abortString;
    }
    else
    {
        if ( stdOutString.length()<=0 &&  stdErrString.length() >0 )
        {
            normalExited=false;
            output=stdErrString;
#ifdef DEBUG
            x2goDebug<<"have only stderr, something must be wrong"<<endl;
#endif
        }
        else
            output=stdOutString;
    }
#ifdef DEBUG
    x2goDebug<<"ssh finished:"<<normalExited<<" - "<<output<<endl;
#endif
    emit sshFinished(normalExited, output, this);
}
