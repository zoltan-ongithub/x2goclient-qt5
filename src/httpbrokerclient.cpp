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

#include "httpbrokerclient.h"
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUuid>
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QSslSocket>
#include "x2gologdebug.h"
#include <QMessageBox>
#include <QDateTime>
#include "onmainwindow.h"
#include "x2gosettings.h"
#include <QDesktopWidget>
#include <QTimer>
#include "SVGFrame.h"
#include "onmainwindow.h"
#include <QTemporaryFile>
#include <QInputDialog>


HttpBrokerClient::HttpBrokerClient ( ONMainWindow* wnd, ConfigFile* cfg )
{
    config=cfg;
    mainWindow=wnd;
    sshConnection=0;
    QUrl lurl ( config->brokerurl );
    if(lurl.userName().length()>0)
        config->brokerUser=lurl.userName();
    nextAuthId=config->brokerUserId;

    if(config->brokerurl.indexOf("ssh://")==0)
    {
        sshBroker=true;
        x2goDebug<<"host:"<<lurl.host();
        x2goDebug<<"port:"<<lurl.port(22);
        x2goDebug<<"uname:"<<lurl.userName();
        x2goDebug<<"path:"<<lurl.path();
        config->sshBrokerBin=lurl.path();
    }
    else
    {
        sshBroker=false;

        if ((config->brokerCaCertFile.length() >0) && (QFile::exists(config->brokerCaCertFile))) {
            QSslSocket::addDefaultCaCertificates(config->brokerCaCertFile, QSsl::Pem);
            x2goDebug<<"Custom CA certificate file loaded into HTTPS broker client: "<<config->brokerCaCertFile;
        }

        http=new QNetworkAccessManager ( this );
        x2goDebug<<"Setting up connection to broker: "<<config->brokerurl;

        connect ( http, SIGNAL ( sslErrors ( QNetworkReply*, const QList<QSslError>& ) ),this,
                  SLOT ( slotSslErrors ( QNetworkReply*, const QList<QSslError>& ) ) );

        connect ( http,SIGNAL ( finished (QNetworkReply*) ),this,
                  SLOT ( slotRequestFinished (QNetworkReply*) ) );
    }
}


HttpBrokerClient::~HttpBrokerClient()
{
}

void HttpBrokerClient::createSshConnection()
{
    QUrl lurl ( config->brokerurl );
    sshConnection=new SshMasterConnection (this, lurl.host(), lurl.port(22),false,
                                           config->brokerUser, config->brokerPass,config->brokerSshKey,config->brokerAutologin,
                                           config->brokerKrbLogin, false);

    connect ( sshConnection, SIGNAL ( connectionOk(QString)), this, SLOT ( slotSshConnectionOk() ) );
    connect ( sshConnection, SIGNAL ( serverAuthError ( int,QString, SshMasterConnection* ) ),this,
              SLOT ( slotSshServerAuthError ( int,QString, SshMasterConnection* ) ) );
    connect ( sshConnection, SIGNAL ( needPassPhrase(SshMasterConnection*, bool)),this,
              SLOT ( slotSshServerAuthPassphrase(SshMasterConnection*, bool)) );
    connect ( sshConnection, SIGNAL ( userAuthError ( QString ) ),this,SLOT ( slotSshUserAuthError ( QString ) ) );
    connect ( sshConnection, SIGNAL ( connectionError(QString,QString)), this,
              SLOT ( slotSshConnectionError ( QString,QString ) ) );
    sshConnection->start();
}

void HttpBrokerClient::slotSshConnectionError(QString message, QString lastSessionError)
{
    if ( sshConnection )
    {
        sshConnection->wait();
        delete sshConnection;
        sshConnection=0l;
    }

    QMessageBox::critical ( 0l,message,lastSessionError,
                            QMessageBox::Ok,
                            QMessageBox::NoButton );
}

void HttpBrokerClient::slotSshConnectionOk()
{
    getUserSessions();
}

