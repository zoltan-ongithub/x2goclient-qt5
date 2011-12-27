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

#ifndef SSHMASTERCONNECTION_H
#define SSHMASTERCONNECTION_H

#include <libssh/libssh.h>
#include <QString>
#include <QList>
#include <QMutex>
#include <QThread>
#include <QStringList>

class ONMainWindow;
class SshProcess;
struct ChannelConnection
{
    ssh_channel channel;
    int sock;
    SshProcess* creator;
    int forwardPort;
    int localPort;
    QString forwardHost;
    QString localHost;
    QString command;
    bool operator==(ChannelConnection& t)
    {
        return (channel==t.channel);
    }
};

struct CopyRequest
{
    SshProcess* creator;
    QString src;
    QString dst;
};


class SshMasterConnection: public QThread
{
    Q_OBJECT
public:
    void run();
    SshMasterConnection(QString host, int port, bool acceptUnknownServers, QString user,
                        QString pass, QString key, bool autologin, bool krblogin, QObject* parent = 0);
    ~SshMasterConnection();
    static void finalizeLibSsh();
    void addChannelConnection(SshProcess* creator, int sock, QString forwardHost,
                              int forwardPort, QString localHost, int localPort, void* channel=0l);
    void addChannelConnection(SshProcess* creator, QString cmd);
    void addCopyRequest(SshProcess* creator, QString src, QString dst);
    void disconnectSession();
    void setAcceptUnknownServers(bool accept)
    {
        acceptUnknownServers=accept;
    }
    SshMasterConnection* reverseTunnelConnection(SshProcess* creator, int remotePort,
            QString localHost, int localPort);
    QString getHost()
    {
        return host;
    }
    QString getUser()
    {
        return user;
    }
    int getPort()
    {
        return port;
    }
    bool useKerberos()
    {
        return kerberos;
    };


private:
    SshMasterConnection(QString host, int port, bool acceptUnknownServers, QString user, QString pass, QString key,
                        bool autologin,
                        int remotePort, QString localHost, int localPort, SshProcess* creator,
                        QObject* parent, ONMainWindow* parWnd);
    bool sshConnect();
    bool userAuthWithPass();
    bool userAuthAuto();
    bool userAuthWithKey();
    bool userAuth();
    void channelLoop();
    void finalize(int arg1);
    void copy();
    int serverAuth(QString& errorMsg);
#ifdef Q_OS_WIN
    void parseKnownHosts();
#endif

private:
    ssh_session my_ssh_session;
    QList<ChannelConnection> channelConnections;
    QList<CopyRequest> copyRequests;
    QList<SshMasterConnection*> reverseTunnelConnections;
    QMutex channelConnectionsMutex;
    QMutex copyRequestMutex;
    QMutex disconnectFlagMutex;
    QMutex reverseTunnelConnectionsMutex;
    QString host;
    int port;
    QString user;
    QString pass;
    QString key;
    QStringList authErrors;
    bool autologin;
    bool disconnectSessionFlag;
    bool reverseTunnel;
    int reverseTunnelRemotePort;
    int reverseTunnelLocalPort;
    bool acceptUnknownServers;
    QString reverseTunnelLocalHost;
    SshProcess* reverseTunnelCreator;
    ONMainWindow* mainWnd;
    bool kerberos;
    QString sshProcErrString;

signals:
    void stdErr(SshProcess* caller, QByteArray data);
    void stdOut(SshProcess* caller, QByteArray data);
    void ioErr(SshProcess* caller, QString error, QString lastSessionError);
    void copyErr(SshProcess* caller, QString error, QString lastSessionError);
    void copyOk(SshProcess* caller);
    void channelClosed(SshProcess* caller);

    void connectionError(QString message, QString lastSessionError);
    void serverAuthError(int errCode, QString lastSessionError);
    void userAuthError(QString error);

    void newReverceTunnelConnection(SshProcess* creator, void* newChannel);
    void reverseListenOk(SshProcess* creator);
    void connectionOk( QString host);
};


#endif // SSHMASTERCONNECTION_H
