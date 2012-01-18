/***************************************************************************
*   Copyright (C) 2005-2012 by Oleksandr Shneyder   *
*   oleksandr.shneyder@obviously-nice.de   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  F*
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#include "onmainwindow_privat.h"

void ONMainWindow::slotSshConnectionOk()
{
    x2goDebug<<"ssh connection ok"<<endl;
    passForm->setEnabled ( true );
    if ( useLdap )
    {
	continueLDAPSession();
    }
    else
        continueNormalSession();
}


SshMasterConnection*  ONMainWindow::findServerSshConnection(QString host)
{
  for(int i=0;i<serverSshConnections.count();++i)
  {
    if(serverSshConnections[i])
    {
      if(serverSshConnections[i]->getHost()==host)
	return serverSshConnections[i];
    }
  }
  return 0l;
}

void ONMainWindow::slotServSshConnectionOk(QString server)
{
        SshMasterConnection* con=findServerSshConnection(server);
	if(!con)
	  return;
        SshProcess* lproc;
        lproc=new SshProcess ( con,  this );
        connect ( lproc,SIGNAL ( sshFinished ( bool,
                                               QString,SshProcess* ) ),
                  this,SLOT (
                      slotListAllSessions ( bool,
                                            QString,SshProcess* ) ) );
	x2goDebug<<"getting sessions on "<<server<<endl;
        lproc->startNormal ( "export HOSTNAME && x2golistsessions" );
}

void ONMainWindow::slotSshServerAuthError ( int error, QString sshMessage )
{
    if ( startHidden )
    {
        startHidden=false;
        slotResize();
        show();
        activateWindow();
        raise();
    }
    QString errMsg;
    switch ( error )
    {
    case SSH_SERVER_KNOWN_CHANGED:
        errMsg=tr ( "Host key for server changed.\nIt is now: " ) +sshMessage+"\n"+
               tr ( "For security reasons, connection will be stopped" );
        if ( sshConnection )
        {
            delete sshConnection;
            sshConnection=0l;
        }
        slotSshUserAuthError ( errMsg );
        return;

    case SSH_SERVER_FOUND_OTHER:
        errMsg=tr ( "The host key for this server was not found but an other"
                    "type of key exists.An attacker might change the default server key to"
                    "confuse your client into thinking the key does not exist" );
        if ( sshConnection )
        {
            delete sshConnection;
            sshConnection=0l;
        }
        slotSshUserAuthError ( errMsg );
        return ;

    case SSH_SERVER_ERROR:
        if ( sshConnection )
        {
            delete sshConnection;
            sshConnection=0l;
        }
        slotSshUserAuthError ( errMsg );
        return ;
    case SSH_SERVER_FILE_NOT_FOUND:
        errMsg=tr ( "Could not find known host file."
                    "If you accept the host key here, the file will be automatically created" );
        break;

    case SSH_SERVER_NOT_KNOWN:
        errMsg=tr ( "The server is unknown. Do you trust the host key?\nPublic key hash: " ) +sshMessage;
        break;
    }

    if ( QMessageBox::warning ( this, tr ( "Host key verification failed" ),errMsg,tr ( "Yes" ), tr ( "No" ) ) !=0 )
    {
        if ( sshConnection )
        {
            delete sshConnection;
            sshConnection=0l;
        }
        slotSshUserAuthError ( tr ( "Host key verification failed" ) );
        return;
    }
    sshConnection->setAcceptUnknownServers ( true );
    sshConnection->start();
}

void ONMainWindow::slotSshUserAuthError ( QString error )
{
    if ( sshConnection )
    {
        delete sshConnection;
        sshConnection=0l;
    }

    QMessageBox::critical ( 0l,tr ( "Authentification failed" ),error,
                            QMessageBox::Ok,
                            QMessageBox::NoButton );
// 	currentKey=QString::null;
    setEnabled ( true );
    passForm->setEnabled ( true );
    slotShowPassForm();
    pass->setFocus();
    pass->selectAll();



    passForm->setEnabled ( true );
    if ( startHidden )
    {
        startHidden=false;
        slotResize();
        show();
        activateWindow();
        raise();
    }
}

void ONMainWindow::slotSessEnter()
{
    if ( useLdap )
    {
        slotPassEnter();
        return;
    }
    if(brokerMode)
    {
      if(!config.brokerAuthenticated)
      {
	x2goDebug<<"starting broker request";
	slotStartBroker();
	return;
      }
    }

    resumingSession.sessionId=QString::null;
    resumingSession.server=QString::null;
    resumingSession.display=QString::null;
    setStatStatus ( tr ( "connecting" ) );

    QString sid="";
    if ( !embedMode )
        sid=lastSession->id();
    startSession ( sid );
}

void ONMainWindow::continueNormalSession()
{
    x2goDebug<<"continue normal x2go session"<<endl;
    if(brokerMode)
    {
      slotListSessions(true,QString::null,0);
      return;
    }
    SshProcess* proc=new SshProcess ( sshConnection, this );
    connect ( proc,SIGNAL ( sshFinished ( bool,QString,SshProcess* ) ),
              this,SLOT ( slotListSessions ( bool, QString,
                                             SshProcess* ) ) );
    if ( !shadowSession )      
        proc->startNormal ( "export HOSTNAME && x2golistsessions" );
    else
        proc->startNormal ( "export HOSTNAME && x2golistdesktops" );

}

void ONMainWindow::continueLDAPSession()
{


    SshProcess* proc=new SshProcess ( sshConnection,this );
    connect ( proc,SIGNAL ( sshFinished ( bool,QString,SshProcess* ) ),
              this,SLOT ( slotGetServers ( bool, QString,SshProcess* ) ) );
    proc->startNormal ( "x2gogetservers" );
}


bool ONMainWindow::startSession ( const QString& sid )
{
    setEnabled ( false );
    QString passwd;
    QString user;
    QString host;
    bool autologin=false;
    bool krblogin=false;
    user=getCurrentUname();
    runRemoteCommand=true;
    shadowSession=false;

    if ( managedMode )
    {
        slotListSessions ( true, QString::null,0 );
        return true;
    }

    if ( !embedMode && !brokerMode )
    {

        X2goSettings st ( "sessions" );

        passForm->setEnabled ( false );
        host=st.setting()->value ( sid+"/host",
                                   ( QVariant ) QString::null ).toString();
        QString cmd=st.setting()->value ( sid+"/command",
                                          ( QVariant ) QString::null ).toString();
        autologin=st.setting()->value ( sid+"/autologin",
                                        ( QVariant ) false ).toBool();
        krblogin=st.setting()->value ( sid+"/krblogin",
                                        ( QVariant ) false ).toBool();
        if ( cmd=="SHADOW" )
            shadowSession=true;
    }
    else
    {
        host=config.server;
        sshPort=config.sshport;
        selectedCommand=config.command;
    }
    if(!brokerMode)
       passwd=getCurrentPass();
    else
    {
      currentKey=config.key;
      host=config.server;
      X2goSettings st ( config.iniFile, QSettings::IniFormat );
      passForm->setEnabled ( false );
      user=st.setting()->value ( sid+"/user",
                                   ( QVariant ) QString::null ).toString();
      login->setText(user);
      sshPort=st.setting()->value ( sid+"/sshport",
                                   ( QVariant ) "22" ).toString();
    }
    if(sshConnection)
      sshConnection->disconnectSession();
    sshConnection=startSshConnection ( host,sshPort,acceptRsa,user,passwd,autologin, krblogin );
    return true;
}


void ONMainWindow::slotListSessions ( bool result,QString output,
                                      SshProcess* proc )
{
    if ( proc )
        delete proc;
    if ( result==false )
    {
        cardReady=false;
        cardStarted=false;
        QString message=tr ( "<b>Connection failed</b>\n" ) +output;
        if ( message.indexOf ( "publickey,password" ) !=-1 )
        {
            message=tr ( "<b>Wrong password!</b><br><br>" ) +
                    message;
        }

        QMessageBox::critical ( 0l,tr ( "Error" ),message,
                                QMessageBox::Ok,
                                QMessageBox::NoButton );
// 		currentKey=QString::null;
        setEnabled ( true );
        passForm->setEnabled ( true );
        slotShowPassForm();
        pass->setFocus();
        pass->selectAll();
        return;
    }

    passForm->hide();
    if ( !embedMode )
    {
        setUsersEnabled ( false );
        uname->setEnabled ( false );
        u->setEnabled ( false );
    }
    if ( managedMode || brokerMode )
    {
        x2goDebug<<"sess data:"<<config.sessiondata;
        if ( config.sessiondata.indexOf ( "|S|" ) ==-1 )
        {
            x2goDebug<<"start new managed session";
            startNewSession();
        }
        else
        {
            x2goSession s=getSessionFromString (
                              config.sessiondata );
            x2goDebug<<"resuming managed session:"<<s.sessionId;
            resumeSession ( s );
        }
        return;
    }

    QStringList sessions=output.trimmed().split ( '\n',
                         QString::SkipEmptyParts );
    if ( shadowSession )
    {
        selectSession ( sessions );
    }
    else
    {
        if ( ( sessions.size() ==0 ) ||
                ( sessions.size() ==1&&sessions[0].length() <5 ) )
            startNewSession();
        else if ( sessions.size() ==1 )
        {
            x2goSession s=getSessionFromString ( sessions[0] );
            QDesktopWidget wd;
            if ( s.status=="S" && isColorDepthOk ( wd.depth(),
                                                   s.colorDepth )
                    &&s.command == selectedCommand )
                resumeSession ( s );
            else
            {
                if ( startHidden )
                    startNewSession();
                else
                    selectSession ( sessions );
            }
        }
        else
        {
            if ( !startHidden )
                selectSession ( sessions );
            else
            {
                for ( int i=0;i<sessions.size();++i )
                {
                    x2goSession s=getSessionFromString (
                                      sessions[i] );
                    QDesktopWidget wd;
                    if ( s.status=="S" && isColorDepthOk (
                                wd.depth(),s.colorDepth )
                            &&s.command == selectedCommand )
                    {
                        resumeSession ( s );
                        return;
                    }
                }
                startNewSession();
            }
        }
    }
}


x2goSession ONMainWindow::getSessionFromString ( const QString& string )
{
    QStringList lst=string.split ( '|' );
    x2goSession s;
    s.agentPid=lst[0];
    s.sessionId=lst[1];
    s.display=lst[2];
    s.server=lst[3];
    s.status=lst[4];
    s.crTime=lst[5];
    s.cookie=lst[6];
    s.clientIp=lst[7];
    s.grPort=lst[8];
    s.sndPort=lst[9];
    if ( lst.count() >13 )
        s.fsPort=lst[13];
    s.colorDepth=0;
    if ( s.sessionId.indexOf ( "_dp" ) !=-1 )
    {
        s.colorDepth=s.sessionId.split ( "_dp" ) [1].toInt();
    }
    s.sessionType=x2goSession::DESKTOP;
    s.command=tr ( "unknown" );
    if ( s.sessionId.indexOf ( "_st" ) !=-1 )
    {
        QString cmdinfo=s.sessionId.split ( "_st" ) [1];
        cmdinfo=cmdinfo.split ( "_" ) [0];
        QChar st=cmdinfo[0];
        if ( st=='R' )
            s.sessionType=x2goSession::ROOTLESS;
        if ( st=='S' )
            s.sessionType=x2goSession::SHADOW;
        QString command=cmdinfo.mid ( 1 );
        if ( command.length() >0 )
            s.command=command;
    }
    return s;
}


void ONMainWindow::startNewSession()
{
    newSession=true;
    QString passwd=getCurrentPass();
    QString user=getCurrentUname();

    QString pack;
    bool fullscreen;
    int height;
    int width;
    int quality;
    int speed;
    bool usekbd;
    bool rootless=false;
    bool setDPI=defaultSetDPI;
    uint dpi=defaultDPI;
    QString layout;
    QString type;
    QString command;
    QString xdmcpServer;
    runRemoteCommand=true;
    QString host=QString::null;

    if ( useLdap )
    {
        pack=defaultPack;
        fullscreen=defaultFullscreen;
        height=defaultHeight;
        width=defaultWidth;
        quality=defaultQuality;
        speed=defaultLink;
        usekbd=defaultSetKbd;
        layout=defaultLayout[0];
        type=defaultKbdType;
        command=defaultCmd;
        shadowSession=false;
        for ( int j=0;j<x2goServers.size();++j )
        {
            if ( x2goServers[j].connOk )
            {
                host=x2goServers[j].name;
                break;
            }
        }
        if ( host==QString::null )
        {
            QMessageBox::critical ( 0l,tr ( "Error" ),
                                    tr ( "No server availabel" ),
                                    QMessageBox::Ok,
                                    QMessageBox::NoButton );
            return;
        }
        sshConnection=findServerSshConnection(host);
	if(!sshConnection)
        {
            QMessageBox::critical ( 0l,tr ( "Error" ),
                                    tr ( "Server not availabel" ),
                                    QMessageBox::Ok,
                                    QMessageBox::NoButton );
            return;
        }	
    }
    else
    {
        X2goSettings* st;
	
	if(!brokerMode)
	  st=new X2goSettings( "sessions" );
	else
	  st= new X2goSettings(config.iniFile,QSettings::IniFormat);
        
	QString sid;
        if ( !embedMode )
            sid=lastSession->id();
        else
            sid="embedded";
        pack=st->setting()->value ( sid+"/pack",
                                   ( QVariant ) defaultPack ).toString();
        fullscreen=st->setting()->value ( sid+"/fullscreen",
                                         ( QVariant )
                                         defaultFullscreen ).toBool();
        height=st->setting()->value ( sid+"/height",
                                     ( QVariant ) defaultHeight ).toInt();
        width=st->setting()->value ( sid+"/width",
                                    ( QVariant ) defaultWidth ).toInt();
        setDPI=st->setting()->value ( sid+"/setdpi",
                                     ( QVariant ) defaultSetDPI ).toBool();
        dpi=st->setting()->value ( sid+"/dpi",
                                  ( QVariant ) defaultDPI ).toUInt();
        quality=st->setting()->value (
                    sid+"/quality",
                    ( QVariant ) defaultQuality ).toInt();
        speed=st->setting()->value ( sid+"/speed",
                                    ( QVariant ) defaultLink ).toInt();

        usekbd=st->setting()->value ( sid+"/usekbd",
                                     ( QVariant ) defaultSetKbd ).toBool();
        layout=st->setting()->value ( sid+"/layout",
                                     ( QVariant )
                                     defaultLayout[0] ).toString();
        type=st->setting()->value ( sid+"/type",
                                   ( QVariant )
                                   defaultKbdType ).toString();
        if ( !embedMode )
        {
            command=st->setting()->value ( sid+"/command",
                                          ( QVariant ) defaultCmd ).toString();
            host=st->setting()->value (
                     sid+"/host",
                     ( QVariant )
                     ( QString ) "localhost" ).toString();

            rootless=st->setting()->value ( sid+"/rootless",
                                           ( QVariant ) false ).toBool();
            xdmcpServer=st->setting()->value ( sid+"/xdmcpserver",
                                              ( QVariant )
                                              "localhost" ).toString();
        }
        else
        {
            command=config.command;
            rootless= config.rootless;
            host=config.server;
            startEmbedded=false;
            if ( st->setting()->value ( sid+"/startembed",
                                       ( QVariant ) true ).toBool() )
            {
                startEmbedded=true;
                fullscreen=false;
                height=bgFrame->size().height()-stb->height();
                width=bgFrame->size().width();

                if ( height<0 ||width<0 )
                {
                    height=defaultHeight;
                    width=defaultWidth;
                }
            }
            if ( config.confConSpd )
                speed=config.conSpeed;
            if ( config.confCompMet )
                pack=config.compMet;
            if ( config.confImageQ )
                quality=config.imageQ;
            if ( config.confDPI )
            {
                dpi=config.dpi;
                setDPI=true;
            }
            if ( config.confKbd )
            {
                layout=config.kbdLay;
                type=config.kbdType;
                usekbd=true;
            }
        }
        if ( command=="RDP" )
            rootless=true;
        if ( command=="XDMCP" )
        {
            runRemoteCommand=false;
        }
        delete st;
    }


    if ( shadowSession )
    {
        runRemoteCommand=false;
    }

    resumingSession.server=host;
    
    if(defaultLayout.size()>0)
	  layout=cbLayout->currentText();


    QString geometry;
#ifdef Q_OS_WIN
    x2goDebug<<"fullscreen: "<<fullscreen;
    maximizeProxyWin=false;
    proxyWinWidth=width;
    proxyWinHeight=height;
//#ifdef CFGCLIENT
    xorgMode=WIN;
    if(fullscreen)
      xorgMode=FS;
    if(rootless)
      xorgMode=SAPP;
    xorgWidth=QString::number(width);
    xorgHeight=QString::number(height);
    if(! startXorgOnStart)
      startXOrg();
//#endif
#endif
    if ( fullscreen )
    {
        geometry="fullscreen";
#ifdef Q_OS_WIN
//        fullscreen=false;
        maximizeProxyWin=true;
        x2goDebug<<"maximizeProxyWin: "<<maximizeProxyWin;
#endif
    }
    if ( ! fullscreen )
    {
        geometry=QString::number ( width ) +"x"+
                 QString::number ( height );
        if ( embedMode )
        {
            QPoint position=mapToGlobal ( bgFrame->pos() );
            geometry+="+"+QString::number ( position.x() ) +"+"+
                      QString::number ( position.y() +
                                        stb->height() );
        }

    }
    QString link;
    switch ( speed )
    {
    case MODEM:
        link="modem";
        break;
    case ISDN:
        link="isdn";
        break;
    case ADSL:
        link="adsl";
        break;
    case WAN:
        link="wan";
        break;
    case LAN:
        link="lan";
        break;
    }

    QFile file ( ":/txt/packs" );
    if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
        return;
    QTextStream in ( &file );
    while ( !in.atEnd() )
    {
        QString pc=in.readLine();
        if ( pc.indexOf ( "-%" ) !=-1 )
        {
            pc=pc.left ( pc.indexOf ( "-%" ) );
            if ( pc==pack )
            {
                pack+="-"+QString::number ( quality );
                break;
            }
        }
    }
    file.close();


    if ( selectSessionDlg->isVisible() )
    {
        if ( !embedMode )
            slotCloseSelectDlg();
        else
            selectSessionDlg->hide();
    }
    QDesktopWidget wd;
    QString depth=QString::number ( wd.depth() );
#ifdef Q_OS_DARWIN
    usekbd=0;
    type="query";
#endif
    QString sessTypeStr="D ";
    if ( rootless )
        sessTypeStr="R ";
    if ( shadowSession )
        sessTypeStr="S ";
    QString dpiEnv;
    QString xdmcpEnv;
    if ( runRemoteCommand==false && command=="XDMCP" )
        xdmcpEnv="X2GOXDMCP="+xdmcpServer+" ";
    if ( setDPI )
    {
        dpiEnv="X2GODPI="+QString::number ( dpi ) +" ";
    }
    QString cmd=dpiEnv+xdmcpEnv+"x2gostartagent "+
                geometry+" "+link+" "+pack+
                " unix-kde-depth_"+depth+" "+layout+" "+type+" ";
    if ( usekbd )
        cmd += "1 ";
    else
        cmd += "0 ";
    QFileInfo f ( command );
    if ( !shadowSession )
        cmd+=sessTypeStr+f.fileName();
    else
    {
        cmd+=sessTypeStr+QString::number ( shadowMode ) +"XSHAD"+
             shadowUser+"XSHAD"+shadowDisplay;
    }
    x2goDebug<<cmd<<endl;
    if ( managedMode )
    {
        slotRetResumeSess ( true,config.sessiondata,0 );
        passForm->hide();
        return;
    }

    SshProcess* proc=0l;

    proc=new SshProcess ( sshConnection, this ) ;


    connect ( proc,SIGNAL ( sshFinished ( bool, QString,SshProcess* ) ),
              this,SLOT ( slotRetResumeSess ( bool,
                                              QString,SshProcess* ) ) );

    proc->startNormal ( cmd );
    passForm->hide();
}



void ONMainWindow::resumeSession ( const x2goSession& s )
{
    newSession=false;

    QString passwd=getCurrentPass();
    QString user=getCurrentUname();
    QString host=s.server;
    bool rootless=false;

    QString pack;
    bool fullscreen;
    int height;
    int width;
    int quality;
    int speed;
    bool usekbd;
    QString layout;
    QString type;

    if ( useLdap )
    {
        pack=defaultPack;
        fullscreen=defaultFullscreen;
        height=defaultHeight;
        width=defaultWidth;
        quality=defaultQuality;
        speed=defaultLink;
        usekbd=defaultSetKbd;
        layout=defaultLayout[0];
        type=defaultKbdType;
        sshConnection=findServerSshConnection(host);
	if(!sshConnection)
        {
            QMessageBox::critical ( 0l,tr ( "Error" ),
                                    tr ( "Server not availabel" ),
                                    QMessageBox::Ok,
                                    QMessageBox::NoButton );
            return;
        }
    }
    else
    {

        QString sid;
        if ( !embedMode )
            sid=lastSession->id();
        else
            sid="embedded";
        X2goSettings* st;
	if(!brokerMode)
	  st=new X2goSettings( "sessions" );
	else
	  st=new X2goSettings(config.iniFile,QSettings::IniFormat);

        pack=st->setting()->value ( sid+"/pack",
                                   ( QVariant ) defaultPack ).toString();

        fullscreen=st->setting()->value ( sid+"/fullscreen",
                                         ( QVariant )
                                         defaultFullscreen ).toBool();
        height=st->setting()->value ( sid+"/height",
                                     ( QVariant ) defaultHeight ).toInt();
        width=st->setting()->value ( sid+"/width",
                                    ( QVariant ) defaultWidth ).toInt();
        quality=st->setting()->value ( sid+"/quality",
                                      ( QVariant )
                                      defaultQuality ).toInt();
        speed=st->setting()->value ( sid+"/speed",
                                    ( QVariant ) defaultLink ).toInt();
        usekbd=st->setting()->value ( sid+"/usekbd",
                                     ( QVariant ) defaultSetKbd ).toBool();
        layout=st->setting()->value ( sid+"/layout",
                                     ( QVariant )
                                     defaultLayout[0] ).toString();
        type=st->setting()->value ( sid+"/type",
                                   ( QVariant )
                                   defaultKbdType ).toString();
        rootless=st->setting()->value ( sid+"/rootless",
                                     ( QVariant ) false ).toBool();

        if ( !embedMode )
        {
            host=st->setting()->value ( sid+"/host",
                                       ( QVariant ) s.server ).toString();
        }
        else
        {
            startEmbedded=false;
            if ( st->setting()->value ( sid+"/startembed",
                                       ( QVariant ) true ).toBool() )
            {
                fullscreen=false;
                startEmbedded=true;
                height=bgFrame->size().height()-stb->height();
                width=bgFrame->size().width();
                if ( height<0 ||width<0 )
                {
                    height=defaultHeight;
                    width=defaultWidth;
                }
            }
            rootless=config.rootless;
            host=config.server;
            if ( config.confConSpd )
                speed=config.conSpeed;
            if ( config.confCompMet )
                pack=config.compMet;
            if ( config.confImageQ )
                quality=config.imageQ;
            if ( config.confKbd )
            {
                layout=config.kbdLay;
                type=config.kbdType;
                usekbd=true;
            }
        }
        delete st;
    }
    
    if(defaultLayout.size()>0)
	layout=cbLayout->currentText();

    QString geometry;
#ifdef Q_OS_WIN
    maximizeProxyWin=false;
    proxyWinWidth=width;
    proxyWinHeight=height;
// #ifdef CFGCLIENT
    xorgMode=WIN;
    if(fullscreen)
      xorgMode=FS;
    if(rootless)
      xorgMode=SAPP;
    xorgWidth=QString::number(width);
    xorgHeight=QString::number(height);
    if(! startXorgOnStart)
      startXOrg();
// #endif
    
#endif
    if ( fullscreen )
    {
        geometry="fullscreen";
#ifdef Q_OS_WIN
//        fullscreen=false;
        maximizeProxyWin=true;
        x2goDebug<<"maximizeProxyWin: "<<maximizeProxyWin;

#endif
    }
    if ( !fullscreen )
    {
        geometry=QString::number ( width ) +"x"+
                 QString::number ( height );
    }
    QString link;
    switch ( speed )
    {
    case MODEM:
        link="modem";
        break;
    case ISDN:
        link="isdn";
        break;
    case ADSL:
        link="adsl";
        break;
    case WAN:
        link="wan";
        break;
    case LAN:
        link="lan";
        break;
    }

    QFile file ( ":/txt/packs" );
    if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
        return;
    QTextStream in ( &file );
    while ( !in.atEnd() )
    {
        QString pc=in.readLine();
        if ( pc.indexOf ( "-%" ) !=-1 )
        {
            pc=pc.left ( pc.indexOf ( "-%" ) );
            if ( pc==pack )
            {
                pack+="-"+QString::number ( quality );
                break;
            }
        }
    }
    file.close();

#ifdef Q_OS_DARWIN
    usekbd=0;
    type="query";
#endif


    if ( selectSessionDlg->isVisible() )
    {
        if ( !embedMode )
            slotCloseSelectDlg();
        else
            selectSessionDlg->hide();
    }
    QString cmd="x2goresume-session "+s.sessionId+" "+geometry+
                " "+link+" "+pack+" "+layout+
                " "+type+" ";
    if ( usekbd )
        cmd += "1";
    else
        cmd += "0";

    SshProcess* proc=0l;
    proc=new SshProcess ( sshConnection, this );

    connect ( proc,SIGNAL ( sshFinished ( bool, QString,SshProcess* ) ),
              this,SLOT ( slotRetResumeSess ( bool, QString,
                                              SshProcess* ) ) );

    proc->startNormal ( cmd );
    resumingSession=s;
    passForm->hide();
}


void ONMainWindow::selectSession ( QStringList& sessions )
{
    setEnabled ( true );
    sessionStatusDlg->hide();
    passForm->hide();
// 	x2goDebug<<"check type";
    if ( !shadowSession )
    {
// 		x2goDebug<<"! shadow";
        if ( !miniMode )
            selectSesDlgLayout->setContentsMargins ( 25,25,10,10 );

        bNew->show();
        bSusp->show();
        bTerm->show();
        sOk->show();
        sCancel->show();
// 		x2goDebug<<"standart buttons ok";
        desktopFilter->hide();
        desktopFilterCb->hide();
        bShadow->hide();
        bShadowView->hide();
        bCancel->hide();
// 		x2goDebug<<"shadow buttons ok";
// 		model->clear();
        model->removeRows ( 0,model->rowCount() );
        selectSessionLabel->setText ( tr ( "Select session:" ) );
        selectedSessions.clear();
        QFontMetrics fm ( sessTv->font() );
        for ( int row = 0; row < sessions.size(); ++row )
        {

// 			x2goDebug<<"decoding sessionstr:"<<sessions[row];
            x2goSession s=getSessionFromString ( sessions[row] );
// 			x2goDebug<<"listing id:"<<s.sessionId;
            selectedSessions.append ( s );
            QStandardItem *item;

            item= new QStandardItem ( s.display );
            model->setItem ( row,S_DISPLAY,item );

            if ( s.status=="R" )
                item= new QStandardItem ( tr ( "running" ) );
            else
                item= new QStandardItem ( tr ( "suspended" ) );
            model->setItem ( row,S_STATUS,item );

            item= new QStandardItem ( transAppName ( s.command ) );
            model->setItem ( row,S_COMMAND,item );

            QString type=tr ( "Desktop" );
            if ( s.sessionType==x2goSession::ROOTLESS )
                type=tr ( "single application" );
            if ( s.sessionType==x2goSession::SHADOW )
                type=tr ( "shadow session" );

            item= new QStandardItem ( type );
            model->setItem ( row,S_TYPE,item );

            item= new QStandardItem ( s.crTime );
            model->setItem ( row,S_CRTIME,item );
            item= new QStandardItem ( s.server );
            model->setItem ( row,S_SERVER,item );
            item= new QStandardItem ( s.clientIp );
            model->setItem ( row,S_IP,item );
            item= new QStandardItem ( s.sessionId );
            model->setItem ( row,S_ID,item );
            for ( int j=0;j<8;++j )
            {
                QString txt=
                    model->index ( row,j ).data().toString();
                if ( sessTv->header()->sectionSize ( j ) <
                        fm.width ( txt ) +6 )
                {
                    sessTv->header()->resizeSection (
                        j,fm.width ( txt ) +6 );
                }
            }
        }
    }
    else
    {
        shadowMode=SHADOW_VIEWONLY;
        selectedDesktops.clear();
        selectedDesktops=sessions;
        if ( sessions.size() ==0 )
        {
            QMessageBox::information ( this,tr ( "Information" ),
                                       tr ( "No accessible desktop "
                                            "found" ) );
            slotCloseSelectDlg();
            return;
        }
        sessTv->setModel ( ( QAbstractItemModel* ) modelDesktop );
        desktopFilter->show();
        desktopFilterCb->show();
        sOk->hide();
        sCancel->hide();
        bShadow->show();
        bCancel->show();
        bShadowView->show();
        desktopFilter->setText ( tr ( "Filter" ) );
        sessions.sort();
        if ( !miniMode )
            selectSesDlgLayout->setContentsMargins ( 25,25,25,25 );
        bNew->hide();
        bSusp->hide();
        bTerm->hide();
        selectSessionLabel->setText ( tr ( "Select desktop:" ) );
        filterDesktops ( "" );
        desktopFilter->setFocus();
        desktopFilter->selectAll();
    }
    selectSessionDlg->show();
}

void ONMainWindow::slotCloseSelectDlg()
{
    selectSessionDlg->hide();
    if ( !embedMode )
    {
        u->setEnabled ( true );
        uname->setEnabled ( true );
    }
    slotShowPassForm();
}



void ONMainWindow::slotActivated ( const QModelIndex& index )
{
    if ( !shadowSession )
    {
        QString status=sessTv->model()->index (
                           index.row(),
                           S_STATUS ).data().toString();
        if ( status==tr ( "running" ) )
        {
            bSusp->setEnabled ( true );
            sOk->setEnabled ( false );
        }
        else
        {
            bSusp->setEnabled ( false );
            sOk->setEnabled ( true );
        }
        bTerm->setEnabled ( true );
        if ( status==QString::null )
        {
            sOk->setEnabled ( false );
            bTerm->setEnabled ( false );
        }
    }
    else
    {
        QString user=sessTv->model()->index (
                         index.row(),
                         D_USER ).data().toString();
        bShadowView->setEnabled ( true );
        bShadow->setEnabled ( user==getCurrentUname() );
    }
}


void ONMainWindow::slotResumeSess()
{
    x2goSession s=getSelectedSession();
    QDesktopWidget wd;
    if ( isColorDepthOk ( wd.depth(),s.colorDepth ) )
        resumeSession ( s );
    else
    {
        QString depth=QString::number ( s.colorDepth );
        int res;
        if ( s.colorDepth==24 || s.colorDepth==32 )
        {
            res=QMessageBox::warning (
                    0l,tr ( "Warning" ),
                    tr (
                        "Your current color depth is "
                        "different to the color depth of your "
                        "x2go-session. This may cause problems "
                        "reconnecting to this session and in most "
                        "cases <b>you will loose the session</b> "
                        "and have to start a new one! It's highly "
                        "recommended to change the color depth of "
                        "your Display to " ) +tr ( "24 or 32" ) +
                    tr (
                        " bit and restart your X-server before you "
                        "reconnect to this x2go-session.<br>Resume "
                        "this session anyway?" ),tr ( "Yes" ),
                    tr ( "No" ) );

        }
        else
        {
            res=QMessageBox::warning (
                    0l,tr ( "Warning" ),
                    tr (
                        "Your current color depth is different to "
                        "the color depth of your x2go-session. "
                        "This may cause problems reconnecting to "
                        "this session and in most cases <b>you "
                        "will loose the session</b> and have to "
                        "start a new one! It's highly recommended "
                        "to change the color depth of your "
                        "Display to " ) +depth+
                    tr (
                        " bit and restart your X-server before you "
                        "reconnect to this x2go-session.<br>Resume "
                        "this session anyway?" ),tr ( "Yes" ),
                    tr ( "No" ) );
        }
        if ( res==0 )
            resumeSession ( s );
    }

}


void ONMainWindow::slotSuspendSess()
{

    QString passwd;
    QString user=getCurrentUname();

    passwd=getCurrentPass();

    selectSessionDlg->setEnabled ( false );


    QString sessId=sessTv->model()->index (
                       sessTv->currentIndex().row(),
                       S_ID ).data().toString();
    QString host=sessTv->model()->index (
                     sessTv->currentIndex().row(),
                     S_SERVER ).data().toString();
    if ( !useLdap )
    {
        if ( !embedMode )
        {
            X2goSettings st ( "sessions" );
            QString sid=lastSession->id();
            host=st.setting()->value ( sid+"/host",
                                       ( QVariant ) host ).toString();
        }
        else
        {
            host=config.server;
        }
    }
    else
    {
      sshConnection=findServerSshConnection(host);
      if(!sshConnection)
      {
            QMessageBox::critical ( 0l,tr ( "Error" ),
                                    tr ( "Server not availabel" ),
                                    QMessageBox::Ok,
                                    QMessageBox::NoButton );
            return;
      }
    }

     
    suspendSession ( sessId );
}


void ONMainWindow::slotSuspendSessFromSt()
{
    QString passwd;
    QString user=getCurrentUname();
    passwd=getCurrentPass();
    setStatStatus ( tr ( "suspending" ) );


    sbExp->setEnabled ( false );

    if ( !shadowSession )
        suspendSession ( resumingSession.sessionId );
    else
        termSession ( resumingSession.sessionId,false );
}

void ONMainWindow::slotTermSessFromSt()
{

    /*	x2goDebug <<"disconnect export"<<endl;
    	disconnect ( sbExp,SIGNAL ( clicked() ),this,
    	             SLOT ( slot_exportDirectory() ) );*/
    sbExp->setEnabled ( false );

    if ( !shadowSession )
    {
        if ( termSession ( resumingSession.sessionId ) )
            setStatStatus ( tr ( "terminating" ) );
    }
    else
        termSession ( resumingSession.sessionId,false );
}


