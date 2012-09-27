//
// C++ Implementation: httpbrokerclient
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "httpbrokerclient.h"
#include <QUrl>
#include <QHttp>
#include <QTextStream>
#include <QFile>
#include <QDir>
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

HttpBrokerClient::HttpBrokerClient ( ONMainWindow* wnd, ConfigFile* cfg )
{
    config=cfg;
    mainWindow=wnd;
    QUrl lurl ( config->brokerurl );
    http=new QHttp ( this );

    if ( config->brokerurl.indexOf ( "https://" ) !=-1 )
        http->setHost ( lurl.host(),QHttp::ConnectionModeHttps,
                        lurl.port ( 443 ) );
    else
        http->setHost ( lurl.host(),QHttp::ConnectionModeHttp,
                        lurl.port ( 80 ) );

    connect ( http,SIGNAL ( requestFinished ( int,bool ) ),this,
              SLOT ( slotRequestFinished ( int,bool ) ) );
    connect ( http,SIGNAL ( sslErrors ( const QList<QSslError>& ) ),this,
              SLOT ( slotSslErrors ( const QList<QSslError>& ) ) );
}


HttpBrokerClient::~HttpBrokerClient()
{
}

void HttpBrokerClient::getUserSessions()
{
    QString req;
    QTextStream ( &req ) <<
    "task=listsessions&"<<
    "user="<<config->brokerUser<<"&"<<
    "password="<<config->brokerPass<<"&"<<
    "authid="<<config->brokerUserId;
    QUrl lurl ( config->brokerurl );
    httpSessionAnswer.close();
    httpSessionAnswer.setData ( 0,0 );
    sessionsRequest=http->post ( lurl.path(),req.toUtf8(),&httpSessionAnswer );
    config->sessiondata=QString::null;

}

void HttpBrokerClient::selectUserSession(const QString& session)
{
//     x2goDebug<<"selected sid: "<<session;
    QString req;
    QTextStream ( &req ) <<
    "task=selectsession&"<<
    "sid="<<session<<"&"<<
    "user="<<config->brokerUser<<"&"<<
    "password="<<config->brokerPass<<"&"<<
    "authid="<<config->brokerUserId;
    QUrl lurl ( config->brokerurl );
    httpSessionAnswer.close();
    httpSessionAnswer.setData ( 0,0 );
    selSessRequest=http->post ( lurl.path(),req.toUtf8(),&httpSessionAnswer );

}

void HttpBrokerClient::changePassword(QString newPass)
{
    newBrokerPass=newPass;
    QString req;
    QTextStream ( &req ) <<
    "task=setpass&"<<
    "newpass="<<newPass<<"&"<<
    "user="<<config->brokerUser<<"&"<<
    "password="<<config->brokerPass<<"&"<<
    "authid="<<config->brokerUserId;
    QUrl lurl ( config->brokerurl );
    httpSessionAnswer.close();
    httpSessionAnswer.setData ( 0,0 );
    chPassRequest=http->post ( lurl.path(),req.toUtf8(),&httpSessionAnswer );

}

void HttpBrokerClient::testConnection()
{
    QString req;
    QTextStream ( &req ) <<
    "task=testcon";
    
    QUrl lurl ( config->brokerurl );
    httpSessionAnswer.close();
    httpSessionAnswer.setData ( 0,0 );
    requestTime.start();
    testConRequest=http->post ( lurl.path(),req.toUtf8(),&httpSessionAnswer );
}


void HttpBrokerClient::createIniFile(const QString& content)
{
    QString cont;
    QStringList lines=content.split("START_USER_SESSIONS<br>");
    if (lines.count()>1)
    {
        cont=lines[1];
        cont=cont.split("END_USER_SESSIONS")[0];
        cont.replace("\n","");
        cont.replace("<br>","\n");
    }
    mainWindow->config.iniFile=cont;
}


void HttpBrokerClient::slotRequestFinished ( int id, bool error )
{
//   	x2goDebug<<"http request "<<id<<", finished with: "<<error;

    if ( error )
    {
        x2goDebug<<http->errorString();
        QMessageBox::critical(0,tr("Error"),http->errorString());
        emit fatalHttpError();
        return;
    }

    if (id==testConRequest)
    {
        
        //x2goDebug<<"cmd request answer: "<<answer;
	x2goDebug<<"elapsed: "<<requestTime.elapsed()<<"received:"<<httpSessionAnswer.size()<<endl;
	emit connectionTime(requestTime.elapsed(),httpSessionAnswer.size());
        return;
    }
    if ( id== sessionsRequest || id == selSessRequest || id==chPassRequest)
    {
        QString answer ( httpSessionAnswer.data() );
        x2goDebug<<"cmd request answer: "<<answer;
        if (answer.indexOf("Access granted")==-1)
        {
            QMessageBox::critical (
                0,tr ( "Error" ),
                tr ( "Login failed!<br>"
                     "Please try again" ) );
            emit authFailed();
            return;
        }
        config->brokerAuthenticated=true;
        if (id == sessionsRequest)
        {
            createIniFile(answer);
            emit sessionsLoaded();
        }
        if (id == selSessRequest)
        {
            emit getSession(answer);
        }
        if ( id == chPassRequest)
        {
            if (answer.indexOf("CHANGING PASS OK")!=-1)
            {
                emit passwordChanged(newBrokerPass);
            }
            else
            {
                emit passwordChanged(QString::null);
            }

        }
    }
}


void HttpBrokerClient::slotSslErrors ( const QList<QSslError> & errors )
{
    QStringList err;
    QSslCertificate cert;
    for ( int i=0;i<errors.count();++i )
    {
        x2goDebug<<"sslError ,code:"<<errors[i].error() <<":";
        err<<errors[i].errorString();
        if ( !errors[i].certificate().isNull() )
            cert=errors[i].certificate();
    }


    QString md5=getHexVal ( cert.digest() );
    QString fname=md5;
    fname=fname.replace(":","_");
    QUrl lurl ( config->brokerurl );
    QString homeDir=mainWindow->getHomeDirectory();
    if ( QFile::exists ( homeDir+"/ssl/exceptions/"+
                         lurl.host() +"/"+fname ) )
    {
        QFile fl ( homeDir+"/ssl/exceptions/"+
                   lurl.host() +"/"+fname );
        fl.open ( QIODevice::ReadOnly | QIODevice::Text );
        QSslCertificate mcert ( &fl );
        if ( mcert==cert )
        {
            http->ignoreSslErrors();
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
        dr.mkpath ( homeDir+"/ssl/exceptions/"+lurl.host() +"/" );
        QFile fl ( homeDir+"/ssl/exceptions/"+
                   lurl.host() +"/"+fname );
        fl.open ( QIODevice::WriteOnly | QIODevice::Text );
        QTextStream ( &fl ) <<cert.toPem();
        fl.close();
        http->ignoreSslErrors();
        x2goDebug<<"store certificate in  "<<homeDir+"/ssl/exceptions/"+
        lurl.host() +"/"+fname;
        requestTime.restart();
    }
    else
        emit fatalHttpError();
}


QString HttpBrokerClient::getHexVal ( const QByteArray& ba )
{
    QStringList val;
    for ( int i=0;i<ba.size();++i )
    {
        QString bt;
        bt.sprintf ( "%02X", ( unsigned char ) ba[i] );
        val<<bt;
    }
    return val.join ( ":" );
}
