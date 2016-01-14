/**************************************************************************
*   Copyright (C) 2005-2015 by Oleksandr Shneyder                         *
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

// #undef DEBUG
#define DEBUG

#define KEEPALIVE_OPTION " -o ServerAliveInterval=60 "

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
    x2goDebug<<"SshProcess destructor called.";
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
    x2goDebug<<"New TCP connection.";
#endif
    int tcpSocket=accept(serverSocket, (struct sockaddr*)&address,&addrlen);

#ifdef DEBUG
    x2goDebug<<"New socket: "<<tcpSocket;
#endif
    masterCon->addChannelConnection(this, tcpSocket, forwardHost, forwardPort, localHost,
                                    ntohs(address.sin_port));
}


void SshProcess::tunnelLoop()
{

    serverSocket=socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket<=0)
    {
        QString err=tr("Error creating socket.");
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
    x2goDebug<<"Direct tunnel: waiting for connections on "<<localHost<<":"<<localPort;
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
    if (uuidStr.isEmpty())
    {
#ifdef DEBUG
        x2goDebug<<"uuidStr is empty. No PuTTY session reg key to delete.";
#endif
        return;
    }
#ifdef DEBUG
    x2goDebug<<"Deleting key in registry: HKEY_CURRENT_USER\\Software\\SimonTatham\\PuTTY\\Sessions\\"+uuidStr;
#endif
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
    x2goDebug<<"Executing remote command via SshProcess object "<<pid<<": "<<cmd;
// #endif
    if(!masterCon->useKerberos())
    {
        QString shcmd = "bash -l -c 'echo \"X2GODATABEGIN:" + uuidStr + "\"; export PATH=\"/usr/local/bin:/usr/bin:/bin\"; export TERM=\"dumb\"; "+cmd+"; echo \"X2GODATAEND:" + uuidStr + "\";'";
        x2goDebug << "this="<<this<<" Running masterCon->addChannelConnection(this, '" << uuidStr << "', '" << shcmd.left (200) << "');";
        masterCon->addChannelConnection(this, uuidStr, shcmd);
        connect(masterCon,SIGNAL(stdOut(SshProcess*,QByteArray)),this,SLOT(slotStdOut(SshProcess*,QByteArray)));
        connect(masterCon,SIGNAL(channelClosed(SshProcess*,QString)), this,SLOT(slotChannelClosed(SshProcess*,QString)));
    }
    else
    {
        QString host=masterCon->getHost();

        QString shcmd = "";

        /* On Windows, arguments are automatically wrapped in double quotes.
         * Additionally, QProcess automatically doubles escape characters before
         * double quotes and inserts an escape character before any non-escaped
         * double quotes.
         * Thus, we don't escape double quotes here and let Qt handle this stuff.
         *
         * On UNIX-like platforms, likewise, we MUST NOT escape double quotes,
         * as there is no preceding "outer double quote" the whole argument
         * is wrapped in.
         */
        shcmd = "bash -l -c 'echo \"X2GODATABEGIN:" + uuidStr + "\"; export PATH=\"/usr/local/bin:/usr/bin:/bin\"; export TERM=\"dumb\"; "+cmd+"; echo \"X2GODATAEND:" + uuidStr + "\";'";

        proc=new QProcess(this);
        QString local_cmd = "";
        QStringList local_args;
#ifdef Q_OS_WIN
        if(masterCon->get_kerberosDelegation())
        {
            addPuttyReg(host, uuidStr);
            host = uuidStr;
        }
        local_cmd = "plink";

        /* General options. */
        local_args << "-batch";

        /* Port option. Must be the last one added! */
        local_args << "-P";
#else
        local_cmd = "ssh";

        /* General options. */
        local_args << QString (KEEPALIVE_OPTION).trimmed ().split (" ");

        /* Kerberos options. */
        local_args << "-k";
        if(masterCon->get_kerberosDelegation())
        {
            local_args << "-K";
        }

        /* Authentication options. */
        local_args << "-o" << "GSSApiAuthentication=yes"
                   << "-o" << "PasswordAuthentication=no"
                   << "-o" << "PubkeyAuthentication=no";

        /* Port option. Must be the last one added! */
        local_args << "-p";
