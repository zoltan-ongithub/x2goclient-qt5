/***************************************************************************
*   Copyright (C) 2005-2011 by Oleksandr Shneyder   *
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


x2goSession ONMainWindow::getNewSessionFromString ( const QString& string )
{
    QStringList lst=string.split ( '|' );
    x2goSession s;
    s.display=lst[0];
    s.cookie=lst[1];
    s.agentPid=lst[2];
    s.sessionId=lst[3];
    s.grPort=lst[4];
    s.sndPort=lst[5];
    if ( lst.count() >6 )
        s.fsPort=lst[6];
    return s;
}


void ONMainWindow::runCommand()
{
    QString passwd=getCurrentPass();
    QString user=getCurrentUname();
    QString host=resumingSession.server;
    QString command;
    QString sessionType="D";
    QString rdpOpts,rdpServer;
    bool rdpFS=false;
    QString rdpWidth;
    QString rdpHeight;
    bool rootless=false;
    if ( !embedMode )
    {
        X2goSettings st ( "sessions" );


        if ( useLdap )
            command=sessionCmd;
        else
        {
            QString sid=lastSession->id();
            command=st.setting()->value (
                        sid+"/command",
                        ( QVariant ) tr ( "KDE" ) ).toString();
            rdpOpts=st.setting()->value (
                        sid+"/rdpoptions",
                        ( QVariant ) "" ).toString();
            rdpServer=st.setting()->value (
                          sid+"/rdpserver",
                          ( QVariant ) "" ).toString();
            rootless=st.setting()->value ( sid+"/rootless",
                                           ( QVariant ) false ).toBool();

            rdpFS=st.setting()->value (
                      sid+"/fullscreen",
                      ( QVariant ) defaultFullscreen ).toBool();
            rdpHeight=st.setting()->value (
                          sid+"/height",
                          ( QVariant ) defaultHeight ).toString();
            rdpWidth=st.setting()->value (
                         sid+"/width",
                         ( QVariant ) defaultWidth ).toString();

        }
    }
    else
    {
        command=config.command;
        rootless=config.rootless;
    }
    if ( rootless )
        sessionType="R";

    if ( command=="KDE" )
    {
        command="startkde";
    }
    else if ( command=="GNOME" )
    {
        command="gnome-session";
    }
    else if ( command=="LXDE" )
    {
        command="startlxde";
    }
    else if ( command=="RDP" )
    {
        command="rdesktop ";
        if ( rdpFS )
            command+=" -f ";
        else
            command+=" -g "+rdpWidth+"x"+rdpHeight;
        command+=" "+rdpOpts+ " "+rdpServer;

        sessionType="R";
    }

    if ( managedMode )
        return;

    SshProcess *proc=0l;

    QString cmd;

    command.replace ( " ","X2GO_SPACE_CHAR" );

    if ( !startSessSound  || startSessSndSystem==PULSE )
    {
        cmd="setsid x2goruncommand "+resumingSession.display+" "+
            resumingSession.agentPid + " " +
            resumingSession.sessionId+" "+
            resumingSession.sndPort+ " "+ command+" nosnd "+
            sessionType +">& /dev/null & exit";
        if ( startSessSndSystem ==PULSE )
        {
            cmd="PULSE_CLIENTCONFIG=~/.x2go/C-"+
                resumingSession.sessionId+
                "/.pulse-client.conf "+cmd;
        }
    }
    else
    {
        switch ( startSessSndSystem )
        {
        case ESD:
            cmd="setsid x2goruncommand "+
                resumingSession.display+" "+
                resumingSession.agentPid + " " +
                resumingSession.sessionId+" "+
                resumingSession.sndPort+ " "+
                command+" esd "+
                sessionType +">& /dev/null & exit";
            break;
        case ARTS:
            cmd="setsid x2goruncommand "+
                resumingSession.display+" "+
                resumingSession.agentPid + " " +
                resumingSession.sessionId+" "+
                resumingSession.sndPort+ " "+
                command+" arts "+
                sessionType +">& /dev/null & exit";
            break;

        }
    }

    if ( runRemoteCommand )
    {
        proc=new SshProcess ( sshConnection, this );
        connect ( proc,SIGNAL ( sshFinished ( bool, QString,
                                              SshProcess* ) ),
                  this,SLOT ( slotRetRunCommand ( bool,
                                                  QString,
                                                  SshProcess* ) ) );

        proc->startNormal ( cmd );
    }
#ifdef Q_WS_HILDON
    //wait 5 seconds and execute xkbcomp
    QTimer::singleShot ( 5000, this, SLOT ( slotExecXmodmap() ) );
#endif
}

void ONMainWindow::slotRetRunCommand ( bool result, QString output,
                                       SshProcess* proc )
{
    if ( proc )
        delete proc;
    if ( result==false )
    {
        QString message=tr ( "<b>Connection failed</b>\n:\n" ) +output;
        if ( message.indexOf ( "publickey,password" ) !=-1 )
        {
            message=tr ( "<b>Wrong password!</b><br><br>" ) +
                    message;
        }
        QMessageBox::critical ( 0l,tr ( "Error" ),message,
                                QMessageBox::Ok,
                                QMessageBox::NoButton );
    }
}


bool ONMainWindow::parseParameter ( QString param )
{
    if ( param=="--help" )
    {
        showHelp();
        return false;
    }

    if ( param=="--help-pack" )
    {
        showHelpPack();
        return false;
    }

    if ( param == "--portable" )
    {
        ONMainWindow::portable=true;
        return true;
    }

    if ( param=="--no-menu" )
    {
        drawMenu=false;
        return true;
    }

    if ( param=="--maximize" )
    {
        startMaximized=true;
        return true;
    }
    if ( param=="--hide" )
    {
        startHidden=true;
        return true;
    }
    if ( param=="--pgp-card" )
    {
        usePGPCard=true;
        return true;
    }
    if ( param=="--ldap-printing" )
    {
        LDAPPrintSupport=true;
        return true;
    }
    if ( param=="--add-to-known-hosts" )
    {
        acceptRsa=true;
        return true;
    }

    QString setting,value;
    QStringList vals=param.split ( "=" );
    if ( vals.size() <2 )
    {
        printError ( param );
        return false;
    }
    setting=vals[0];
    vals.removeFirst();
    value=vals.join ( "=" );
    if ( setting=="--link" )
    {
        return linkParameter ( value );
    }
    if ( setting=="--sound" )
    {
        return soundParameter ( value );
    }
    if ( setting=="--geometry" )
    {
        return geometry_par ( value );
    }
    if ( setting=="--pack" )
    {
        return packParameter ( value );
    }
    if ( setting=="--kbd-layout" )
    {
        defaultKbdType=value;
        return true;
    }
    if ( setting=="--session" )
    {
        defaultSession=true;
        defaultSessionName=value;
        return true;
    }
    if ( setting=="--sessionid" )
    {
        defaultSession=true;
        defaultSessionId=value;
        return true;
    }
    if ( setting=="--user" )
    {
        defaultUser=true;
        defaultUserName=value;
        return true;
    }
    if ( setting=="--kbd-type" )
    {
        defaultKbdType=value;
        return true;
    }
    if ( setting=="--set-kbd" )
    {
        return setKbd_par ( value );
    }
    if ( setting=="--ldap" )
    {
        return ldapParameter ( value );
    }
    if ( setting=="--ldap1" )
    {
        return ldap1Parameter ( value );
    }
    if ( setting=="--ldap2" )
    {
        return ldap2Parameter ( value );
    }
    if ( setting=="--command" )
    {
        defaultCmd=value;
        return true;
    }
    if ( setting=="--read-exports-from" )
    {
        readExportsFrom=value;
        return true;
    }
    if ( setting=="--external-login" )
    {
        extLogin=true;
        readLoginsFrom=value;
        return true;
    }
    if ( setting=="--ssh-port" )
    {
        defaultSshPort=value;
        return true;
    }
    if ( setting=="--dpi" )
    {
        defaultSetDPI=true;
        defaultDPI=value.toUInt();
        return true;
    }
    if ( setting=="--client-ssh-port" )
    {
        clientSshPort=value;
        return true;
    }
    if ( setting == "--embed-into" )
    {
        embedMode=true;
        embedParent=value.toLong();
        return true;
    }
    printError ( param );
    return false;
}


bool ONMainWindow::linkParameter ( QString value )
{
    if ( value=="modem" )
        defaultLink=MODEM;
    else if ( value=="isdn" )
        defaultLink=ISDN;
    else if ( value=="adsl" )
        defaultLink=ADSL;
    else if ( value=="wan" )
        defaultLink=WAN;
    else if ( value=="lan" )
        defaultLink=LAN;
    else
    {
        qCritical (
            "%s",tr (
                "wrong value for argument\"--link\""
            ).toLocal8Bit().data() );
        return false;
    }
    return true;

}

bool ONMainWindow::soundParameter ( QString val )
{
    if ( val=="1" )
        defaultUseSound=true;
    else if ( val=="0" )
        defaultUseSound=false;
    else
    {
        qCritical (
            "%s",tr ( "wrong value for "
                      "argument\"--sound\"" ).toLocal8Bit().data() );
        return false;
    }
    return true;
}

bool ONMainWindow::geometry_par ( QString val )
{
    if ( val=="fullscreen" )
        defaultFullscreen=true;
    else
    {
        QStringList res=val.split ( "x" );
        if ( res.size() !=2 )
        {
            qCritical (
                "%s",tr (
                    "wrong value for argument\"--geometry\"" ).
                toLocal8Bit().data() );
            return false;
        }
        bool o1,o2;
        defaultWidth=res[0].toInt ( &o1 );
        defaultHeight=res[1].toInt ( &o2 );
        if ( ! ( defaultWidth >0 && defaultHeight >0 && o1 && o2 ) )
        {
            qCritical (
                "%s",tr (
                    "wrong value for argument\"--geometry\"" ).
                toLocal8Bit().data() );
            return false;
        }
    }
    return true;
}

bool ONMainWindow::setKbd_par ( QString val )
{
    if ( val=="1" )
        defaultSetKbd=true;
    else if ( val=="0" )
        defaultSetKbd=false;
    else
    {
        qCritical (
            "%s",tr (
                "wrong value for argument\"--set-kbd\"" ).
            toLocal8Bit().data() );
        return false;
    }
    return true;
}

bool ONMainWindow::ldapParameter ( QString val )
{
    QString ldapstring=val;
    useLdap=true;
    ldapstring.replace ( "\"","" );
    QStringList lst=ldapstring.split ( ':',QString::SkipEmptyParts );
    if ( lst.size() !=3 )
    {
        qCritical (
            "%s",tr (
                "wrong value for argument\"--ldap\"" ).
            toLocal8Bit().data() );
        return false;
    }
    ldapOnly=true;
    ldapServer=lst[0];
    ldapPort=lst[1].toInt();
    ldapDn=lst[2];


    return true;
}

bool ONMainWindow::ldap1Parameter ( QString val )
{
    QString ldapstring=val;
    ldapstring.replace ( "\"","" );
    QStringList lst=ldapstring.split ( ':',QString::SkipEmptyParts );
    if ( lst.size() !=2 )
    {
        qCritical (
            "%s",tr (
                "wrong value for argument\"--ldap1\"" ).
            toLocal8Bit().data() );
        return false;
    }
    ldapServer1=lst[0];
    ldapPort1=lst[1].toInt();

    return true;
}

bool ONMainWindow::ldap2Parameter ( QString val )
{
    QString ldapstring=val;
    ldapstring.replace ( "\"","" );
    QStringList lst=ldapstring.split ( ':',QString::SkipEmptyParts );
    if ( lst.size() !=2 )
    {
        qCritical ( "%s",
                    tr (
                        "wrong value for argument\"--ldap2\"" ).
                    toLocal8Bit().data() );
        return false;
    }
    ldapServer2=lst[0];
    ldapPort2=lst[1].toInt();

    return true;
}


bool ONMainWindow::packParameter ( QString val )
{

    QFile file ( ":/txt/packs" );
    if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
        return true;
    QTextStream in ( &file );
    while ( !in.atEnd() )
    {
        QString pc=in.readLine();
        if ( pc.indexOf ( "-%" ) !=-1 )
        {
            pc=pc.left ( pc.indexOf ( "-%" ) );

            QStringList pctails=val.split ( "-" );
            QString pcq=pctails[pctails.size()-1];
            pctails.removeLast();

            if ( pctails.join ( "-" ) ==pc )
            {
                bool ok;
                int v=pcq.toInt ( &ok );
                if ( ok && v>=0 && v<=9 )
                {
                    defaultPack=pc;
                    defaultQuality=v;
                    return true;
                }
                else
                    break;
            }
        }
        else
        {
            if ( pc==val )
            {
                defaultPack=val;
                return true;
            }
        }
    }
    file.close();
    qCritical ( "%s",tr ( "wrong value for argument\"--pack\"" ).
                toLocal8Bit().data() );
    return false;
}


void ONMainWindow::printError ( QString param )
{
    qCritical ( "%s", ( tr ( "wrong parameter: " ) +param ).
                toLocal8Bit().data() );
#ifdef Q_OS_WIN
    x2goDebug<<tr ( "wrong parameter: " ) +param <<endl;
#endif
}

void ONMainWindow::showHelp()
{
    QString helpMsg=
        "Usage: x2goclient [Options]\n"
        "Options:\n"
        "--help\t\t\t\t show this message\n"
        "--help-pack\t\t\t show available pack methods\n"
        "--no-menu\t\t\t hide menu bar\n"
        "--maximize\t\t\t start maximized\n"
        "--hide\t\t\t\t start hidden\n"
        "--portable\t\t\t start in \"portable\" mode\n"
        "--pgp-card\t\t\t use openPGP card authentication\n"
        "--ldap-printing\t\t\t allow client side printing in LDAP mode\n"
        "--add-to-known-hosts\t\t add RSA key fingerprint to "
        ".ssh/known_hosts\n"
        "\t\t\t\t if authenticity of server can't be established\n\n"
        "--ldap=<host:port:dn> \t\t start with LDAP support. Example:\n"
        "\t\t\t\t --ldap=ldapserver:389:o=organization,c=de\n\n"
        "--ldap1=<host:port>\t\t LDAP failover server #1 \n"
        "--ldap2=<host:port>\t\t LDAP failover server #2 \n"
        "--ssh-port=<port>\t\t connect to this port, default 22\n"
        "--client-ssh-port=<port>\t local ssh port (for fs export), "
        "default 22\n"
        "--command=<cmd>\t\t\t Set default command, default value 'KDE'\n"
        "--session=<session>\t\t Start session 'session'\n"
        "--user=<username>\t\t in LDAP mode, select user 'username'\n"
        "--geomerty=<W>x<H>|fullscreen\t set default geometry, default "
        "value '800x600'\n"
        "--dpi=<dpi>\t\t\t set dpi of x2goagent to dpi, default not set\n"
        "--link=<modem|isdn|adsl|wan|lan> set default link type, "
        "default 'adsl'\n"
        "--pack=<packmethod>\t\t set default pack method, default "
        "'16m-jpeg-9'\n"
        "--kbd-layout=<layout>\t\t set default keyboard layout\n"
        "--kbd-type=<typed>\t\t set default keyboard type\n"
        "--set-kbd=<0|1>\t\t\t overwrite current keyboard settings\n" ;
    qCritical ( "%s",helpMsg.toLocal8Bit().data() );
    QMessageBox::information ( this,tr ( "Options" ),helpMsg );
}

void ONMainWindow::showHelpPack()
{
    qCritical ( "%s",tr (
                    "Available pack methodes:" ).toLocal8Bit().data() );
    QFile file ( ":/txt/packs" );
    if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
        return;
    QTextStream in ( &file );
    QString msg;
    while ( !in.atEnd() )
    {
        QString pc=in.readLine();
        if ( pc.indexOf ( "-%" ) !=-1 )
        {
            pc=pc.left ( pc.indexOf ( "-%" ) );
            pc+="-[0-9]";
        }
        msg+=pc+"\n";
        qCritical ( "%s",pc.toLocal8Bit().data() );
    }
    file.close();
#ifdef Q_OS_WIN

    QMessageBox::information ( this,tr ( "Options" ),msg );
#endif

}

void ONMainWindow::slotGetServers ( bool result, QString output,
                                    SshProcess* proc )
{
    if ( proc )
        delete proc;
    proc=0;
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
        pass->setFocus();
        pass->selectAll();
        return;
    }

    passForm->hide();
    setUsersEnabled ( false );
    uname->setEnabled ( false );
    u->setEnabled ( false );
    QStringList servers=output.trimmed().split ( '\n' );
    for ( int i=0;i<servers.size();++i )
    {
        QStringList lst=servers[i].simplified().split ( ' ' );
        if ( lst.size() >1 )
        {
            for ( int j=0;j<x2goServers.size();++j )
                if ( x2goServers[j].name==lst[0] )
                {
                    x2goServers[j].sess=
                        lst[1].toInt() *
                        x2goServers[j].factor;
                    x2goDebug<<x2goServers[j].name<<
                    ": sessions "<<
                    lst[1].toInt() <<
                    ", multiplied "<<x2goServers[j].sess;
                    break;
                }
        }
    }

    qSort ( x2goServers.begin(),x2goServers.end(),serv::lt );

    listedSessions.clear();
    retSessions=0;
// TODO: should use x2golistsessions --all-servers to create less ssh sessions
    for ( int j=0;j<x2goServers.size();++j )
    {
        QString passwd;
        QString user=getCurrentUname();
        QString host=x2goServers[j].name;
        passwd=getCurrentPass();

        SshProcess* lproc;
        lproc=new SshProcess ( sshConnection,  this );
        connect ( lproc,SIGNAL ( sshFinished ( bool,
                                               QString,SshProcess* ) ),
                  this,SLOT (
                      slotListAllSessions ( bool,
                                            QString,SshProcess* ) ) );
        lproc->startNormal ( "export HOSTNAME && x2golistsessions" );
    }
}


void ONMainWindow::slotListAllSessions ( bool result,QString output,
        SshProcess* proc )
{
    bool last=false;
    ++retSessions;
    if ( retSessions == x2goServers.size() )
        last=true;
    if ( proc )
        delete proc;
    proc=0;
    if ( result==false )
    {
        QString message=tr ( "<b>Connection failed</b>\n" ) +output;
        if ( message.indexOf ( "publickey,password" ) !=-1 )
        {
            message=tr ( "<b>Wrong password!</b><br><br>" ) +
                    message;
        }

        QMessageBox::critical ( 0l,tr ( "Error" ),message,
                                QMessageBox::Ok,
                                QMessageBox::NoButton );
        QString sv=output.split ( ":" ) [0];
        for ( int j=0;j<x2goServers.size();++j )
        {
            if ( x2goServers[j].name==sv )
            {
                x2goServers[j].connOk=false;
            }
        }
    }
    else
    {
        listedSessions+=output.trimmed().split ( '\n',
                        QString::SkipEmptyParts );
    }
    if ( last )
    {
        if ( listedSessions.size() ==0||
                ( listedSessions.size() ==1 &&
                  listedSessions[0].length() <5 ) )
        {
// 			x2goDebug<<"start New Session";
            startNewSession();
        }
        else if ( listedSessions.size() ==1 )
        {
// 			x2goDebug<<"have one session";
            x2goSession s=getSessionFromString (
                              listedSessions[0] );
// 			x2goDebug<<"will procceed one session";
            QDesktopWidget wd;
            if ( s.status=="S" && isColorDepthOk (
                        wd.depth(),s.colorDepth ) )
                resumeSession ( s );
            else
            {
// 				x2goDebug<<"select one Session";
                selectSession ( listedSessions );
            }
        }
        else
        {
            selectSession ( listedSessions );
        }
    }
}

void ONMainWindow::slotResize()
{
    if ( startHidden )
    {
        hide();
        return;
    }
    if ( !startMaximized && !mwMax )
    {
        resize ( mwSize );
        move ( mwPos );
        show();
    }
    else
        showMaximized();
}

void ONMainWindow::slotExportDirectory()
{
    if ( shadowSession )
        return;

    bool hide_after=false;
    if(isHidden())
    {
      showNormal();
      hide_after=true;
    }
    QString path;
    if ( !useLdap && !embedMode )
    {
        ExportDialog dlg ( lastSession->id(),this );
        if ( dlg.exec() ==QDialog::Accepted )
            path=dlg.getExport();
    }
    else
           
        path= QFileDialog::getExistingDirectory (
                  this,QString::null,
                  homeDir );
     if(hide_after)
       hide();
#ifdef Q_OS_WIN
    if ( ONMainWindow::getPortable() &&
            ONMainWindow::U3DevicePath().length() >0 )
    {
        path.replace ( "(U3)",u3Device );
    }

    path=cygwinPath ( wapiShortFileName ( path ) );
#endif
    if ( path!=QString::null )
        exportDirs ( path );
}


void ONMainWindow::exportDirs ( QString exports,bool removable )
{
    if ( shadowSession )
        return;
    if ( embedMode )
    {
        if ( config.confFS  && ! ( config.useFs ) )
        {
            return;
        }
    }
    fsExportKeyReady=false;
    directory dr;


    dr.dirList=exports;
    dr.key=createRSAKey();
    QString passwd;
    x2goDebug<<"key created on: "<<dr.key;

    passwd=getCurrentPass();

    fsInTun=false;
    if ( !useLdap )
    {
        if ( !embedMode )
        {
            X2goSettings st ( "sessions" );

            QString sid=lastSession->id();

            fsInTun=st.setting()->value ( sid+"/fstunnel",
                                          ( QVariant ) true ).toBool();
        }
        else
            fsInTun=true;
    }
    if ( fsInTun )
    {
        if ( fsTunnel==0l )
            if ( startSshFsTunnel() )
                return;
    }
    SshProcess* lproc;
    QString uname=getCurrentUname();
    lproc=new SshProcess ( sshConnection, this );
    connect ( lproc,SIGNAL ( sshFinished ( bool,QString,SshProcess* ) ),
              this,SLOT ( slotCopyKey ( bool, QString,SshProcess* ) ) );
    QString dst=dr.key;
    QString dhdir=homeDir+"/.x2go";
#ifdef Q_OS_WIN
    dhdir=wapiShortFileName ( dhdir );
#endif
    dst.replace ( dhdir +"/ssh/gen/","" );
    dst="~"+uname +"/.x2go/ssh/"+dst;
    dr.dstKey=dst;
    dr.isRemovable=removable;
    exportDir.append ( dr );
    QString keyFile=dr.key;
    lproc->start_cp ( keyFile,dst );

}


void ONMainWindow::exportDefaultDirs()
{
    QStringList dirs;
    bool clientPrinting= ( useLdap && LDAPPrintSupport );

    if ( !useLdap )
    {
        if ( !embedMode )
        {

            X2goSettings st ( "sessions" );
            clientPrinting= st.setting()->value (
                                lastSession->id() +
                                "/print", true ).toBool();

            QString exd=st.setting()->value (
                            lastSession->id() +"/export",
                            ( QVariant ) QString::null ).toString();
            QStringList lst=exd.split ( ";",
                                        QString::SkipEmptyParts );
            for ( int i=0;i<lst.size();++i )
            {
#ifndef Q_OS_WIN
                QStringList tails=lst[i].split (
                                      ":",
                                      QString::SkipEmptyParts );
#else

                QStringList tails=lst[i].split (
                                      "#",
                                      QString::SkipEmptyParts );
#endif

                if ( tails[1]=="1" )
                {
#ifdef Q_OS_WIN
                    if ( ONMainWindow::getPortable() &&
                            ONMainWindow::U3DevicePath (
                            ).length() >0 )
                    {
                        tails[0].replace (
                            "(U3)",u3Device );
                    }

                    tails[0]=cygwinPath (
                                 wapiShortFileName (
                                     tails[0] ) );
#endif
                    dirs+=tails[0];
                }
            }
        }
        else
        {
            clientPrinting=true;
            if ( config.confFS )
            {
                clientPrinting=config.useFs;
            }
        }
    }

    if ( clientPrinting )
    {
        QString path= homeDir +
                      "/.x2go/S-"+
                      resumingSession.sessionId +"/spool";
        QDir spooldir;
        if ( !spooldir.exists ( path ) )
        {
            if ( !spooldir.mkpath ( path ) )
            {
                QString message=
                    tr (
                        "Unable to create folder:" ) + path;
                QMessageBox::critical ( 0l,tr (
                                            "Error" ),message,
                                        QMessageBox::Ok,
                                        QMessageBox::NoButton );

            }
        }
        spoolDir=path;
#ifdef Q_OS_WIN
        path=cygwinPath (
                 wapiShortFileName (
                     path ) );
#endif
        QFile::setPermissions (
            path,QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner );

        path+="__PRINT_SPOOL_";
        dirs+=path;
        printSupport=true;
        if ( spoolTimer )
            delete spoolTimer;
        spoolTimer=new QTimer ( this );
        connect ( spoolTimer,SIGNAL ( timeout() ),this,
                  SLOT ( slotCheckPrintSpool() ) );
        spoolTimer->start ( 2000 );
    }
    if ( dirs.size() <=0 )
        return;
    exportDirs ( dirs.join ( ":" ) );
}

QString ONMainWindow::createRSAKey()
{
    QDir dr;
    QString keyPath=homeDir +"/.x2go/ssh/gen";
    dr.mkpath ( keyPath );
#ifdef Q_OS_WIN
    keyPath=wapiShortFileName ( keyPath );
#endif
    QTemporaryFile fl ( keyPath+"/key" );
    fl.open();
    QString keyName=fl.fileName();
    fl.setAutoRemove ( false );
    fl.close();
    fl.remove();

    QStringList args;

    args<<"-t"<<"rsa"<<"-b"<<"1024"<<"-N"<<""<<"-f"<<keyName;
// 	x2goDebug <<keyName<<endl;

    if ( QProcess::execute ( "ssh-keygen",args ) !=0 )
    {
        x2goDebug <<"ssh-keygen failed" <<endl;
        return QString::null;
    }
// 	x2goDebug <<"ssh-keygen ok" <<endl;

    QFile rsa ( "/etc/ssh/ssh_host_rsa_key.pub" );
#ifdef Q_OS_WIN
    rsa.setFileName (
        wapiShortFileName (
            homeDir+"\\.x2go\\etc\\ssh_host_dsa_key.pub" ) );
#else
    if ( userSshd )
        rsa.setFileName ( homeDir+"/.x2go/etc/ssh_host_dsa_key.pub" );

#endif

#ifdef Q_OS_DARWIN
    rsa.setFileName ( "/etc/ssh_host_rsa_key.pub" );
#endif
    if ( !rsa.open ( QIODevice::ReadOnly | QIODevice::Text ) )
    {
#ifdef Q_OS_LINUX
        generateHostDsaKey();
        generateEtcFiles();
        startSshd();
        rsa.setFileName ( homeDir+"/.x2go/etc/ssh_host_dsa_key.pub" );
        rsa.open ( QIODevice::ReadOnly | QIODevice::Text );
#else
        printSshDError();
        return QString::null;
#endif
    }

    QByteArray rsa_pub;

    if ( !rsa.atEnd() )
        rsa_pub = rsa.readLine();
    else
    {
        x2goDebug<<"error: rsa file empty";
        return QString::null;
    }

    QFile file ( keyName );
    if ( !file.open (
                QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append )
       )
    {
        x2goDebug<<"error openning key:"<<keyName;
        return keyName;
    }
    QTextStream out ( &file );
    out<<"----BEGIN RSA IDENTITY----"<<rsa_pub;
    file.close();
    return keyName;
}

void ONMainWindow::slotCopyKey ( bool result, QString output, SshProcess* proc )
{
    fsExportKey=proc->getSource();
    if ( proc )
        delete proc;
    proc=0;
    x2goDebug<<"exported key "<<fsExportKey;
    QFile::remove ( fsExportKey );
    x2goDebug<<"key removed";
    if ( result==false )
    {
        QString message=tr ( "<b>Connection failed</b>\n" ) +output;
        if ( message.indexOf ( "publickey,password" ) !=-1 )
        {
            message=tr ( "<b>Wrong password!</b><br><br>" ) +
                    message;
        }

        QMessageBox::critical ( 0l,tr ( "Error" ),message,
                                QMessageBox::Ok,
                                QMessageBox::NoButton );
        QFile::remove ( fsExportKey+".pub" );
        return;
    }
    fsExportKeyReady=true;

    //start reverse mounting if RSA Key and FS tunnel are ready
    //start only once from slotFsTunnelOk() or slotCopyKey().
    if ( !fsInTun || fsTunReady )
        startX2goMount();

}

directory* ONMainWindow::getExpDir ( QString key )
{
    for ( int i=0;i<exportDir.size();++i )
    {
        if ( exportDir[i].key==key )
            return &exportDir[i];
    }
    return 0l;
}




void ONMainWindow::slotRetExportDir ( bool result,QString output,
                                      SshProcess* proc )
{

    QString key;
    for ( int i=0;i<exportDir.size();++i )
        if ( exportDir[i].proc==proc )
        {
            key=exportDir[i].key;
            exportDir.removeAt ( i );
            break;
        }

    if ( proc )
        delete proc;

    if ( result==false )
    {
        QString message=tr ( "<b>Connection failed</b>\n" ) +output;
        if ( message.indexOf ( "publickey,password" ) !=-1 )
        {
            message=tr ( "<b>Wrong password!</b><br><br>" ) +
                    message;
        }

        QMessageBox::critical ( 0l,tr ( "Error" ),message,
                                QMessageBox::Ok,
                                QMessageBox::NoButton );
    }
    QFile file ( key+".pub" );
    if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        printSshDError();
        QFile::remove
        ( key+".pub" );
        return;
    }

    QByteArray line = file.readLine();
    file.close();
    QString authofname=homeDir;
#ifdef Q_OS_WIN
    QDir dir;
    dir.mkpath ( authofname+"\\.x2go\\.ssh" );
    x2goDebug<<"Creating "<<authofname+"\\.x2go\\.ssh"<<endl;
    authofname=wapiShortFileName ( authofname ) +"/.x2go";
#endif
    authofname+="/.ssh/authorized_keys" ;
    file.setFileName ( authofname );
    if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        printSshDError();
        QFile::remove
        ( key+".pub" );
        return;
    }


    QTemporaryFile tfile ( authofname );
    tfile.open();
    tfile.setAutoRemove ( true );
    QTextStream out ( &tfile );

    while ( !file.atEnd() )
    {
        QByteArray newline = file.readLine();
        if ( newline!=line )
            out<<newline;
    }
    file.close();
    tfile.close();
    file.remove();
    tfile.copy ( authofname );
    QFile::remove
    ( key+".pub" );
}



void ONMainWindow::exportsEdit ( SessionButton* bt )
{
    EditConnectionDialog dlg ( bt->id(),this,3 );
    if ( dlg.exec() ==QDialog::Accepted )
    {
        bt->redraw();
        bool vis=bt->isVisible();
        placeButtons();
        users->ensureVisible ( bt->x(),bt->y(),50,220 );
        bt->setVisible ( vis );
    }
}


void ONMainWindow::slotExtTimer()
{

    if ( QFile::permissions ( readLoginsFrom ) !=
            ( QFile::ReadUser|QFile::WriteUser|QFile::ExeUser|
              QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner ) )
    {
        x2goDebug <<"Wrong permissions on "<<readLoginsFrom <<":"<<endl;
        x2goDebug << ( int ) ( QFile::permissions (
                                   readLoginsFrom+"/." ) )
        <<"must be"<< ( int ) ( QFile::ReadUser|QFile::WriteUser
                                |QFile::ExeUser|QFile::ReadOwner|
                                QFile::WriteOwner|
                                QFile::ExeOwner ) <<endl;
        if ( extLogin )
            extTimer->stop();
        return;
    }
    QString loginDir;
    QString logoutDir;
    QDir dir ( readLoginsFrom );
    QStringList list = dir.entryList ( QDir::Files );
    for ( int i=0;i<list.size();++i )
    {
        QFile file ( readLoginsFrom+"/"+list[i] );
        if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
            continue;
        if ( !file.atEnd() )
        {
            QByteArray line = file.readLine();
            QString ln ( line );
            QStringList args=ln.split ( "=",
                                        QString::SkipEmptyParts );
            if ( args.size() >1 )
            {
                if ( args[0]=="login" )
                {
                    args[1].replace ( "\n","" );
                    if ( args[1].size() )
                        loginDir=args[1];
                }
                if ( args[0]=="logout" )
                {
                    x2goDebug <<
                    " I HAVE external logout"<<
                    endl;
                    args[1].replace ( "\n","" );
                    if ( args[1].size() )
                        logoutDir=args[1];
                }
            }
        }
        file.close();
        file.remove();
    }
    if ( exportTimer->isActive() ) //running session
    {
        if ( logoutDir != QString::null )
        {
            x2goDebug <<"external logout"<<endl;
            externalLogout ( logoutDir );
        }
    }
    else
    {
        if ( loginDir != QString::null )
        {
            x2goDebug <<"external login"<<endl;
            externalLogin ( loginDir );
        }
    }
}


void ONMainWindow::slotExportTimer()
{

    if ( QFile::permissions ( readExportsFrom ) != ( QFile::ReadUser|
            QFile::WriteUser|
            QFile::ExeUser|
            QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner ) )
    {
        x2goDebug <<"Wrong permissions on "<<
        readExportsFrom <<":"<<endl;
        x2goDebug << ( int ) ( QFile::permissions (
                                   readExportsFrom+"/." ) )
        <<"must be"<< ( int ) ( QFile::ReadUser|QFile::WriteUser
                                |QFile::ExeUser|QFile::ReadOwner|
                                QFile::WriteOwner|
                                QFile::ExeOwner ) <<endl;
        exportTimer->stop();
        return;
    }

    QDir dir ( readExportsFrom );
    QStringList list = dir.entryList ( QDir::Files );
    QString expList;
    QString unexpList;
    QString loginDir;
    QString logoutDir;
    for ( int i=0;i<list.size();++i )
    {
        QFile file ( readExportsFrom+"/"+list[i] );
        if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
            continue;
        if ( !file.atEnd() )
        {
            QByteArray line = file.readLine();
            QString ln ( line );
            QStringList args=ln.split ( "=",
                                        QString::SkipEmptyParts );
            if ( args.size() >1 )
            {
                if ( args[0]=="export" )
                {
                    args[1].replace ( "\n","" );
                    if ( args[1].size() )
                        expList+=":"+args[1];
                }
                if ( args[0]=="unexport" )
                {
                    args[1].replace ( "\n","" );
                    if ( args[1].size() )
                        unexpList+=":"+args[1];
                }
            }
        }
        file.close();
        file.remove();
    }
    QStringList args=expList.split ( ":",QString::SkipEmptyParts );
    expList=args.join ( ":" );
    if ( expList.size() >0 )
    {
        exportDirs ( expList,true );
    }
    args.clear();
    args=unexpList.split ( ":",QString::SkipEmptyParts );

    QString passwd=getCurrentPass();
    QString user=getCurrentUname();
    QString host=resumingSession.server;
    QString sessionId=resumingSession.sessionId;

    for ( int i=0;i<args.size();++i )
    {
        SshProcess* sproc=new SshProcess (
            sshConnection, this );
        sproc->startNormal ( "export HOSTNAME && x2goumount_session "+
                             sessionId+" "+args[i] );
    }
}

void ONMainWindow::slotAboutQt()
{
    QMessageBox::aboutQt ( this );
}

void ONMainWindow::slotAbout()
{
    QString aboutStr=tr (
                         "</b><br> (C. 2006-2010 <b>obviously nice</b>: "
                         "Oleksandr Shneyder, Heinz-Markus Graesing)<br>" );
    if ( embedMode )
        aboutStr+=tr ( "<br>x2goplugin mode was sponsored by "
                       "<a href=\"http://www.foss-group.de/\">"
                       "FOSS-Group GmbH(Freiburg)</a><br>" );
    aboutStr+=
        tr (
            "<br>Client for use with the X2Go network based "
            "computing environment. This Client will be able "
            "to connect to X2Go server(s) and start, stop, "
            "resume and terminate (running) desktop sessions. "
            "X2Go Client stores different server connections "
            "and may automatically request authentification "
            "data from LDAP directories. Furthermore it can be "
            "used as fullscreen loginscreen (replacement for "
            "loginmanager like xdm). Please visit x2go.org for "
            "further information." );
    QMessageBox::about (
        this,tr ( "About X2GO client" ),
        tr ( "<b>X2Go client V. " ) +VERSION+
        " </b >(Qt - "+qVersion() +")"+
        aboutStr );
}



void ONMainWindow::slotRereadUsers()
{
    if ( !useLdap )
        return;
#ifdef USELDAP

    if ( ld )
    {
        delete ld;
        ld=0;
    }


    if ( ! initLdapSession ( false ) )
    {
        return;
    }


    list<string> attr;
    attr.push_back ( "uidNumber" );
    attr.push_back ( "uid" );


    list<LDAPBinEntry> result;
    try
    {
        ld->binSearch ( ldapDn.toStdString(),attr,
                        "objectClass=posixAccount",result );
    }
    catch ( LDAPExeption e )
    {
        QString message="Exeption in: ";
        message=message+e.err_type.c_str();
        message=message+" : "+e.err_str.c_str();
        QMessageBox::critical ( 0l,tr ( "Error" ),message,
                                QMessageBox::Ok,QMessageBox::NoButton );
        QMessageBox::critical ( 0l,tr ( "Error" ),
                                tr ( "Please check LDAP Settings" ),
                                QMessageBox::Ok,QMessageBox::NoButton );
        slotConfig();
        return;
    }

    list<LDAPBinEntry>::iterator it=result.begin();
    list<LDAPBinEntry>::iterator end=result.end();

    for ( ;it!=end;++it )
    {
        user u;
        QString uin=LDAPSession::getBinAttrValues (
                        *it,"uidNumber" ).front().getData();
        u.uin=uin.toUInt();
        if ( u.uin<firstUid || u.uin>lastUid )
        {
            continue;
        }
        u.uid=LDAPSession::getBinAttrValues (
                  *it,"uid" ).front().getData();
        if ( !findInList ( u.uid ) )
        {
            reloadUsers();
            return;
        }
    }
#endif
}

void ONMainWindow::reloadUsers()
{
    int i;
    for ( i=0;i<names.size();++i )
        names[i]->close();
    for ( i=0;i<sessions.size();++i )
        sessions[i]->close();

    userList.clear();
    sessions.clear();


    loadSettings();
    if ( useLdap )
    {
        act_new->setEnabled ( false );
        act_edit->setEnabled ( false );
        u->setText ( tr ( "Login:" ) );
        QTimer::singleShot ( 1, this, SLOT ( readUsers() ) );
    }
    else
    {
        act_new->setEnabled ( true );
        act_edit->setEnabled ( true );
        u->setText ( tr ( "Session:" ) );
        QTimer::singleShot ( 1, this, SLOT ( slotReadSessions() ) );
    }
    slotResize ( fr->size() );
}


bool ONMainWindow::findInList ( const QString& uid )
{
    for ( int i=0;i<userList.size();++i )
    {
        if ( userList[i].uid==uid )
            return true;
    }
    return false;
}

void ONMainWindow::setUsersEnabled ( bool enable )
{

    if ( useLdap )
    {
        QScrollBar* bar=users->verticalScrollBar();
        bar->setEnabled ( enable );
        int upos=bar->value();
        QDesktopWidget dw;
        int height=dw.screenGeometry ( fr ).height();
        QList<UserButton*>::iterator it;
        QList<UserButton*>::iterator endit=names.end();
        if ( !enable )
        {
            for ( it=names.begin();it!=endit;it++ )
            {
                QPoint pos= ( *it )->pos();
                if ( ( pos.y() >upos-height ) &&
                        ( pos.y() <upos+height ) )
                    ( *it )->setEnabled ( false );
                if ( pos.y() >upos+height )
                    break;
            }
        }
        else
        {
            for ( it=names.begin();it!=endit;it++ )
            {
                if ( ! ( *it )->isEnabled() )
                    ( *it )->setEnabled ( true );
            }
        }
    }
    else
        users->setEnabled ( enable );
}


void ONMainWindow::externalLogin ( const QString& loginDir )
{
    QFile file ( loginDir+"/username" );
    QString user;

    if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
        return;
    QTextStream in ( &file );
    while ( !in.atEnd() )
    {
        user=in.readLine();
        break;
    }
    file.close();


    if ( passForm->isVisible() )
        slotClosePass();
    uname->setText ( user );
    slotUnameEntered();
    currentKey=loginDir+"/dsa.key";
    extStarted=true;
    slotPassEnter();
}


void ONMainWindow::externalLogout ( const QString& )
{
    if ( extStarted )
    {
        extStarted=false;
        currentKey=QString::null;
        if ( nxproxy )
            if ( nxproxy->state() ==QProcess::Running )
                nxproxy->terminate();
    }
}


void ONMainWindow::slotStartPGPAuth()
{
    scDaemon=new QProcess ( this );
    QStringList arguments;
    arguments<<"--multi-server";
    connect ( scDaemon,SIGNAL ( readyReadStandardError() ),this,
              SLOT ( slotScDaemonError() ) );
    connect ( scDaemon,SIGNAL ( readyReadStandardOutput() ),this,
              SLOT ( slotScDaemonOut() ) );
    connect ( scDaemon,SIGNAL ( finished ( int,QProcess::ExitStatus ) ),
              this,
              SLOT (
                  slotScDaemonFinished ( int, QProcess::ExitStatus ) ) );
    scDaemon->start ( "scdaemon",arguments );
    QTimer::singleShot ( 3000, this, SLOT ( slotCheckScDaemon() ) );
    isScDaemonOk=false;
}

void ONMainWindow::slotCheckScDaemon()
{
    if ( !isScDaemonOk )
    {
        scDaemon->kill();
    }
}

void ONMainWindow::slotScDaemonError()
{
    QString stdOut ( scDaemon->readAllStandardError() );
    stdOut=stdOut.simplified();
    x2goDebug<<"SCDAEMON err:"<<stdOut<<endl;
    if ( stdOut.indexOf ( "updating slot" ) !=-1 ||
            stdOut.indexOf ( "updating status of slot" ) !=-1 )
    {
        isScDaemonOk=true;
        //USABLE or PRESENT
        if ( ( stdOut.indexOf ( "0x0002" ) !=-1 ) ||
                ( stdOut.indexOf ( "0x0007" ) !=-1 ) )
        {
            scDaemon->kill();
        }
    }
}

void ONMainWindow::slotScDaemonOut()
{
    QString stdOut ( scDaemon->readAllStandardOutput() );
    stdOut=stdOut.simplified();
    x2goDebug<<"SCDAEMON out:"<<stdOut<<endl;
}

void ONMainWindow::slotScDaemonFinished ( int , QProcess::ExitStatus )
{
    scDaemon=0l;
    if ( isScDaemonOk )
    {
        x2goDebug<<"scDaemon finished"<<endl;
        gpg=new QProcess ( this );
        QStringList arguments;
        arguments<<"--card-status";
        connect ( gpg,SIGNAL ( readyReadStandardError() ),
                  this,SLOT ( slotGpgError() ) );
        connect ( gpg,SIGNAL ( finished ( int,
                                          QProcess::ExitStatus ) ),this,
                  SLOT ( slotGpgFinished ( int,
                                           QProcess::ExitStatus ) ) );
        gpg->start ( "gpg",arguments );
    }
    else
        slotStartPGPAuth();
}



void ONMainWindow::slotGpgError()
{
    QString stdOut ( gpg->readAllStandardError() );
    stdOut=stdOut.simplified();
    x2goDebug<<"GPG err:"<<stdOut<<endl;
    if ( stdOut.indexOf ( "failed" ) !=-1 )
    {
        QMessageBox::critical ( 0l,tr ( "Error" ),
                                tr ( "No valid card found" ),
                                QMessageBox::Ok,
                                QMessageBox::NoButton );
        gpg->kill();
    }
}


void ONMainWindow::slotGpgFinished ( int exitCode,
                                     QProcess::ExitStatus exitStatus )
{
    x2goDebug<<"gpg finished, exit code:"<<exitCode<<" exit status:"<<
    exitStatus<<endl;
    if ( exitStatus==0 )
    {
        QString stdOut ( gpg->readAllStandardOutput() );
        stdOut.chop ( 1 );
        x2goDebug<<"GPG out:"<<stdOut<<endl;
        QStringList lines=stdOut.split ( "\n" );
        QString login;
        QString appId;
        QString authKey;
        for ( int i=0;i<lines.count();++i )
        {
            if ( lines[i].indexOf ( "Application ID" ) !=-1 )
            {
                appId=lines[i].split ( ":" ) [1];
            }
            else if ( lines[i].indexOf ( "Login data" ) !=-1 )
            {
                login=lines[i].split ( ":" ) [1];
            }
            else if ( lines[i].indexOf (
                          "Authentication key" ) !=-1 )
            {
                authKey=lines[i].split ( ":" ) [1];
                break;
            }
        }
        appId=appId.simplified();
        login=login.simplified();
        authKey=authKey.simplified();
        x2goDebug<<"card data: "<<appId<<login<<authKey<<endl;
        if ( login=="[not set]" || authKey == "[none]" )
        {
            x2goDebug<<"Card not configured\n";
            QMessageBox::critical (
                0l,tr ( "Error" ),
                tr (
                    "This card is unknown by X2Go system" ),
                QMessageBox::Ok,
                QMessageBox::NoButton );
            QTimer::singleShot ( 1000, this,
                                 SLOT ( slotStartPGPAuth() ) );
        }
        else
            startGPGAgent ( login,appId );
    }
    else
        QTimer::singleShot ( 1000, this, SLOT ( slotStartPGPAuth() ) );
    gpg=0l;
}



void ONMainWindow::startGPGAgent ( const QString& login, const QString& appId )
{
    QString gpgPath=homeDir +"/.x2goclient/gnupg";
    QDir d;
    cardLogin=login;
    d.mkpath ( gpgPath );
    QFile file ( gpgPath+"/scd-event" );
    if ( !file.open ( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        QMessageBox::critical (
            0l,tr ( "Error" ),
            tr (
                "Unable to create file: " ) +
            gpgPath+"/scd-event"
            ,QMessageBox::Ok,
            QMessageBox::NoButton );
        exit ( -1 );
    }
    QTextStream out ( &file );
    out << "#!/bin/bash\n\n"
    "if [ \"$6\" != \"0x0002\" ] && [ \"$6\" != "
    "\"0x0007\" ]\n\
	then\n\
	kill -9 $_assuan_pipe_connect_pid\n\
	fi"<<endl;
    file.close();
    file.setPermissions ( gpgPath+"/scd-event",
                          QFile::ReadOwner|
                          QFile::WriteOwner|
                          QFile::ExeOwner );

    gpgAgent=new QProcess ( this );
    QStringList arguments;
    arguments<<"--pinentry-program"<<"/usr/bin/pinentry-x2go"<<
    "--enable-ssh-support"<<"--daemon"<<"--no-detach";

    connect ( gpgAgent,SIGNAL ( finished ( int,QProcess::ExitStatus ) ),
              this,
              SLOT ( slotGpgAgentFinished ( int,
                                            QProcess::ExitStatus ) ) );

    QStringList env=QProcess::systemEnvironment();
    env<<"GNUPGHOME="+gpgPath<<"CARDAPPID="+appId;
    gpgAgent->setEnvironment ( env );
    gpgAgent->start ( "gpg-agent",arguments );
}

void ONMainWindow::slotGpgAgentFinished ( int , QProcess::ExitStatus )
{
    QString stdOut ( gpgAgent->readAllStandardOutput() );
    stdOut=stdOut.simplified();
    stdOut.replace ( " ","" );
    QStringList envLst=stdOut.split ( ";" );
    QString gpg_agent_info=envLst[0].split ( "=" ) [1];
    QString ssh_auth_sock=envLst[2].split ( "=" ) [1];
    agentPid=envLst[4].split ( "=" ) [1];
    x2goDebug<<gpg_agent_info<<ssh_auth_sock<<agentPid<<endl;
    x2goDebug<<"GPGAGENT out:"<<envLst[0]<<envLst[2]<<envLst[4]<<endl;

    agentCheckTimer->start ( 1000 );
    cardReady=true;

    sshEnv.clear();
    sshEnv<<envLst[0]<<envLst[2]<<envLst[4];
// 	x2goDebug<<"sshenv:"<<sshEnv<<endl;

    if ( !useLdap )
    {
        if ( passForm->isVisible() )
        {
            if ( passForm->isEnabled() )
            {
                if ( login->isEnabled() )
                {
                    login->setText ( cardLogin );
                    slotSessEnter();
                    return;
                }
            }
        }
        QProcess sshadd ( this ); //using it to start scdaemon
        sshadd.setEnvironment ( sshEnv );
        QStringList arguments;
        arguments<<"-l";
        sshadd.start ( "ssh-add",arguments );
        sshadd.waitForFinished ( -1 );
        QString sshout ( sshadd.readAllStandardOutput() );
        sshout=sshout.simplified();
        x2goDebug<<"SSH-ADD out:"<<sshout<<endl;
    }
    else
    {
        if ( selectSessionDlg->isVisible() ||
                sessionStatusDlg->isVisible() )
        {
            QProcess sshadd ( this ); //using it to start scdaemon
            sshadd.setEnvironment ( sshEnv );
            QStringList arguments;
            arguments<<"-l";
            sshadd.start ( "ssh-add",arguments );
            sshadd.waitForFinished ( -1 );
            QString sshout ( sshadd.readAllStandardOutput() );
            sshout=sshout.simplified();
            x2goDebug<<"SSH-ADD out:"<<sshout<<endl;
            return;
        }
        if ( passForm->isVisible() )
            slotClosePass();
        uname->setText ( cardLogin );
        slotUnameEntered();
        slotPassEnter();
    }
}


void ONMainWindow::slotCheckAgentProcess()
{
    if ( checkAgentProcess() )
        return;
    agentCheckTimer->stop();
    cardReady=false;
    if ( cardStarted )
    {
        cardStarted=false;
        if ( nxproxy )
            if ( nxproxy->state() ==QProcess::Running )
                nxproxy->terminate();
    }

    x2goDebug<<"gpg-agent finished\n";
    slotStartPGPAuth();
}

bool ONMainWindow::checkAgentProcess()
{
    QFile file ( "/proc/"+agentPid+"/cmdline" );
    if ( file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        QString line ( file.readLine() );
        file.close();
        if ( line.indexOf ( "gpg-agent" ) !=-1 )
        {
            return true;
        }
    }
    return false;
}

#if defined ( Q_OS_DARWIN )
QString ONMainWindow::getXDisplay()
{
    QTcpSocket tcpSocket ( this );
    uint dispNumber=0;
    QString xname,xdir,xopt;
    dispNumber=0;
    xdir=ConfigDialog::getXDarwinDirectory();
    xname=xdir+"/Contents/MacOS/X11";
    xopt=" -rootless :0";

    //for newer versions of XQuartz start startx instead of X11.app
    xname="/usr/X11/bin/startx";
    xopt="";
    tcpSocket.connectToHost ( "127.0.0.1",6000+dispNumber );

    if ( tcpSocket.waitForConnected ( 3000 ) )
    {
        tcpSocket.close();
        return QString::number ( dispNumber );
    }
    if ( xname==QString::null )
    {
        QMessageBox::critical (
            this,tr ( "Can't connect to X-Server" ),
            tr (
                "Can't connect to X-Server\nPlease check your settings"
            ) );
        slotConfig();
        return QString::null;
    }
    QProcess* pr=new QProcess ( this );
    pr->setWorkingDirectory ( xdir );
    pr->start ( xname+" "+xopt,QIODevice::NotOpen );
    if ( pr->waitForStarted ( 3000 ) )
    {
#ifdef Q_OS_DARWIN
        //FIXME: the call of unistd.h sleep() do not work on all
        // Mac OS X systems
        system ( "sleep 3" );
#endif

        tcpSocket.connectToHost ( "127.0.0.1",6000+dispNumber );
        if ( tcpSocket.waitForConnected ( 1000 ) )
        {
            tcpSocket.close();
            return QString::number ( dispNumber );
        }
        QMessageBox::critical (
            this,tr ( "Can't connect to X-Server" ),
            tr (
                "Can't connect to X-Server\nPlease check your settings"
            ) );
        slotConfig();
        return QString::null;
    }
    QMessageBox::critical (
        this,QString::null,
        tr (
            "Can't start X Server\nPlease check your settings" ) );
    slotConfig();
    return QString::null;
}
#endif

#ifdef Q_OS_WIN
QString ONMainWindow::getXDisplay()
{
    if ( !isServerRunning ( 6000+xDisplay ) )
    {
        QMessageBox::critical (
            this,QString::null,
            tr (
                "Can't start X Server\nPlease check your installation" )
        );
        close();
    }
    return QString::number ( xDisplay );

}

QString ONMainWindow::cygwinPath ( const QString& winPath )
{
    QString cPath="/cygdrive/"+winPath;
    cPath.replace ( "\\","/" );
    cPath.replace ( ":","" );
    return cPath;
}
#endif

bool ONMainWindow::isColorDepthOk ( int disp, int sess )
{
    if ( sess==0 )
        return true;
    if ( disp==sess )
        return true;
    if ( ( disp == 24 || disp == 32 ) && ( sess == 24 || sess == 32 ) )
        return true;
    return false;
}

#ifndef Q_OS_LINUX
void ONMainWindow::setWidgetStyle ( QWidget* widget )
{
    widget->setStyle ( widgetExtraStyle );
}
#else
void ONMainWindow::setWidgetStyle ( QWidget* )
{
}
#endif

QString ONMainWindow::internAppName ( const QString& transAppName, bool* found )
{
    if ( found )
        *found=false;
    int ind=_transApplicationsNames.indexOf ( transAppName );
    if ( ind!=-1 )
    {
        if ( found )
            *found=true;
        return _internApplicationsNames[ind];
    }
    return transAppName;
}


QString ONMainWindow::transAppName ( const QString& internAppName, bool* found )
{
    if ( found )
        *found=false;
    int ind=_internApplicationsNames.indexOf ( internAppName );
    if ( ind!=-1 )
    {
        if ( found )
            *found=true;
        return _transApplicationsNames[ind];
    }
    return internAppName;
}

void ONMainWindow::addToAppNames ( QString intName, QString transName )
{
    _internApplicationsNames.append ( intName );
    _transApplicationsNames.append ( transName );
}


void ONMainWindow::slotExecXmodmap()
{
#ifdef Q_WS_HILDON
    QString passwd=getCurrentPass();
    QString user=getCurrentUname();
    QString host=resumingSession.server;
    QString cmd;

    cmd="(xmodmap -pke ;"
        "echo keycode 73= ;"
// 	    "echo clear shift ;"
// 	    "echo clear lock ;"
// 	    "echo clear control ;"
// 	    "echo clear mod1 ;"
// 	    "echo clear mod2 ;"
// 	    "echo clear mod3 ;"
// 	    "echo clear mod4 ;"
// 	    "echo clear mod5 ;"
//  	    "echo add shift = Shift_L ;"
        "echo add control = Control_R "
//  	    "echo add mod5 = ISO_Level3_Shift"
        ")| DISPLAY=:"
        +resumingSession.display+" xmodmap - ";
    x2goDebug<<"cmd:"<<cmd;
    SshProcess* xmodProc;
    try
    {
        xmodProc=new SshProcess ( this,user,host,sshPort,
                                  cmd,
                                  passwd,currentKey,acceptRsa );
    }
    catch ( QString message )
    {
        return;
    }

    if ( cardReady /*|| useSshAgent*/ )
    {
        QStringList env=xmodProc->environment();
        env+=sshEnv;
        xmodProc->setEnvironment ( env );
    }
    xmodProc->setFwX ( true );
    xmodProc->startNormal();
