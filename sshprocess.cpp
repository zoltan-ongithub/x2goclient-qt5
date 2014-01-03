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

#include "x2goclientconfig.h"
#include "x2gologdebug.h"
#include "sshmasterconnection.h"
#include "sshprocess.h"
#include <QTimer>
#include <QUuid>

#include <QProcess>
#ifndef Q_OS_WIN
#include <arpa/inet.h>
#include <netinet/tcp.h>
#endif

#undef DEBUG
// #define DEBUG

#ifdef Q_OS_DARWIN
#define KEEPALIVE_OPTION " -o ServerAliveInterval=60 "
#else
#define KEEPALIVE_OPTION " -o ProtocolKeepAlives=60 "
#endif

SshProcess::SshProcess(SshMasterConnection* master, int pid): QObject(0)
{
    masterCon=master;
    serverSocket=0;
    connect(master,SIGNAL(stdErr(SshProcess*,QByteArray)),this,SLOT(slotStdErr(SshProcess*,QByteArray)));
    connect(master,SIGNAL(ioErr(SshProcess*,QString,QString)),this,SLOT(slotIOerr(SshProcess*,QString,QString)));
    tunnel=false;
    normalExited=true;
    this->pid=pid;
    proc=0l;
    execProcess=false;
}

SshProcess::~SshProcess()
{
#ifdef DEBUG
    x2goDebug<<"ssh process destructor";
#endif

    if (proc)
    {
        if (tunnel)
        {
            disconnect(proc,SIGNAL(finished(int,QProcess::ExitStatus)),this,
                       SLOT(slotSshProcFinished(int,QProcess::ExitStatus)));
            disconnect(proc,SIGNAL(readyReadStandardError()),this,SLOT(slotSshProcStdErr()));
            disconnect(proc,SIGNAL(readyReadStandardOutput()),this,SLOT(slotSshProcStdOut()));
        }
        if (proc->state()==QProcess::Running && execProcess)
        {
            if(!proc->waitForFinished(3000))
            {
                proc->terminate();
            }
        }
        if (proc->state()==QProcess::Running)
        {
            proc->kill();
        }
        if(proc->state()!=QProcess::Running)
        {
            delete proc;
        }
        proc=0;
    }
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
        emit sshFinished(false,err,pid);
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
        emit sshFinished(false,err,pid);
        return;
    }
    listen(serverSocket,5);
    addrlen=sizeof(struct sockaddr_in);
    QTimer* timer=new QTimer();
    connect(timer,SIGNAL(timeout()),this,SLOT(slotCheckNewConnection()));
    timer->start(100);
    emit sshTunnelOk(pid);
#ifdef DEBUG
    x2goDebug<<"Direct tunnel: waiting for connections on "<<localHost<<":"<<localPort<<endl;
#endif
}

#ifdef Q_OS_WIN
#include <QSettings>
void SshProcess::addPuttyReg(QString host, QString uuidStr)
{
    QSettings st("HKEY_CURRENT_USER\\Software\\SimonTatham\\PuTTY\\Sessions\\"+uuidStr,
                 QSettings::NativeFormat);
    st.setValue("HostName", host);
    st.setValue("GssapiFwd", (uint) 1);
    st.sync();
}

void SshProcess::rmPuttyReg(QString uuidStr)
{
    QSettings st("HKEY_CURRENT_USER\\Software\\SimonTatham\\PuTTY\\Sessions",
                 QSettings::NativeFormat);
    st.remove(uuidStr);
    st.sync();
}
#endif

