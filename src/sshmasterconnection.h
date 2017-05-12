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

#ifndef SSHMASTERCONNECTION_H
#define SSHMASTERCONNECTION_H

#include <libssh/libssh.h>
#include <QString>
#include <QList>
#include <QMutex>
#include <QThread>
#include <QStringList>
#include <QTcpSocket>
#include <QNetworkProxy>


#define PROPERTY(TYPE,NAME)  private: TYPE NAME; \
public: TYPE get_##NAME(){return NAME;} \
void set_##NAME(TYPE VAL){NAME=VAL;}


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
    QString uuid;
    bool operator==(ChannelConnection& t)
    {
        return (channel==t.channel);
    }
};

struct ReverseTunnelRequest
{
    uint localPort;
    uint forwardPort;
    QString localHost;
    SshProcess* creator;
    bool listen;
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
    PROPERTY(bool, kerberosDelegation)
public:
    enum ProxyType {PROXYSSH, PROXYHTTP};
    void run();
    SshMasterConnection(QObject* parent, QString host, int port, bool acceptUnknownServers, QString user,
                        QString pass, QString key, bool autologin, bool krblogin=false,
                        bool useproxy=false, ProxyType type=PROXYSSH, QString proxyserver=QString::null, quint16 proxyport=0,
                        QString proxylogin=QString::null, QString proxypassword=QString::null, QString proxyKey=QString::null,
                        bool proxyAutologin=false, bool proxyKrbLogin=false);
    ~SshMasterConnection();
    static void finalizeLibSsh();
    void addChannelConnection(SshProcess* creator, int sock, QString forwardHost,
                              int forwardPort, QString localHost, int localPort, void* channel=0l);
    void addChannelConnection(SshProcess* creator, QString uuid, QString cmd);
    void addCopyRequest(SshProcess* creator, QString src, QString dst);
    void writeKnownHosts(bool);
    void setKeyPhrase(QString);

    int executeCommand(const QString& command, QObject* receiver=0, const char* slotFinished=0, bool overridePath=true);
    int startTunnel(const QString& forwardHost, uint forwardPort, const QString& localHost,
                    uint localPort, bool reverse=false, QObject* receiver=0, const char* slotTunnelOk=0, const char* slotFinished=0);
    int copyFile(const QString& src, const QString dst, QObject* receiver=0, const char* slotFinished=0);
    QString getSourceFile(int pid);

    void setAcceptUnknownServers(bool accept)
    {
        acceptUnknownServers=accept;
    }
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
    bool sshConnect();
    bool userAuthWithPass();
    bool userAuthAuto();
    bool userAuthWithKey();
    bool userChallengeAuth();
    bool checkLogin();
    bool userAuth();
    bool userAuthKrb();
    void channelLoop();
    void finalize(int arg1);
    void copy();
    int serverAuth(QString& errorMsg);
    void setVerficationCode(QString code);
    void checkReverseTunnelConnections();
    void addReverseTunnelConnections();
#ifdef Q_OS_WIN
    void parseKnownHosts();
#endif

private slots:

    void slotSshProxyServerAuthError ( int,QString, SshMasterConnection* );
    void slotSshProxyServerAuthAborted ();
    void slotSshProxyUserAuthError ( QString );
    void slotSshProxyConnectionError ( QString,QString );


    void slotSshProxyConnectionOk();
    void slotSshProxyTunnelOk(int);
    void slotSshProxyTunnelFailed(bool result,  QString output,
                                  int);
    void slotSshProxyInteractionStart ( SshMasterConnection* connection, QString prompt );
    void slotSshProxyInteractionUpdate ( SshMasterConnection* connection, QString output );
    void slotSshProxyInteractionFinish ( SshMasterConnection* connection);

public slots:
    void interactionTextEnter(QString text);
    void interactionInterruptSlot();

private:
    ssh_session my_ssh_session;
    QList<ChannelConnection> channelConnections;
    QList<CopyRequest> copyRequests;
    QList<ReverseTunnelRequest> reverseTunnelRequest;
    QMutex channelConnectionsMutex;
    QMutex copyRequestMutex;
    QMutex disconnectFlagMutex;
    QMutex writeHostKeyMutex;
    QMutex reverseTunnelRequestMutex;
    QMutex interactionInputMutex;
    QString interactionInputText;
    bool interactionInterrupt;
    bool writeHostKey;
    bool writeHostKeyReady;
    int nextPid;
    QList<SshProcess*> processes;

    QString keyPhrase;
    bool keyPhraseReady;
    QMutex keyPhraseMutex;

    QString host;
    int port;
    QString user;
    QString pass;
    QString key;
    bool useproxy;
    QString proxyserver;
    quint16 proxyport;
    QString proxylogin;
    QString proxypassword;
    ProxyType proxytype;
    bool proxyautologin;
    bool proxyKrbLogin;
    QString proxykey;
    QStringList authErrors;
    bool autologin;
    bool disconnectSessionFlag;
    int localProxyPort;
    bool acceptUnknownServers;
    ONMainWindow* mainWnd;
    bool kerberos;
    QString sshProcErrString;
    QTcpSocket *tcpProxySocket;
    QNetworkProxy *tcpNetworkProxy;
    SshMasterConnection* sshProxy;
    bool sshProxyReady;
    bool breakLoop;

    bool challengeAuthPasswordAccepted;
    QString challengeAuthVerificationCode;

    static const QString challenge_auth_code_prompts_[];

signals:
    void stdErr(SshProcess* caller, QByteArray data);
    void stdOut(SshProcess* caller, QByteArray data);
    void ioErr(SshProcess* caller, QString error, QString lastSessionError);
    void copyErr(SshProcess* caller, QString error, QString lastSessionError);
    void copyOk(SshProcess* caller);
    void channelClosed(SshProcess* caller, QString uuid);
    void reverseTunnelOk(SshProcess* caller);
    void reverseTunnelFailed(SshProcess* caller, QString error);

    void connectionError(QString message, QString lastSessionError);
    void serverAuthError(int errCode, QString lastSessionError, SshMasterConnection*);
    void serverAuthAborted();
    void userAuthError(QString error);

    void connectionOk( QString host);

    void needPassPhrase(SshMasterConnection*, bool verificationCode);
    void needChallengeResponse(SshMasterConnection*, QString Challenge);
    void startInteraction(SshMasterConnection*, QString prompt);
    void finishInteraction(SshMasterConnection*);
    void updateInteraction(SshMasterConnection*, QString output);
};


#endif // SSHMASTERCONNECTION_H

