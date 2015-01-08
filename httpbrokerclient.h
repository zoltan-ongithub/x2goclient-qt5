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

#ifndef HTTPBROKERCLIENT_H
#define HTTPBROKERCLIENT_H
#include "x2goclientconfig.h"
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslError>
#include <QBuffer>
#include <QObject>
#include <QDateTime>
#include <QSslSocket>
#include "sshmasterconnection.h"
/**
	@author Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>
*/
class QNetworkAccessManager;
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
    QNetworkAccessManager* http;
    QNetworkRequest* netRequest;
    QSslSocket* sslSocket;
    QNetworkReply* sessionsRequest;
    QNetworkReply* selSessRequest;
    QNetworkReply* chPassRequest;
    QNetworkReply* testConRequest;
    QString nextAuthId;
    QString newBrokerPass;
    ConfigFile* config;
    ONMainWindow* mainWindow;
    QTime requestTime;
    bool sshBroker;
    SshMasterConnection* sshConnection;
private:
    void createIniFile(const QString& raw_content);
    void parseSession(QString sInfo);
    void createSshConnection();
    bool checkAccess(QString answer);

private slots:
    void slotRequestFinished ( QNetworkReply*  reply );
    void slotSslErrors ( QNetworkReply* netReply, const QList<QSslError> & errors ) ;
    QString getHexVal ( const QByteArray& ba );
    void slotSshConnectionError ( QString message, QString lastSessionError );
    void slotSshServerAuthError ( int error, QString sshMessage, SshMasterConnection* connection );
    void slotSshServerAuthPassphrase ( SshMasterConnection* connection, bool verificationCode=false );
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