void ONMainWindow::slotRetSuspSess ( bool result, QString output,
                                     SshProcess* proc )
{
    if ( proc )
        delete proc;
    if ( result==false )
    {
        QString message=tr ( "<b>Connection failed</b>\n" ) +output;
        if ( message.indexOf ( "publickey,password" ) !=-1 )
        {
            message=tr (
                        "<b>Wrong password!</b><br><br>" ) +message;
        }

        QMessageBox::critical ( 0l,tr ( "Error" ),message,
                                QMessageBox::Ok,
                                QMessageBox::NoButton );
    }
    else
    {
        if ( selectSessionDlg->isVisible() )
        {
            ( ( QStandardItemModel* )
              ( sessTv->model() ) )->item (
                  sessTv->currentIndex().row(),
                  S_STATUS )->setData (
                      QVariant ( ( QString ) tr ( "suspended" ) ),
                      Qt::DisplayRole );
            bSusp->setEnabled ( false );
            sOk->setEnabled ( true );

        }
    }
    if ( selectSessionDlg->isVisible() )
        selectSessionDlg->setEnabled ( true );
}



void ONMainWindow::slotTermSess()
{

    selectSessionDlg->setEnabled ( false );


    QString sessId=sessTv->model()->index ( sessTv->currentIndex().row(),
                                            S_ID ).data().toString();

    if ( !useLdap )
    {
        if ( !embedMode )
        {
            X2goSettings st ( "sessions" );

            QString sid=lastSession->id();
        }
    }
    else
    {
      QString host=sessTv->model()->index ( sessTv->currentIndex().row(),
                                            S_SERVER ).data().toString();
					    
      sshConnection=findServerSshConnection(host);
      if(!sshConnection)
      {
            QMessageBox::critical ( 0l,tr ( "Error" ),
                                    tr ( "Server not availabel" ),
                                    QMessageBox::Ok,
                                    QMessageBox::NoButton );
            return;
      }
    }

    termSession ( sessId );
}