void HttpBrokerClient::slotSshServerAuthError(int error, QString sshMessage, SshMasterConnection* connection)
{
    QString errMsg;
    switch ( error )
    {
    case SSH_SERVER_KNOWN_CHANGED:
        errMsg=tr ( "Host key for server changed.\nIt is now: " ) +sshMessage+"\n"+
               tr ( "For security reasons, connection will be stopped" );
        connection->writeKnownHosts(false);
        connection->wait();
        if(sshConnection && sshConnection !=connection)
        {
            sshConnection->wait();
            delete sshConnection;
        }
        sshConnection=0;
        slotSshUserAuthError ( errMsg );
        return;

    case SSH_SERVER_FOUND_OTHER:
        errMsg=tr ( "The host key for this server was not found but an other"
                    "type of key exists.An attacker might change the default server key to"
                    "confuse your client into thinking the key does not exist" );
        connection->writeKnownHosts(false);
        connection->wait();
        if(sshConnection && sshConnection !=connection)
        {
            sshConnection->wait();
            delete sshConnection;
        }
        sshConnection=0;
        slotSshUserAuthError ( errMsg );
        return ;

    case SSH_SERVER_ERROR:
        connection->writeKnownHosts(false);
        connection->wait();
        if(sshConnection && sshConnection !=connection)
        {
            sshConnection->wait();
            delete sshConnection;
        }
        sshConnection=0;
        slotSshUserAuthError ( sshMessage );
        return ;
    case SSH_SERVER_FILE_NOT_FOUND:
        errMsg=tr ( "Could not find known hosts file."
                    "If you accept the host key here, the file will be automatically created." );
        break;

    case SSH_SERVER_NOT_KNOWN:
        errMsg=tr ( "The server is unknown. Do you trust the host key?\nPublic key hash: " ) +sshMessage;
        break;
    }

    if ( QMessageBox::warning ( 0, tr ( "Host key verification failed" ),errMsg,tr ( "Yes" ), tr ( "No" ) ) !=0 )
    {
        connection->writeKnownHosts(false);
        connection->wait();
        if(sshConnection && sshConnection !=connection)
        {
            sshConnection->wait();
            delete sshConnection;
        }
        sshConnection=0;
        slotSshUserAuthError ( tr ( "Host key verification failed" ) );
        return;
    }
    connection->writeKnownHosts(true);
    connection->wait();
    connection->start();

}

void HttpBrokerClient::slotSshServerAuthPassphrase(SshMasterConnection* connection, bool verificationCode)
{
    bool ok;
    QString message;

    if(verificationCode)
    {
        message=tr("Verification code:");
    }
    else
    {
        message=tr("Enter passphrase to decrypt a key");
    }


    QString phrase=QInputDialog::getText(0,connection->getUser()+"@"+connection->getHost()+":"+QString::number(connection->getPort()),
                                         message, QLineEdit::Password,QString::null, &ok);
    if(!ok)
    {
        phrase=QString::null;
    }
    else
    {
        if(phrase==QString::null)
            phrase="";
    }
    connection->setKeyPhrase(phrase);

}

void HttpBrokerClient::slotSshUserAuthError(QString error)
{
    if ( sshConnection )
    {
        sshConnection->wait();
        delete sshConnection;
        sshConnection=0l;
    }

    QMessageBox::critical ( 0l,tr ( "Authentication failed" ),error,
                            QMessageBox::Ok,
                            QMessageBox::NoButton );
    emit authFailed();
    return;
}

void HttpBrokerClient::getUserSessions()
{
    QString brokerUser=config->brokerUser;
    // Otherwise, after logout from the session, we will be connected by a previous user without a password by authid.
    if (config->brokerAutologoff) {
        nextAuthId=config->brokerUserId;
    }
    x2goDebug<<"called getUserSessions: brokeruser: "<<brokerUser<<" authid: "<<nextAuthId;
    if(mainWindow->getUsePGPCard())
        brokerUser=mainWindow->getCardLogin();
    config->sessiondata=QString::null;
    if(!sshBroker)
    {
        QString req;
        QTextStream ( &req ) <<
                             "task=listsessions&"<<
                             "user="<<QUrl::toPercentEncoding(brokerUser)<<"&"<<
                             "password="<<QUrl::toPercentEncoding(config->brokerPass)<<"&"<<
                             "authid="<<nextAuthId;

        x2goDebug << "sending request: "<< req.toUtf8();
        QNetworkRequest request(QUrl(config->brokerurl));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        sessionsRequest=http->post (request, req.toUtf8() );
    }
    else
    {
        if(!sshConnection)
        {
            createSshConnection();
            return;
        }
        if (nextAuthId.length() > 0) {
            sshConnection->executeCommand ( config->sshBrokerBin+" --user "+ brokerUser +" --authid "+nextAuthId+ " --task listsessions",
                                            this, SLOT ( slotListSessions ( bool, QString,int ) ));
        } else {
            sshConnection->executeCommand ( config->sshBrokerBin+" --user "+ brokerUser +" --task listsessions",
                                            this, SLOT ( slotListSessions ( bool, QString,int ) ));
        }
    }
}

