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

#ifndef SSHPROCESS_H
#define SSHPROCESS_H

#include <libssh/libssh.h>
#include <QObject>

class SshMasterConnection;


class SshProcess : public QObject
{
    Q_OBJECT

public:
    SshProcess(SshMasterConnection* master, QObject* parent=0);
    ~SshProcess();

public:
    void startNormal(const QString& cmd);
    void startTunnel(const QString& forwardHost, uint forwardPort, const QString& localHost,
                     uint localPort, bool reverse=false);
    void start_cp(QString src, QString dst);
    QString getSource() 
    {
        return scpSource;
    }

private:
    void tunnelLoop();

private:
    SshMasterConnection* masterCon;
    SshMasterConnection* tunnelConnection;
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


private slots:
    void slotCheckNewConnection();
    void slotStdErr(SshProcess* creator, QByteArray data);
    void slotStdOut(SshProcess* creator, QByteArray data);
    void slotIOerr(SshProcess* creator,QString message, QString sshSessionErr);
    void slotChannelClosed(SshProcess* creator);
    void slotReverseTunnelOk(SshProcess* creator);
    void slotCopyOk(SshProcess* creator);
    void slotCopyErr(SshProcess* creator,QString message, QString sshSessionErr);
signals:
    void sshFinished ( bool result, QString output, SshProcess* proc);
    void sshTunnelOk();
    /*
        void sudoConfigError ( QString, SshProcess* );
        */
};

#endif // SSHPROCESS_H