void ONMainWindow::slotNewSess()
{
    startNewSession();
}


void ONMainWindow::slotRetTermSess ( bool result,  QString output,
                                     SshProcess* proc )
{
    bool nodel= ( proc==0 );
    if ( proc )
        delete proc;
    if ( result==false )
    {
        QString message=tr ( "<b>Connection failed</b>\n" ) +output;
        if ( message.indexOf ( "publickey,password" ) !=-1 )
        {
            message=tr (
                        "<b>Wrong password!</b><br><br>" ) +message;
        }

        QMessageBox::critical ( 0l,tr ( "Error" ),
                                message,QMessageBox::Ok,
                                QMessageBox::NoButton );
    }
    else
    {
        if ( selectSessionDlg->isVisible() &&!nodel )
        {
            sessTv->model()->removeRow (
                sessTv->currentIndex().row() );
            slotActivated ( sessTv->currentIndex() );
        }
    }
    if ( selectSessionDlg->isVisible() )
        selectSessionDlg->setEnabled ( true );
}

void ONMainWindow::slotRetResumeSess ( bool result,
                                       QString output,
                                       SshProcess* proc )
{
    if ( proc )
        delete proc;
    x2goDebug<<"Agent output:"<<output;
    if ( result==false )
    {
        QString message=tr ( "<b>Connection failed</b>\n" ) +output;
        if ( message.indexOf ( "publickey,password" ) !=-1 )
        {
            message=tr (
                        "<b>Wrong Password!</b><br><br>" ) +message;
        }
        if ( output.indexOf ( "LIMIT" ) !=-1 )
        {
            QString sessions=output.mid (
                                 output.indexOf ( "LIMIT" ) +6 );

            message="Sessions limit reached:"+sessions;
        }
        if ( output.indexOf ( "ACCESS DENIED" ) !=-1 )
        {
            message="Access denied from user";
        }

        QMessageBox::critical ( 0l,tr ( "Error" ),
                                message,QMessageBox::Ok,
                                QMessageBox::NoButton );
        slotShowPassForm();
        return;
    }

    output.replace ( " ","" );
    QString passwd=getCurrentPass();
    QString user=getCurrentUname();
    QString host;

    bool sound=true;
    int sndSystem=PULSE;
    QString sndPort;
#ifndef Q_OS_WIN
    sndPort="4713";
#endif
    bool startSoundServer=true;
    bool sshSndTunnel=true;

    if ( useLdap )
    {
        sound=startSound;
        startSoundServer=LDAPSndStartServer;
        if ( LDAPSndSys=="arts" )
            sndSystem=ARTS;
        if ( LDAPSndSys=="esd" )
            sndSystem=ESD;
        sndPort=LDAPSndPort;
    }
    else
    {
        QString sid;
        if ( !embedMode )
            sid=lastSession->id();
        else
            sid="embedded";
        X2goSettings st ( "sessions" );

        sound=st.setting()->value ( sid+"/sound",
                                    ( QVariant ) true ).toBool();
        QString sndsys=st.setting()->value (
                           sid+"/soundsystem",
                           ( QVariant ) "pulse" ).toString();
        if ( sndsys=="arts" )
            sndSystem=ARTS;
        if ( sndsys=="esd" )
            sndSystem=ESD;
#ifndef Q_OS_WIN
        sndPort=st.setting()->value ( sid+"/sndport" ).toString();
#endif
        startSoundServer=st.setting()->value (
                             sid+"/startsoundsystem",
                             true ).toBool();

        if ( embedMode&&config.confSnd )
        {
            sound=config.useSnd;
        }


#ifndef Q_OS_WIN
        bool defPort=st.setting()->value ( sid+
                                           "/defsndport",true ).toBool();
        if ( defPort )
        {
            switch ( sndSystem )
            {
            case PULSE:
                sndPort="4713";
                break;
            case ESD:
                sndPort="16001";
                break;
            }
        }
#endif
        sshSndTunnel=st.setting()->value ( sid+"/soundtunnel",
                                           true ).toBool();

#ifdef Q_OS_WIN
        switch ( sndSystem )
        {
        case PULSE:
            sndPort=QString::number ( pulsePort );
            break;
        case ESD:
            sndPort=QString::number ( esdPort );
            break;
        }
#endif
    }

    //Will be used in runCommand
    startSessSound=sound;
    startSessSndSystem=sndSystem;

    if ( newSession )
    {
        QString sString=output.trimmed();
        sString.replace ( '\n','|' );
        host=resumingSession.server;
        resumingSession=getNewSessionFromString ( sString );
        resumingSession.server=host;
        resumingSession.crTime=QDateTime::currentDateTime().toString (
                                   "dd.MM.yy HH:mm:ss" );
        if ( managedMode )
        {
            //replace session data for future resuming
            config.sessiondata=resumingSession.agentPid+"|"+
                               resumingSession.sessionId+"|"+
                               resumingSession.display+"|"+
                               resumingSession.server+"|"+
                               "S|"+
                               resumingSession.crTime+"|"+
                               resumingSession.cookie+"|"+
                               resumingSession.clientIp+"|"+
                               resumingSession.grPort+"|"+
                               resumingSession.sndPort+"|"+
                               resumingSession.crTime+"|"+
                               user+"|"+
                               "0|"+
                               resumingSession.fsPort;
        }
    }
    else
        host=resumingSession.server;
    if ( !useLdap )
    {
        if ( !embedMode )
        {
            X2goSettings st ( "sessions" );

            QString sid=lastSession->id();
            host=st.setting()->value ( sid+"/host",
                                       ( QVariant ) host ).toString();
        }
        else
            host=config.server;
        resumingSession.server=host;
    }
    tunnel=new SshProcess ( sshConnection, this );

    connect ( tunnel,SIGNAL ( sshFinished ( bool,  QString,SshProcess* ) ),
              this,SLOT ( slotTunnelFailed ( bool,
                                             QString,SshProcess* ) ) );
    connect ( tunnel,SIGNAL ( sshTunnelOk() ),
              this,SLOT ( slotTunnelOk() ) );

    localGraphicPort=resumingSession.grPort;
    int iport=localGraphicPort.toInt() +1000;
    while ( iport == resumingSession.sndPort.toInt() ||
            iport == resumingSession.fsPort.toInt() ||
            isServerRunning ( iport ) )
        ++iport;
    localGraphicPort=QString::number ( iport );

    tunnel->startTunnel ( "localhost",resumingSession.grPort.toInt(),"localhost",
                          localGraphicPort.toInt() );
    if ( shadowSession )
        return;

    sndTunnel=0l;
    if ( sound )
    {
        if ( sndSystem==PULSE )
        {
            startSoundServer=false;
            QString scmd;
            if ( !sshSndTunnel )
                scmd="echo \"default-server=`echo "
                     "$SSH_CLIENT | awk '{print $1}'`:"+
                     sndPort+
                     "\"> ~/.x2go/C-"+
                     resumingSession.sessionId+
                     "/.pulse-client.conf"
                     ";echo \"cookie-file=.x2go/C-"+
                     resumingSession.sessionId+
                     "/.pulse-cookie"+
                     "\">> ~/.x2go/C-"+
                     resumingSession.sessionId+
                     "/.pulse-client.conf";
            else
                scmd="echo \"default-server=localhost:"+
                     resumingSession.sndPort+
                     "\"> ~/.x2go/C-"+
                     resumingSession.sessionId+
                     "/.pulse-client.conf"
                     ";echo \"cookie-file=.x2go/C-"+
                     resumingSession.sessionId+
                     "/.pulse-cookie"+
                     "\">> ~/.x2go/C-"+
                     resumingSession.sessionId+
                     "/.pulse-client.conf";
            SshProcess* paProc;
            paProc=new SshProcess ( sshConnection, this );
            paProc->startNormal ( scmd );

            bool sysPulse=false;
#ifdef Q_OS_LINUX
            loadPulseModuleNativeProtocol();
            QFile file ( "/etc/default/pulseaudio" );
            if ( file.open ( QIODevice::ReadOnly |
                             QIODevice::Text ) )
            {

                while ( !file.atEnd() )
                {
                    QByteArray line = file.readLine();
                    int pos=line.indexOf (
                                "PULSEAUDIO_SYSTEM_START=1"
                            );
                    if ( pos!=-1 )
                    {
                        int commentPos=line.indexOf (
                                           "#" );
                        if ( commentPos==-1 ||
                                commentPos>pos )
                        {
                            sysPulse=true;
                            break;
                        }
                    }
                }
                file.close();
            }
#endif
            if ( sysPulse )
                paProc->start_cp (
                    "/var/run/pulse/.pulse-cookie",
                    "~/.x2go/C-"+
                    resumingSession.sessionId+
                    "/.pulse-cookie" );
            else
            {
#ifndef Q_OS_WIN
                paProc->start_cp ( homeDir+"/.pulse-cookie",
                                   "~/.x2go/C-"+
                                   resumingSession.sessionId+
                                   "/.pulse-cookie" );
#else
                QString cooFile=
                    wapiShortFileName ( homeDir )  +
                    "/.x2go/pulse/.pulse-cookie";
                QString destFile="~/.x2go/C-"+
                                 resumingSession.sessionId+
                                 "/.pulse-cookie";
                paProc->start_cp ( cooFile,
                                   destFile );

                /*x2goDebug<<"cookie file: "<<cooFile
                <<" remote:"<<
                destFile<<endl;*/
                connect ( paProc,
                          SIGNAL (
                              sshFinished ( bool,
                                            QString,
                                            SshProcess* ) ),
                          this,
                          SLOT (
                              slotPCookieReady ( bool,
                                                 QString,
                                                 SshProcess* )
                          ) );
                parecTunnelOk=true;
#endif
            }
        }
        if ( sndSystem==ESD )
        {
            SshProcess* paProc;
            paProc=new SshProcess ( sshConnection, this );
#ifndef Q_OS_WIN
            paProc->start_cp ( homeDir+"/.esd_auth",
                               "~/.esd_auth" );
#else
            QString cooFile=
                wapiShortFileName ( homeDir )  +
                "/.x2go/pulse/.esd_auth";
            QString destFile="~/.esd_auth";
            paProc->start_cp ( cooFile,
                               destFile );
#endif
        }
#ifndef Q_OS_WIN
        if ( startSoundServer )
        {
            soundServer=new QProcess ( this );
            QString acmd="artsd",ecmd="esd";
#ifdef Q_OS_DARWIN
            QStringList env = soundServer->environment();
            QDir dir ( appDir );
            dir.cdUp();
            dir.cd ( "esd" );
            env.insert ( 0,"DYLD_LIBRARY_PATH="+
                         dir.absolutePath() );
            soundServer->setEnvironment ( env );
            ecmd="\""+dir.absolutePath() +"\"/esd";
#endif //Q_OS_DARWIN
            if ( sndSystem==ESD )
                soundServer->start (
                    ecmd+
                    " -tcp -nobeeps -bind localhost -port "+
                    resumingSession.sndPort );
            if ( sndSystem==ARTS )
                soundServer->start ( acmd+" -u -N -p "+
                                     resumingSession.sndPort );
            sndPort=resumingSession.sndPort;
        }
#endif //Q_OS_WIN
        if ( sshSndTunnel )
        {
            sndTunnel=new SshProcess ( sshConnection, this );

#ifdef Q_OS_WIN
            if ( sndSystem==PULSE )
            {
                parecTunnelOk=false;
                connect ( sndTunnel,SIGNAL ( sshTunnelOk() ),
                          this,SLOT ( slotSndTunOk() ) );
            }
#endif
            connect ( sndTunnel,SIGNAL ( sshFinished ( bool,
                                         QString,
                                         SshProcess* ) ),
                      this,SLOT (
                          slotSndTunnelFailed ( bool,
                                                QString,
                                                SshProcess* ) ) );

            sndTunnel->startTunnel (
                "localhost",
                resumingSession.sndPort.toInt(),"127.0.0.1",
                sndPort.toInt(),true );
            /*x2goDebug<<"starting tunnel, local port:"<<
            	sndPort<<", remote: "<<
            	resumingSession.sndPort<<
            	endl;*/
        }
    }
}