void SshProcess::startNormal(const QString& cmd)
{
    QUuid uuid = QUuid::createUuid();
    QString uuidStr = uuid.toString().mid(1, 36).toLower();
    execProcess=true;

//#ifdef DEBUG
// ONLY UNCOMMENT FOR TESTING, MIGHT REVEAL PASSWORD WHEN command=RDP
//    x2goDebug<<"executing remote command: "<<shcmd<<endl;
// #endif
    if(!masterCon->useKerberos())
    {
        QString shcmd = "sh -c \"echo X2GODATABEGIN:" + uuidStr + "; "+cmd+"; echo X2GODATAEND:" + uuidStr +"\";";
        masterCon->addChannelConnection(this, uuidStr, shcmd);
        connect(masterCon,SIGNAL(stdOut(SshProcess*,QByteArray)),this,SLOT(slotStdOut(SshProcess*,QByteArray)));
        connect(masterCon,SIGNAL(channelClosed(SshProcess*,QString)), this,SLOT(slotChannelClosed(SshProcess*,QString)));
    }
    else
    {
        QString host=masterCon->getHost();
        QString shcmd = "echo X2GODATABEGIN:" + uuidStr + "; "+cmd+"; echo X2GODATAEND:" + uuidStr;
        proc=new QProcess(this);
#ifdef Q_OS_WIN
        addPuttyReg(host, uuidStr);
        host = uuidStr;
        QString sshString="plink -batch -P "+
#else
        QString sshString=QString::null+"ssh"+ KEEPALIVE_OPTION +"-K -o GSSApiAuthentication=yes -o PasswordAuthentication=no -p "+
#endif
                          QString::number(masterCon->getPort())+" -l "+
                          masterCon->getUser()+" "+ host +  " \""+shcmd+"\"";
#ifdef DEBUG
        x2goDebug<<"running ssh:" <<sshString<<endl;
#endif
        procUuid=uuidStr;
        proc->start(sshString);

        if (!proc->waitForStarted(5000))
        {
            stdErrString=proc->errorString();
#ifdef DEBUG
            x2goDebug<<"ssh start failed:" <<stdErrString<<endl;
#endif
            slotChannelClosed(this, uuidStr);
            return;
        }
        connect(proc,SIGNAL(finished(int,QProcess::ExitStatus)),this,
                SLOT(slotSshProcFinished(int,QProcess::ExitStatus)));
        connect(proc,SIGNAL(readyReadStandardError()),this,SLOT(slotSshProcStdErr()));
        connect(proc,SIGNAL(readyReadStandardOutput()),this,SLOT(slotSshProcStdOut()));
    }

}

void SshProcess::start_cp(QString src, QString dst)
{
    scpSource=src;
    if(!masterCon->useKerberos())
    {
        connect(masterCon, SIGNAL(copyErr(SshProcess*,QString,QString)), this,
                SLOT(slotCopyErr(SshProcess*,QString,QString)));
        connect(masterCon, SIGNAL(copyOk(SshProcess*)), this,SLOT(slotCopyOk(SshProcess*)));
        masterCon->addCopyRequest(this,src,dst);
    }
    else
    {
        proc=new QProcess(this);
#ifdef Q_OS_WIN
//pscp don't working with paths like "~user"
//I hope a home directories of your users are in /home/
        dst.replace("~"+masterCon->getUser(),"/home/"+masterCon->getUser());
        dst.replace("~","/home/"+masterCon->getUser());

        QString sshString="pscp -batch -P "+
#else
        QString sshString="scp -o GSSApiAuthentication=yes -o PasswordAuthentication=no -P "+
#endif
                          QString::number(masterCon->getPort())+" "+src+" "+
                          masterCon->getUser()+"@"+ masterCon->getHost()+":"+dst;
#ifdef DEBUG
        x2goDebug<<"running scp:" <<sshString<<endl;
#endif
        proc->start(sshString);

        if (!proc->waitForStarted(5000))
        {
            stdErrString=proc->errorString();
#ifdef DEBUG
            x2goDebug<<"ssh start failed:" <<stdErrString<<endl;
#endif
            slotChannelClosed(this,"");
            return;
        }
        connect(proc,SIGNAL(finished(int,QProcess::ExitStatus)),this,
                SLOT(slotSshProcFinished(int,QProcess::ExitStatus)));
        connect(proc,SIGNAL(readyReadStandardError()),this,SLOT(slotSshProcStdErr()));
        connect(proc,SIGNAL(readyReadStandardOutput()),this,SLOT(slotSshProcStdOut()));
    }
}