void HttpBrokerClient::selectUserSession(const QString& session)
{
    x2goDebug<<"Called selectUserSession for session "<<session<<".";
    QString brokerUser=config->brokerUser;
    if(mainWindow->getUsePGPCard())
        brokerUser=mainWindow->getCardLogin();

    if(!sshBroker)
    {
        QString req;
        QTextStream ( &req ) <<
                             "task=selectsession&"<<
                             "sid="<<session<<"&"<<
                             "user="<<QUrl::toPercentEncoding(brokerUser)<<"&"<<
                             "password="<<QUrl::toPercentEncoding(config->brokerPass)<<"&"<<
                             "authid="<<nextAuthId;
        x2goDebug << "sending request: "<< req.toUtf8();
        QNetworkRequest request(QUrl(config->brokerurl));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        selSessRequest=http->post (request, req.toUtf8() );

    }
    else
    {
        if (nextAuthId.length() > 0) {
            sshConnection->executeCommand ( config->sshBrokerBin+" --user "+ brokerUser +" --authid "+nextAuthId+ " --task selectsession --sid \""+session+"\"",
                                            this,SLOT ( slotSelectSession(bool,QString,int)));
        } else {
            sshConnection->executeCommand ( config->sshBrokerBin+" --user "+ brokerUser +" --task selectsession --sid \""+session+"\"",
                                            this,SLOT ( slotSelectSession(bool,QString,int)));
        }
    }

}

void HttpBrokerClient::changePassword(QString newPass)
{
    newBrokerPass=newPass;
    QString brokerUser=config->brokerUser;
    if(mainWindow->getUsePGPCard())
        brokerUser=mainWindow->getCardLogin();

    if(!sshBroker)
    {
        QString req;
        QTextStream ( &req ) <<
                             "task=setpass&"<<
                             "newpass="<<QUrl::toPercentEncoding(newPass)<<"&"<<
                             "user="<<QUrl::toPercentEncoding(brokerUser)<<"&"<<
                             "password="<<QUrl::toPercentEncoding(config->brokerPass)<<"&"<<
                             "authid="<<nextAuthId;
        x2goDebug << "sending request: "<< req.toUtf8();
        QNetworkRequest request(QUrl(config->brokerurl));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        chPassRequest=http->post (request, req.toUtf8() );
    }
    else
    {
        if (nextAuthId.length() > 0) {
            sshConnection->executeCommand ( config->sshBrokerBin+" --user "+ brokerUser +" --authid "+nextAuthId+ " --task setpass --newpass "+newPass, this,
                                            SLOT ( slotPassChanged(bool,QString,int)));
        } else {
            sshConnection->executeCommand ( config->sshBrokerBin+" --user "+ brokerUser +" --task setpass --newpass "+newPass, this,
                                            SLOT ( slotPassChanged(bool,QString,int)));
        }
    }
}

void HttpBrokerClient::testConnection()
{
    x2goDebug<<"called testConnection";
    if(!sshBroker)
    {
        QString req;
        QTextStream ( &req ) <<
                             "task=testcon";
        x2goDebug << "sending request: "<< req.toUtf8();
        QNetworkRequest request(QUrl(config->brokerurl));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        testConRequest=http->post (request, req.toUtf8() );
    }
    else
    {
        if (nextAuthId.length() > 0) {
            sshConnection->executeCommand(config->sshBrokerBin+" --authid "+nextAuthId+ " --task testcon",
                                          this, SLOT ( slotSelectSession(bool,QString,int)));
        } else {
            sshConnection->executeCommand(config->sshBrokerBin+" --task testcon",
                                          this, SLOT ( slotSelectSession(bool,QString,int)));
        }
    }
}


void HttpBrokerClient::createIniFile(const QString& raw_content)
{
    QString content;
    content = raw_content;
    content.replace("<br>","\n");
    x2goDebug<<"inifile content: "<<content<<"\n";
    QString cont;
    QStringList lines=content.split("START_USER_SESSIONS\n");
    if (lines.count()>1)
    {
        cont=lines[1];
        cont=cont.split("END_USER_SESSIONS\n")[0];
    }
    mainWindow->config.iniFile=cont;
}


bool HttpBrokerClient::checkAccess(QString answer )
{
    x2goDebug<<"called checkAccess - answer was: "<<answer;
    if (answer.indexOf("Access granted")==-1)
    {
        QMessageBox::critical (
            0,tr ( "Error" ),
            tr ( "Login failed!<br>"
                 "Please try again" ) );
        emit authFailed();
        return false;
    }
    config->brokerAuthenticated=true;
    int authBegin=answer.indexOf("AUTHID:");
    if (authBegin!=-1)
    {
        nextAuthId=answer.mid(authBegin+7, answer.indexOf("\n",authBegin)-authBegin-7);
    }
    return true;
}