#endif
        local_args << QString::number (masterCon->getPort ())
                   << "-l" << masterCon->getUser ()
                   << host;

        /* On Windows, arguments are automatically wrapped in double quotes.
         * This means we do not have to wrap shcmd ourselves.
         *
         * On UNIX-like platforms, we likewise MUST NOT wrap the command in
         * double quotes, as each entry in the arguments list is passed as
         * one entry in argv.
         */
        local_args << shcmd;

        x2goDebug << "Invoking SSH command via SshProcess object " << pid << ": "
                  << local_cmd << " " << local_args.join (" ");
        procUuid=uuidStr;
        proc->start (local_cmd, local_args);

        if (!proc->waitForStarted(15000))
        {
            stdErrString=proc->errorString();
#ifdef DEBUG
            //x2goDebug<<"ssh start failed:" <<stdErrString<<endl;
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
    x2goDebug<<"Copying file via SshProcess object "<<pid<<": "<<src<<" -> "<<dst;

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
//pscp doesn't acccept paths like the following when in SFTP mode (default)
//~user/foo.txt
//~/foo.txt
//${HOME}/foo.txt
//$HOME/foo.txt
//
//However, pscp does let you specify a path relative to the user's home dir.
//You simply specify the relative path without a / at the beginning.
//For example:
//foo.txt
//
//This workaround assumes that files will never be uploaded to a home dir
//other than the user's.

        dst.remove("~"+masterCon->getUser()+"/");
        dst.remove("~"+masterCon->getUser()    );

        dst.remove("~/");
        dst.remove("~" );

        dst.remove("${HOME}/");
        dst.remove("${HOME}");

        dst.remove("$HOME/");
        dst.remove("$HOME");

        QString sshString="pscp -batch -P "+
#else
        QString sshString="scp -o GSSApiAuthentication=yes -o PasswordAuthentication=no -o PubkeyAuthentication=no -P "+
#endif
                          QString::number(masterCon->getPort())+" "+src+" "+
                          masterCon->getUser()+"@"+ masterCon->getHost()+":"+dst;
#ifdef DEBUG
        x2goDebug<<"Running scp:" <<sshString;
#endif
        proc->start(sshString);

        if (!proc->waitForStarted(15000))
        {
            stdErrString=proc->errorString();
#ifdef DEBUG
            x2goDebug<<"SSH start failed:" <<stdErrString;
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
    x2goDebug<<"Starting tunnel via SshProcess object "<<pid<<": "<<forwardHost<<":"<<forwardPort<<" -> "<<localHost<<":"<<localPort<<endl;

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
    }
    else
    {
        proc=new QProcess(0);
#ifdef Q_OS_WIN
        QString sshString="plink -batch -P "+
#else
        QString sshString=QString::null+"ssh"+ KEEPALIVE_OPTION +"-o GSSApiAuthentication=yes -o PasswordAuthentication=no -o PubkeyAuthentication=no -p "+
#endif
                          QString::number(masterCon->getPort())+" "+
                          masterCon->getUser()+"@"+
                          masterCon->getHost() + " -N -v ";
        if (!reverse)
            sshString+=" -L " + QString::number(localPort)+":"+forwardHost+":"+QString::number(forwardPort);
        else
            sshString+=" -R "+ QString::number(forwardPort)+":"+forwardHost+":"+QString::number(localPort);

#ifdef DEBUG
        x2goDebug<<"Tunnel: running ssh:" <<sshString;
#endif
        proc->start(sshString);

        if (!proc->waitForStarted(5000))
        {
            stdErrString=proc->errorString();
#ifdef DEBUG
            x2goDebug<<"SSH start failed:" <<stdErrString;
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
//     x2goDebug<<"new err data:"<<data<<endl;
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
#ifdef DEBUG
            x2goDebug<<"Tunnel OK";
#endif
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
    if (sshSessionErr.length())
        sshSessionErr = " - "+sshSessionErr;
#ifdef DEBUG
    x2goDebug<<"I/O error: "<<message<<sshSessionErr<<" ("<<pid<<").";
#endif
    normalExited=false;
    abortString="I/O error: "+message+sshSessionErr;
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

void SshProcess::slotReverseTunnelFailed(SshProcess* creator, QString error)
{
    if (creator==this)
        emit sshFinished(false,error,pid);

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
        x2goDebug << "SSH finished: raw output (stdout): " << stdOutString;
        if ( output.length()<=0 &&  stdErrString.length() >0 )
        {
            normalExited=false;
            output=stdErrString;
#ifdef DEBUG
            x2goDebug<<"Have stderr only, something must be wrong.";
#endif
        }
    }
#ifdef DEBUG
    x2goDebug<<"SSH finished: "<<normalExited<<" - "<<output<<" ("<<pid<<").";
#endif
    emit sshFinished(normalExited, output, pid);
}

void SshProcess::slotSshProcFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    normalExited=false;
    if (exitCode==0 && exitStatus==QProcess::NormalExit)
        normalExited=true;
#ifdef DEBUG
    x2goDebug<<"SSH process exit code :"<<exitStatus;
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