void SshProcess::startTunnel(const QString& forwardHost, uint forwardPort, const QString& localHost,
                             uint localPort, bool reverse)
{
    tunnel=true;
    tunnelOkEmited=false;
    if(!masterCon->useKerberos())
    {
        this->forwardHost=forwardHost;
        this->forwardPort=forwardPort;
        this->localHost=localHost;
        this->localPort=localPort;
        if (!reverse)
            tunnelLoop();
        else
        {
            connect(masterCon, SIGNAL(reverseListenOk(SshProcess*)), this, SLOT(slotReverseTunnelOk(SshProcess*)));
            tunnelConnection=masterCon->reverseTunnelConnection(this, forwardPort, localHost, localPort);
        }
    }
    else
    {
        proc=new QProcess(0);
#ifdef Q_OS_WIN
        QString sshString="plink -batch -P "+
#else
        QString sshString=QString::null+"ssh"+ KEEPALIVE_OPTION +"-o GSSApiAuthentication=yes -o PasswordAuthentication=no -p "+
#endif
                          QString::number(masterCon->getPort())+" "+
                          masterCon->getUser()+"@"+
                          masterCon->getHost() + " -N -v ";
        if (!reverse)
            sshString+=" -L " + QString::number(localPort)+":"+forwardHost+":"+QString::number(forwardPort);
        else
            sshString+=" -R "+ QString::number(forwardPort)+":"+forwardHost+":"+QString::number(localPort);

#ifdef DEBUG
        x2goDebug<<"running ssh:" <<sshString<<endl;
#endif
        proc->start(sshString);

        if (!proc->waitForStarted(5000))
        {
            stdErrString=proc->errorString();
#ifdef DEBUG
            x2goDebug<<"ssh start failed:" <<stdErrString<<endl;
#endif
            slotChannelClosed(this,"");
            return;
        }
        connect(proc,SIGNAL(finished(int,QProcess::ExitStatus)),this,
                SLOT(slotSshProcFinished(int,QProcess::ExitStatus)));
        connect(proc,SIGNAL(readyReadStandardError()),this,SLOT(slotSshProcStdErr()));
        connect(proc,SIGNAL(readyReadStandardOutput()),this,SLOT(slotSshProcStdOut()));
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

    if(tunnel && !tunnelOkEmited)
    {
#ifdef Q_OS_WIN
        if(stdErrString.indexOf("Access granted")!=-1)
#else
        if(stdErrString.indexOf("Entering interactive session")!=-1)
#endif
        {
            tunnelOkEmited=true;
            emit sshTunnelOk(pid);
        }
    }
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
    emit sshFinished(false, message+" - "+sshSessionErr, pid);
}

void SshProcess::slotCopyOk(SshProcess* creator)
{
    if (creator!=this)
        return;
    emit sshFinished(true,"", pid);
}

void SshProcess::slotReverseTunnelOk(SshProcess* creator)
{
    if (creator==this)
        emit sshTunnelOk(pid);
}


void SshProcess::slotChannelClosed(SshProcess* creator, QString uuid)
{
    if (creator!=this)
        return;
    QString output;
    if (!normalExited)
    {
        output=abortString;
        if (output.length()<5)
        {
            output=stdErrString;
        }
    }
    else
    {
        QString begin_marker = "X2GODATABEGIN:"+uuid+"\n";
        QString end_marker = "X2GODATAEND:"+uuid+"\n";
        int output_begin=stdOutString.indexOf(begin_marker) + begin_marker.length();
        int output_end=stdOutString.indexOf(end_marker);
        output = stdOutString.mid(output_begin, output_end-output_begin);
        if ( output.length()<=0 &&  stdErrString.length() >0 )
        {
            normalExited=false;
            output=stdErrString;
#ifdef DEBUG
            x2goDebug<<"have only stderr, something must be wrong"<<endl;
#endif
        }
    }
#ifdef DEBUG
    x2goDebug<<"ssh finished:"<<normalExited<<" - "<<output<<uuid<<endl;
#endif
    emit sshFinished(normalExited, output, pid);
}

void SshProcess::slotSshProcFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    normalExited=false;
    if (exitCode==0 && exitStatus==QProcess::NormalExit)
        normalExited=true;
#ifdef DEBUG
    x2goDebug<<"ssh process exit code :"<<exitStatus;
#endif
#ifdef Q_OS_WIN
    if(masterCon->useKerberos())
    {
        rmPuttyReg(procUuid);
    }
#endif
    slotChannelClosed(this,procUuid);
}

void SshProcess::slotSshProcStdErr()
{
    slotStdErr(this, proc->readAllStandardError());
}

void SshProcess::slotSshProcStdOut()
{
    slotStdOut(this, proc->readAllStandardOutput());
}