void HttpBrokerClient::slotConnectionTest(bool success, QString answer, int)
{
    x2goDebug<<"called slotConnectionTest";
    if(!success)
    {
        x2goDebug<<answer;
        QMessageBox::critical(0,tr("Error"),answer);
        emit fatalHttpError();
        return;
    }
    if(!checkAccess(answer))
        return;
    if(!sshBroker)
    {
        x2goDebug<<"elapsed: "<<requestTime.elapsed()<<"received:"<<answer.size()<<endl;
        emit connectionTime(requestTime.elapsed(),answer.size());
    }
    return;

}

void HttpBrokerClient::slotListSessions(bool success, QString answer, int)
{
    if(!success)
    {
        x2goDebug<<answer;
        QMessageBox::critical(0,tr("Error"),answer);
        emit fatalHttpError();
        return;
    }
    if(!checkAccess(answer))
        return;
    createIniFile(answer);
    emit sessionsLoaded();
}

void HttpBrokerClient::slotPassChanged(bool success, QString answer, int)
{
    if(!success)
    {
        x2goDebug<<answer;
        QMessageBox::critical(0,tr("Error"),answer);
        emit fatalHttpError();
        return;
    }
    if(!checkAccess(answer))
        return;

}

void HttpBrokerClient::slotSelectSession(bool success, QString answer, int)
{
    if(!success)
    {
        x2goDebug<<answer;
        QMessageBox::critical(0,tr("Error"),answer);
        emit fatalHttpError();
        return;
    }
    if(!checkAccess(answer))
        return;
    x2goDebug<<"parsing "<<answer;
    parseSession(answer);
}


void HttpBrokerClient::slotRequestFinished ( QNetworkReply*  reply )
{
    if(reply->error() != QNetworkReply::NoError)
    {
        x2goDebug<<"Broker HTTP request failed with error: "<<reply->errorString();
        QMessageBox::critical(0,tr("Error"),reply->errorString());
        emit fatalHttpError();
        return;
    }

    QString answer ( reply->readAll() );
    x2goDebug<<"A http request returned.  Result was: "<<answer;
    if (reply == testConRequest)
    {
        slotConnectionTest(true,answer,0);
    }
    if (reply == sessionsRequest)
    {
        slotListSessions(true, answer,0);
    }
    if (reply == selSessRequest)
    {
        slotSelectSession(true,answer,0);
    }
    if (reply == chPassRequest)
    {
        slotPassChanged(true,answer,0);
    }

    // We receive ownership of the reply object
    // and therefore need to handle deletion.
    reply->deleteLater();
}

void HttpBrokerClient::parseSession(QString sinfo)
{
    x2goDebug<<"starting parser\n";
    QStringList lst=sinfo.split("SERVER:",QString::SkipEmptyParts);
    int keyStartPos=sinfo.indexOf("-----BEGIN DSA PRIVATE KEY-----");
    if(keyStartPos==-1)
        keyStartPos=sinfo.indexOf("-----BEGIN RSA PRIVATE KEY-----");
    QString endStr="-----END DSA PRIVATE KEY-----";
    int keyEndPos=sinfo.indexOf(endStr);
    if(keyEndPos==-1)
    {
        endStr="-----END RSA PRIVATE KEY-----";
        keyEndPos=sinfo.indexOf(endStr);
    }
    if (! (keyEndPos == -1 || keyStartPos == -1 || lst.size()==0))
        config->key=sinfo.mid(keyStartPos, keyEndPos+endStr.length()-keyStartPos);
    QString serverLine=(lst[1].split("\n"))[0];
    QStringList words=serverLine.split(":",QString::SkipEmptyParts);
    config->serverIp=words[0];
    if (words.count()>1)
        config->sshport=words[1];
    x2goDebug<<"server IP: "<<config->serverIp<<"\n";
    x2goDebug<<"server port: "<<config->sshport<<"\n";
    if (sinfo.indexOf("SESSION_INFO")!=-1)
    {
        QStringList lst=sinfo.split("SESSION_INFO:",QString::SkipEmptyParts);
        config->sessiondata=lst[1];
        x2goDebug<<"session data: "<<config->sessiondata<<"\n";
    }
    x2goDebug<<"parsing has finished\n";
    emit sessionSelected();
}


