//
// C++ Interface: httpbrokerclient
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef HTTPBROKERCLIENT_H
#define HTTPBROKERCLIENT_H
#include "x2goclientconfig.h"
#include <QSslError>
#include <QBuffer>
#include <QObject>
#include <QDateTime>
#include "sshmasterconnection.h"
/**
	@author Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>
*/
class QHttp;
struct ConfigFile;
class ONMainWindow;

class HttpBrokerClient: public QObject
{
    Q_OBJECT
public:
    HttpBrokerClient ( ONMainWindow* wnd, ConfigFile* cfg );
    ~HttpBrokerClient();
    void selectUserSession(const QString& session );
    void changePassword(QString newPass);
    void testConnection();
private:
    QBuffer httpCmdAnswer;
    QBuffer httpSessionAnswer;
    QHttp* http;
    int sessionsRequest;
    int selSessRequest;
    int chPassRequest;
    int testConRequest;
    QString newBrokerPass;
    ConfigFile* config;
    ONMainWindow* mainWindow;
    QTime requestTime;
    bool sshBroker;
    SshMasterConnection* sshConnection;
private:
    void createIniFile(const QString& content);
    void parseSession(QString sInfo);
    void createSshConnection();
    bool checkAccess(QString answer);

private slots:
    void slotRequestFinished ( int id, bool error );
    void slotSslErrors ( const QList<QSslError> & errors ) ;
    QString getHexVal ( const QByteArray& ba );
    void slotSshConnectionError ( QString message, QString lastSessionError );
    void slotSshServerAuthError ( int error, QString sshMessage, SshMasterConnection* connection );
    void slotSshServerAuthPassphrase ( SshMasterConnection* connection );
    void slotSshUserAuthError ( QString error );
    void slotSshConnectionOk();
    void slotListSessions ( bool success, QString answer, int pid);
    void slotSelectSession ( bool success, QString answer, int pid);
    void slotPassChanged ( bool success, QString answer, int pid);
    void slotConnectionTest( bool success, QString answer, int pid);

public slots:
    void getUserSessions();

signals:
    void haveSshKey ( QString );
    void fatalHttpError();
    void authFailed();
    void sessionsLoaded();
    void sessionSelected( );
    void passwordChanged( QString );
    void connectionTime(int, int);
};

#endif
