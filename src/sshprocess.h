/**************************************************************************
*   Copyright (C) 2005-2017 by Oleksandr Shneyder                         *
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

#ifndef SSHPROCESS_H
#define SSHPROCESS_H

#include <libssh/libssh.h>
#include <QObject>
#include <QProcess>
#ifndef Q_OS_WIN
#include <netinet/in.h>
#endif

#include "sshmasterconnection.h"

class SshProcess : public QObject
{
    Q_OBJECT
    friend class SshMasterConnection;
private:

    SshProcess(SshMasterConnection* master, int pid);
    ~SshProcess();

    void startNormal(const QString& cmd);
    void startTunnel(const QString& forwardHost, uint forwardPort, const QString& localHost,
                     uint localPort, bool reverse=false);
    void start_cp(QString src, QString dst);
    QString getSource()
    {
        return scpSource;
    }

    void tunnelLoop();
#ifdef Q_OS_WIN
    void    addPuttyReg(QString host, QString uuidStr);
    void    rmPuttyReg(QString uuidStr);
#endif

private:
    SshMasterConnection* masterCon;
    SshMasterConnection* tunnelConnection;
    int pid;
    QString forwardHost;
    QString localHost;
    QString command;
    QString scpSource;
    quint16 forwardPort;
    quint16 localPort;
    uint serverSocket;
    struct sockaddr_in address;
#ifndef  Q_OS_WIN
    socklen_t addrlen;
#else
    int addrlen;
#endif
    QString stdOutString;
    QString stdErrString;
    QString abortString;
    bool tunnel;
    bool normalExited;
//only to use with krb (until no GSSAPI support in libssh)
    QProcess* proc;
    QString procUuid;
    bool execProcess;
    bool tunnelOkEmited;

private slots:
    void slotCheckNewConnection();
    void slotStdErr(SshProcess* creator, QByteArray data);
    void slotStdOut(SshProcess* creator, QByteArray data);
    void slotIOerr(SshProcess* creator,QString message, QString sshSessionErr);
    void slotChannelClosed(SshProcess* creator, QString uuid);
    void slotReverseTunnelOk(SshProcess* creator);
    void slotReverseTunnelFailed(SshProcess* creator, QString error);
    void slotCopyOk(SshProcess* creator);
    void slotCopyErr(SshProcess* creator,QString message, QString sshSessionErr);
    //krb stuff
    void slotSshProcFinished( int exitCode, QProcess::ExitStatus exitStatus);
    void slotSshProcStdErr();
    void slotSshProcStdOut();
signals:
    void sshFinished ( bool result, QString output, int processId);
    void sshTunnelOk(int processId);
};

#endif // SSHPROCESS_H