#endif
}

void ONMainWindow::check_cmd_status()
{
    QString passwd;
    QString user=getCurrentUname();
    QString host=resumingSession.server;
    passwd=getCurrentPass();

    x2goDebug<<"check command message"<<endl;
    SshProcess* proc;
    proc=new SshProcess ( sshConnection, this );
    connect ( proc,SIGNAL ( sshFinished ( bool,QString,SshProcess* ) ),
              this,SLOT ( slotCmdMessage ( bool, QString,SshProcess* ) ) );

    proc->startNormal ( "x2gocmdexitmessage "+
                        resumingSession.sessionId );
}

void ONMainWindow::slotCmdMessage ( bool result,QString output,
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
        pass->setFocus();
        pass->selectAll();
        sshConnection->disconnectSession();
        return;
    }
    if ( output.indexOf ( "X2GORUNCOMMAND ERR NOEXEC:" ) !=-1 )
    {
        QString cmd=output;
        cmd.replace ( "X2GORUNCOMMAND ERR NOEXEC:","" );
        QMessageBox::critical ( 0l,tr ( "Error" ),
                                tr ( "Unable to execute: " ) +
                                cmd,QMessageBox::Ok,
                                QMessageBox::NoButton );
    }
    sshConnection->disconnectSession();
}


int ONMainWindow::startSshFsTunnel()
{
    fsTunReady=false;
    x2goDebug<<"starting fs tunnel for:"<<resumingSession.sessionId<<
    "\nfs port: "<<resumingSession.fsPort;

    if ( resumingSession.fsPort.length() <=0 )
    {
        QString message=tr (
                            "Remote server does not "
                            "support file system export "
                            "through SSH Tunnel\n"
                            "Please update to a newer "
                            "x2goserver package" );
        slotFsTunnelFailed ( false,message,0 );
        return 1;
    }
    QString passwd=getCurrentPass();
    QString uname=getCurrentUname();

    fsTunnel=new SshProcess ( sshConnection, this );

    connect ( fsTunnel,SIGNAL ( sshFinished ( bool,
                                QString,SshProcess* ) ),
              this,SLOT ( slotFsTunnelFailed ( bool,
                                               QString,SshProcess* ) ) );

    connect ( fsTunnel,SIGNAL ( sshTunnelOk() ),
              this,SLOT ( slotFsTunnelOk() ) );

    fsTunnel->startTunnel ( "localhost",resumingSession.fsPort.toUInt(),"127.0.0.1",
                            clientSshPort.toInt(), true );
    return 0;
}

void ONMainWindow::slotFsTunnelFailed ( bool result,  QString output,
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
        if ( fsTunnel )
            delete fsTunnel;
        fsTunnel=0l;
        fsTunReady=false;
    }
}


void ONMainWindow::slotFsTunnelOk()
{
    fsTunReady=true;
    //start reverse mounting if RSA Key and FS tunnel are ready
    //start only once from slotFsTunnelOk() or slotCopyKey().
    if ( fsExportKeyReady )
        startX2goMount();
}