void HttpBrokerClient::slotSslErrors ( QNetworkReply* netReply, const QList<QSslError> & errors )
{
    QStringList err;
    QSslCertificate cert;
    for ( int i=0; i<errors.count(); ++i )
    {
        x2goDebug<<"sslError, code:"<<errors[i].error() <<":";
        err<<errors[i].errorString();
        if ( !errors[i].certificate().isNull() )
            cert=errors[i].certificate();
    }


    QString md5=getHexVal ( cert.digest() );
    QString fname=md5;
    fname=fname.replace(":","_");
    QUrl lurl ( config->brokerurl );
    QString homeDir=mainWindow->getHomeDirectory();
    if ( QFile::exists ( homeDir+"/.x2go/ssl/exceptions/"+
                         lurl.host() +"/"+fname ) )
    {
        QFile fl ( homeDir+"/.x2go/ssl/exceptions/"+
                   lurl.host() +"/"+fname );
        fl.open ( QIODevice::ReadOnly | QIODevice::Text );
        QSslCertificate mcert ( &fl );
        if ( mcert==cert )
        {
            netReply->ignoreSslErrors();
            requestTime.restart();
            return;
        }
    }

    QString text=tr ( "<br><b>Server uses an invalid "
                      "security certificate.</b><br><br>" );
    text+=err.join ( "<br>" );
    text+=tr ( "<p style='background:#FFFFDC;'>"
               "You should not add an exception "
               "if you are using an internet connection "
               "that you do not trust completely or if you are "
               "not used to seeing a warning for this server.</p>" );
    QMessageBox mb ( QMessageBox::Warning,tr ( "Secure connection failed" ),
                     text );
    text=QString::null;
    QTextStream ( &text ) <<err.join ( "\n" ) <<"\n"<<
                          "------------\n"<<
                          tr ( "Issued to:\n" ) <<
                          tr ( "Common Name(CN)\t" ) <<
                          cert.issuerInfo ( QSslCertificate::CommonName )
                          <<endl<<
                          tr ( "Organization(O)\t" ) <<
                          cert.issuerInfo ( QSslCertificate::Organization )
                          <<endl<<
                          tr ( "Organizational Unit(OU)\t" ) <<
                          cert.issuerInfo ( QSslCertificate::OrganizationalUnitName )
                          <<endl<<
                          tr ( "Serial Number\t" ) <<getHexVal ( cert.serialNumber() )
                          <<endl<<endl<<
                          tr ( "Issued by:\n" ) <<
                          tr ( "Common Name(CN)\t" ) <<
                          cert.subjectInfo ( QSslCertificate::CommonName )
                          <<endl<<
                          tr ( "Organization(O)\t" ) <<
                          cert.subjectInfo ( QSslCertificate::Organization )
                          <<endl<<
                          tr ( "Organizational Unit(OU)\t" ) <<
                          cert.subjectInfo ( QSslCertificate::OrganizationalUnitName )
                          <<endl<<endl<<

                          tr ( "Validity:\n" ) <<
                          tr ( "Issued on\t" ) <<cert.effectiveDate().toString() <<endl<<
                          tr ( "expires on\t" ) <<cert.expiryDate().toString() <<endl<<endl<<
                          tr ( "Fingerprints:\n" ) <<
                          tr ( "SHA1\t" ) <<
                          getHexVal ( cert.digest ( QCryptographicHash::Sha1 ) ) <<endl<<
                          tr ( "MD5\t" ) <<md5;



    mb.setDetailedText ( text );
    mb.setEscapeButton (
        ( QAbstractButton* ) mb.addButton ( tr ( "Exit X2Go Client" ),
                                            QMessageBox::RejectRole ) );
    QPushButton *okButton=mb.addButton ( tr ( "Add exception" ),
                                         QMessageBox::AcceptRole );
    mb.setDefaultButton ( okButton );

    mb.exec();
    if ( mb.clickedButton() == ( QAbstractButton* ) okButton )
    {
        x2goDebug<<"accept certificate";
        QDir dr;
        dr.mkpath ( homeDir+"/.x2go/ssl/exceptions/"+lurl.host() +"/" );
        QFile fl ( homeDir+"/.x2go/ssl/exceptions/"+
                   lurl.host() +"/"+fname );
        fl.open ( QIODevice::WriteOnly | QIODevice::Text );
        QTextStream ( &fl ) <<cert.toPem();
        fl.close();
        netReply->ignoreSslErrors();
        x2goDebug<<"store certificate in  "<<homeDir+"/.x2go/ssl/exceptions/"+
                 lurl.host() +"/"+fname;
        requestTime.restart();
    }
    else
        emit fatalHttpError();
}


QString HttpBrokerClient::getHexVal ( const QByteArray& ba )
{
    QStringList val;
    for ( int i=0; i<ba.size(); ++i )
    {
        QString bt;
        bt.sprintf ( "%02X", ( unsigned char ) ba[i] );
        val<<bt;
    }
    return val.join ( ":" );
}