x2goSession ONMainWindow::getSelectedSession()
{
    QString sessId=sessTv->model()->index ( sessTv->currentIndex().row(),
                                            S_ID ).data().toString();
    for ( int i=0;i<selectedSessions.size();++i )
    {
        if ( selectedSessions[i].sessionId==sessId )
            return selectedSessions[i];
    }
    return selectedSessions[0]; //warning !!!!! undefined session
}


void ONMainWindow::slotTunnelOk()
{

#ifdef Q_OS_WIN
    //waiting for X
    if ( !winServersReady )
    {
        x2goDebug<<"waiting for win-servers";
        QTimer::singleShot ( 100, this, SLOT ( slotTunnelOk() ) );
        return;
    }
#endif

    showExport=false;
    QString nxroot=homeDir +"/.x2go";
    QString dirpath=nxroot+"/S-"+resumingSession.sessionId;
    QDir d ( dirpath );
    if ( !d.exists() )
        if ( !d.mkpath ( dirpath ) )
        {
            QString message=tr ( "Unable to create folder:" ) +
                            dirpath;
            QMessageBox::critical ( 0l,tr ( "Error" ),message,
                                    QMessageBox::Ok,
                                    QMessageBox::NoButton );
            slotShowPassForm();
            if ( tunnel )
                delete tunnel;
            if ( sndTunnel )
                delete sndTunnel;
            if ( fsTunnel )
                delete fsTunnel;
            if ( soundServer )
                delete soundServer;
            tunnel=sndTunnel=fsTunnel=0l;
            soundServer=0l;
            nxproxy=0l;
            return;
        }
#ifdef Q_OS_WIN
    dirpath=wapiShortFileName ( dirpath );
    nxroot=wapiShortFileName ( nxroot );
#endif
    QFile file ( dirpath+"/options" );
    if ( !file.open ( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        QString message=tr ( "Unable to write file:" ) +
                        dirpath+"/options";
        QMessageBox::critical ( 0l,tr ( "Error" ),message,
                                QMessageBox::Ok,
                                QMessageBox::NoButton );
        slotShowPassForm();
        return;
    }

    QTextStream out ( &file );
#ifdef Q_OS_WIN
    dirpath=cygwinPath ( dirpath );
    nxroot=cygwinPath ( nxroot );
#endif
    out << "nx/nx,root="<<nxroot<<",connect=localhost,cookie="<<
    resumingSession.cookie<<",port="
    <<localGraphicPort/*resumingSession.grPort*/<<
    ",errors="<<dirpath<<"/sessions:"<<
    resumingSession.display;
    file.close();
    xmodExecuted=false;


    nxproxy=new QProcess;
    QStringList env = QProcess::systemEnvironment();
    QString x2golibpath="/usr/lib/x2go";
#if defined ( Q_OS_WIN ) || defined ( Q_OS_DARWIN )
    int dispInd=-1;
#endif
    for ( int l=0;l<env.size();++l )
    {
// 		x2goDebug<<env[l]<<endl;
        if ( env[l].indexOf ( "X2GO_LIB" ) ==0 )
        {
            x2golibpath=env[l].split ( "=" ) [1];
        }
#if defined ( Q_OS_WIN ) || defined ( Q_OS_DARWIN )
        if ( env[l].indexOf ( "DISPLAY" ) ==0 )
        {
            dispInd=l;
        }
#endif
    }
    env << "LD_LIBRARY_PATH="+x2golibpath;
    env << "NX_CLIENT="+QCoreApplication::applicationFilePath ();

#if defined ( Q_OS_WIN ) || defined ( Q_OS_DARWIN )
    QString disp=getXDisplay();
    if ( disp==QString::null )
    {
        //slotProxyerror ( QProcess::FailedToStart );
        return;
    }
    if ( dispInd==-1 )
    {
        env <<"DISPLAY=localhost:"+disp;
        /*		x2goDebug<<"new env DISPLAY"<<"DISPLAY=localhost:"+disp<<endl;*/
    }
    else
    {
        env[dispInd]="DISPLAY=localhost:"+disp;
        /*		x2goDebug<<"existing env DISPLAY("<<dispInd<<
        		") DISPLAY=localhost:"+disp<<endl;*/
    }
#endif
#ifdef Q_OS_DARWIN
    //setting /usr/X11/bin to find xauth
    env.insert (
        0,
        "PATH=/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin:/usr/X11/bin" );
#endif
    nxproxy->setEnvironment ( env );
    /*	x2goDebug<<"new env:"<<endl;
    	for ( int l=0;l<env.size();++l )
    	{
    		x2goDebug<<env[l]<<endl;
    	}
    	x2goDebug<<"##########################"<<endl;*/
    connect ( nxproxy,SIGNAL ( error ( QProcess::ProcessError ) ),this,
              SLOT ( slotProxyError ( QProcess::ProcessError ) ) );
    connect ( nxproxy,SIGNAL ( finished ( int,QProcess::ExitStatus ) ),this,
              SLOT ( slotProxyFinished ( int,QProcess::ExitStatus ) ) );
    connect ( nxproxy,SIGNAL ( readyReadStandardError() ),this,
              SLOT ( slotProxyStderr() ) );
    connect ( nxproxy,SIGNAL ( readyReadStandardOutput() ),this,
              SLOT ( slotProxyStdout() ) );

    QString proxyCmd="nxproxy -S nx/nx,options="+dirpath+"/options:"+
                     resumingSession.display;
#ifdef Q_OS_DARWIN
    //run nxproxy from bundle
    QDir dir ( appDir );
    dir.cdUp();
    dir.cd ( "exe" );
    proxyCmd="\""+dir.absolutePath() +"/\""+proxyCmd;
#endif //Q_OS_DARWIN
    x2goDebug<<"starting nxproxy with: "<<proxyCmd<<endl;
    nxproxy->start ( proxyCmd );
//always search for proxyWin
    proxyWinTimer->start ( 300 );
    if ( embedMode )
    {
//         proxyWinTimer->start ( 300 );
        if ( !startEmbedded )
        {
            act_embedContol->setText (
                tr ( "Attach X2Go window" ) );
        }
    }
#ifdef Q_OS_WIN
    else
    {
// #ifdef CFGCLIENT
//         // if using XMing, we must find proxy win for case, that we should make it fullscreen
        //         if(useInternalX&& (internalX==XMING))
// #endif
//         proxyWinTimer->start ( 300 );
    }
#endif

    showSessionStatus();
    QTimer::singleShot ( 30000,this,SLOT ( slotRestartProxy() ) );

}

void ONMainWindow::slotTunnelFailed ( bool result,  QString output,
                                      SshProcess* )
{
    if ( result==false )
    {
        if ( !managedMode )
        {
            QString message=tr ( "Unable to create SSL tunnel:\n" )
                            +output;
            QMessageBox::critical ( 0l,tr ( "Error" ),message,
                                    QMessageBox::Ok,
                                    QMessageBox::NoButton );
        }
        if ( tunnel )
            delete tunnel;
        if ( sndTunnel )
            delete sndTunnel;
        if ( fsTunnel )
            delete fsTunnel;
        if ( soundServer )
            delete soundServer;
        tunnel=sndTunnel=fsTunnel=0l;
        soundServer=0l;
        nxproxy=0l;
        if ( !managedMode )
            slotShowPassForm();
    }
}

void ONMainWindow::slotSndTunnelFailed ( bool result,  QString output,
        SshProcess* )
{
    if ( result==false )
    {
        if ( !managedMode )
        {
            QString message=tr ( "Unable to create SSL Tunnel:\n" )
                            +output;
            QMessageBox::warning ( 0l,tr ( "Warning" ),message,
                                   QMessageBox::Ok,
                                   QMessageBox::NoButton );
        }
        if ( sndTunnel )
            delete sndTunnel;
        sndTunnel=0l;
    }
}



void ONMainWindow::slotProxyError ( QProcess::ProcessError )
{
    slotProxyFinished ( -1,QProcess::CrashExit );
}


void ONMainWindow::slotProxyFinished ( int,QProcess::ExitStatus )
{
    if ( embedMode )
    {
        if ( proxyWinEmbedded )
        {
#ifdef CFGPLUGIN
            detachClient();
#endif
        }
        proxyWinTimer->stop();
        setEmbedSessionActionsEnabled ( false );
    }
#ifdef Q_OS_WIN
    else
        proxyWinTimer->stop();
// #ifdef CFGCLIENT
    if(! startXorgOnStart)
    {
      if(xorg)
      {
	if(xorg->state() ==QProcess::Running)
	{
	  xorg->terminate();
	  delete xorg;
	  xorg=0;
	}
      }
    }
// #endif
#endif
    if ( closeEventSent )
        return;
    if ( tunnel )
        delete tunnel;
    if ( sndTunnel )
        delete sndTunnel;
    if ( fsTunnel )
        delete fsTunnel;
    if ( soundServer )
        delete soundServer;
    if ( spoolTimer )
        delete spoolTimer;

    x2goDebug<<"deleting proxy"<<endl;
    disconnect ( nxproxy,SIGNAL ( error ( QProcess::ProcessError ) ),this,
                 SLOT ( slotProxyError ( QProcess::ProcessError ) ) );
    disconnect ( nxproxy,SIGNAL ( finished ( int,QProcess::ExitStatus ) ),this,
                 SLOT ( slotProxyFinished ( int,QProcess::ExitStatus ) ) );
    disconnect ( nxproxy,SIGNAL ( readyReadStandardError() ),this,
                 SLOT ( slotProxyStderr() ) );
    disconnect ( nxproxy,SIGNAL ( readyReadStandardOutput() ),this,
                 SLOT ( slotProxyStdout() ) );
#ifndef CFGPLUGIN
    if (trayEnabled)
    {
        trayIconActiveConnectionMenu->setTitle(tr("Not connected"));
        trayIconActiveConnectionMenu->setEnabled(false);
        if (trayMaxDiscon)
            showNormal();
    }
    trayAutoHidden=false;
#endif


#if ! (defined(Q_OS_WIN) && defined (CFGPLUGIN))
    if ( nxproxy )
    {
        if ( nxproxy->state() ==QProcess::Running )
        {
            x2goDebug<<"waiting for proxy to exit"<<endl;
            if ( !nxproxy->waitForFinished ( 3000 ) )
            {
                x2goDebug<<"Failed, try to kill"<<endl;
                nxproxy->kill();
            }
        }
        x2goDebug<<"nxproxy not running"<<endl;
        delete nxproxy;
    }
#endif
    x2goDebug<<"proxy deleted"<<endl;
    spoolTimer=0l;
    tunnel=sndTunnel=fsTunnel=0l;
    soundServer=0l;
    nxproxy=0l;
    proxyWinId=0;

    if ( !shadowSession && !usePGPCard && ! ( embedMode &&
            ( config.checkexitstatus==false ) ) )
        check_cmd_status();
    else
        sshConnection->disconnectSession();
    if ( startHidden )
        close();

    if ( readExportsFrom!=QString::null )
    {
        exportTimer->stop();
        if ( extLogin )
        {
            currentKey=QString::null;
        }
    }
    if ( printSupport )
        cleanPrintSpool();
    if ( !restartResume )
    {
        if ( !embedMode )
        {
	  if(!brokerMode)
	  {
            pass->setText ( "" );
            QTimer::singleShot ( 2000,this,
                                 SLOT ( slotShowPassForm() ) );
	  }
	  else
	                QTimer::singleShot ( 2000,broker,
                                 SLOT ( getUserSessions() ) );
	    
        }
    }
    else
    {
        restartResume=false;
        sessionStatusDlg->hide();
        resumeSession ( resumingSession );
    }
    setStatStatus ( tr ( "Finished" ) );
}


void ONMainWindow::slotProxyStderr()
{
    QString reserr;
    if ( nxproxy )
        reserr= nxproxy->readAllStandardError();
    x2goDebug<<reserr<<endl;
    stInfo->insertPlainText ( reserr );
    stInfo->ensureCursorVisible();
    if ( stInfo->toPlainText().indexOf (
                "Connecting to remote host 'localhost:"+
                /*resumingSession.grPort*/ localGraphicPort ) !=-1 )
        setStatStatus ( tr ( "connecting" ) );

    if ( stInfo->toPlainText().indexOf (
                "Connection to remote proxy 'localhost:"+
                /*resumingSession.grPort*/
                localGraphicPort+"' established" ) !=-1 )
    {
        if ( newSession )
            setStatStatus ( tr ( "starting" ) );
        else
            setStatStatus ( tr ( "resuming" ) );
    }

    if ( stInfo->toPlainText().indexOf (
                "Established X server connection" ) !=-1 )
    {
        setStatStatus ( tr ( "running" ) );
#ifndef CFGPLUGIN
        if (trayEnabled)
        {
	       if(!useLdap)
	          trayIconActiveConnectionMenu->setTitle(lastSession->name());
	       else
		  trayIconActiveConnectionMenu->setTitle(lastUser->username());
            trayIconActiveConnectionMenu->setEnabled(true);
            if (trayMinCon && !trayAutoHidden)
            {
                trayAutoHidden=true;
                hide();
            }
        }
#endif
        if ( embedMode )
            setEmbedSessionActionsEnabled ( true );
        disconnect ( sbSusp,SIGNAL ( clicked() ),this,
                     SLOT ( slotTestSessionStatus() ) );
        disconnect ( sbSusp,SIGNAL ( clicked() ),this,
                     SLOT ( slotSuspendSessFromSt() ) );
        connect ( sbSusp,SIGNAL ( clicked() ),this,
                  SLOT ( slotSuspendSessFromSt() ) );
        if ( !showExport )
        {
            showExport=true;
            /*connect ( sbExp,SIGNAL ( clicked() ),this,
              SLOT ( slot_exportDirectory() ) );*/
            sbExp->setEnabled ( true );
            exportDefaultDirs();
            if ( readExportsFrom!=QString::null )
            {
                exportTimer->start ( 2000 );
            }
        }
        sbSusp->setText ( tr ( "Suspend" ) );
        if ( newSession )
        {
            runCommand();
            newSession=false;
        }
#ifdef 	Q_WS_HILDON
        else
        {
            if ( !xmodExecuted )
            {
                xmodExecuted=true;
                QTimer::singleShot (
                    2000, this,
                    SLOT ( slotExecXmodmap() ) );
            }
        }
#endif
    }
    if ( stInfo->toPlainText().indexOf (
                tr ( "Connection timeout, aborting" ) ) !=-1 )
        setStatStatus ( tr ( "aborting" ) );
#if defined( Q_OS_WIN ) && defined (CFGPLUGIN)
    if ( reserr.indexOf ( "Session terminated at" ) !=-1 )
    {
        x2goDebug<<"proxy finished"<<endl;
        slotProxyFinished ( 0, QProcess::NormalExit );
    }
#endif


}


void ONMainWindow::slotProxyStdout()
{
    QString resout ( nxproxy->readAllStandardOutput() );
    x2goDebug<<resout<<endl;

}


void ONMainWindow::slotShowPassForm()
{
    if ( !useLdap )
    {
        loginPrompt->show();
        login->show();
    }
    else
    {
        loginPrompt->hide();
        login->hide();
    }
    setEnabled ( true );
    if ( !embedMode )
    {
        u->hide();
        uname->hide();
    }
    sessionStatusDlg->hide();
    selectSessionDlg->hide();
    setEnabled ( true );
    if ( isPassShown )
    {
        passForm->show();
        passForm->setEnabled ( true );
    }
    isPassShown=true;
    login->setEnabled ( true );
    if ( login->text().length() >0 )
    {
        pass->setFocus();
        pass->selectAll();
    }
    else
        login->setFocus();


    if ( !embedMode )
    {
        u->setEnabled ( true );
    }
    else
    {
        if ( config.user.length() >0 )
            login->setEnabled ( false );
    }
}


void ONMainWindow::showSessionStatus()
{
    setStatStatus();
}


void ONMainWindow::slotShowAdvancedStat()
{
    if ( !miniMode )
    {
        if ( sbAdv->isChecked() )
        {
            sessionStatusDlg->setFixedSize (
                sessionStatusDlg->width(),
                sessionStatusDlg->height() *2 );
        }
        else
        {
            sessionStatusDlg->setFixedSize (
                sessionStatusDlg->sizeHint() );
            stInfo->hide();
        }
    }
    else
    {
        if ( sbAdv->isChecked() )
        {
            sessionStatusDlg->setFixedSize ( 310,300 );
        }
        else
        {
            stInfo->hide();
            sessionStatusDlg->setFixedSize ( 310,200 );
        }
    }


// 	username->invalidate();


    if ( sbAdv->isChecked() )
    {
        stInfo->show();
    }

    X2goSettings st ( "settings" );
    st.setting()->setValue ( "showStatus",
                             ( QVariant ) sbAdv->isChecked() );
    st.setting()->sync();
}




void ONMainWindow::slotResumeDoubleClick ( const QModelIndex& )
{
    if ( !shadowSession )
        slotResumeSess();
}


void ONMainWindow::suspendSession ( QString sessId )
{
    SshProcess* proc=0l;
    proc=new SshProcess ( sshConnection, this );

    connect ( proc,SIGNAL ( sshFinished ( bool,  QString,SshProcess* ) ),
              this,SLOT ( slotRetSuspSess ( bool,  QString,
                                            SshProcess* ) ) );

    proc->startNormal ( "x2gosuspend-session "+sessId );
}


bool ONMainWindow::termSession ( QString sessId, bool warn )
{
    if ( warn )
    {
        bool hide_after=false;
        if (isHidden())
        {
            showNormal();
            hide_after=true;
        }
        int answer=QMessageBox::warning (
                       this,tr ( "Warning" ),
                       tr (
                           "Are you sure you want to terminate "
                           "this session?\n"
                           "Unsaved documents will be lost" ),
                       QMessageBox::Yes,QMessageBox::No );
        if (hide_after)
            hide();

        if ( answer !=
                QMessageBox::Yes )
        {
            slotRetTermSess ( true,QString::null,0 );
            return false;
        }
    }
    if ( shadowSession )
    {
        nxproxy->terminate();
        return true;
    }

    SshProcess* proc=0l;
    proc=new SshProcess ( sshConnection,  this );

    connect ( proc,SIGNAL ( sshFinished ( bool,  QString,SshProcess* ) ),
              this,SLOT ( slotRetTermSess ( bool,
                                            QString,SshProcess* ) ) );

    proc->startNormal ( "x2goterminate-session "+sessId );
    return true;
}



void ONMainWindow::setStatStatus ( QString status )
{
    setEnabled ( true );
    passForm->hide();
    selectSessionDlg->hide();
    if ( status == QString::null )
        status=statusString;
    else
        statusString=status;
    QString tstr;
    if ( statusLabel )
        statusLabel->setText ( QString::null );
    if ( resumingSession.sessionId!=QString::null )
    {
        QString f="dd.MM.yy HH:mm:ss";
        QDateTime dt=QDateTime::fromString ( resumingSession.crTime,f );
        dt=dt.addYears ( 100 );
        tstr=dt.toString();
    }
    if ( !embedMode || !proxyWinEmbedded )
    {
        statusBar()->showMessage ( "");
#if ! (defined Q_OS_WIN && defined CFGPLUGIN)
        statusBar()->hide();
#endif
        QString srv;
        if ( embedMode )
            srv=config.server;
        else
            srv=resumingSession.server;
        slVal->setText ( resumingSession.sessionId+"\n"+
                         srv+"\n"+
                         getCurrentUname() +"\n"+
                         resumingSession.display+
                         "\n"+tstr+"\n"+status );

        slVal->setFixedSize ( slVal->sizeHint() );
        sessionStatusDlg->show();
    }
    else
    {

        QString srv=config.server;
        QString message=getCurrentUname() +"@"+
                        srv+
                        ", "+tr ( "Session" ) +": "+
                        resumingSession.sessionId+", "+
                        tr ( "Display" ) +": "+
                        resumingSession.display+", "+
                        tr ( "Creation time" ) +": "+tstr;
#if ! (defined Q_OS_WIN && defined CFGPLUGIN)
        if ( statusLabel )
        {
            statusLabel->setText ( "   "+message );
        }
        else
#endif
        {
            if ( config.showstatusbar )
            {
                statusBar()->show();
                statusBar()->showMessage ( message );
            }
        }
        sessionStatusDlg->hide();
    }
}


void ONMainWindow::slotRestartProxy()
{
    if ( !sessionStatusDlg->isVisible() )
        return;
    if ( stInfo->toPlainText().indexOf (
                "Established X server connection" ) ==-1 )
    {
        stInfo->insertPlainText (
            tr (
                "Connection timeout, aborting" ) );
        if ( nxproxy )
            nxproxy->terminate();
        restartResume=true;
    }
}


void ONMainWindow::slotTestSessionStatus()
{

    if ( !sessionStatusDlg->isVisible() )
        return;
    if ( stInfo->toPlainText().indexOf (
                "Established X server connection" ) ==-1 )
    {
        stInfo->insertPlainText (
            tr ( "Connection timeout, aborting" ) );
        if ( nxproxy )
            nxproxy->terminate();
    }
}

