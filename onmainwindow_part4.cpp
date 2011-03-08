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

void ONMainWindow::startX2goMount()
{
    QFile file ( fsExportKey+".pub" );
    if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        QString message=tr ( "Unable to read :\n" ) +fsExportKey+".pub";
        QMessageBox::critical ( 0l,tr ( "Error" ),message,
                                QMessageBox::Ok,
                                QMessageBox::NoButton );
        QFile::remove
        ( fsExportKey+".pub" );
        return;
    }

    QByteArray line = file.readLine();
    file.close();
    QString authofname=homeDir;
#ifdef Q_OS_WIN
    QDir tdir;
    tdir.mkpath ( authofname+"\\.x2go\\.ssh" );
    x2goDebug<<"Creating "<<authofname+"\\.x2go\\.ssh"<<endl;
    authofname=wapiShortFileName ( authofname ) +"/.x2go";
#endif
    authofname+= "/.ssh/authorized_keys" ;

    QFile file1 ( authofname );

    if ( !file1.open ( QIODevice::WriteOnly | QIODevice::Text |
                       QIODevice::Append ) )
    {
        QString message=tr ( "Unable to write :\n" ) + authofname;
        QMessageBox::critical ( 0l,tr ( "Error" ),message,
                                QMessageBox::Ok,
                                QMessageBox::NoButton );
        QFile::remove
        ( fsExportKey+".pub" );
        return;

    }
    QTextStream out ( &file1 );
    out<<line;
    file1.close();
    directory* dir=getExpDir ( fsExportKey );
    bool rem=dir->isRemovable;
    if ( !dir )
        return;

    QString passwd=getCurrentPass();
    QString user=getCurrentUname();
    QString host=resumingSession.server;
    QString sessionId=resumingSession.sessionId;

    QStringList env=QProcess::systemEnvironment();


    QString cuser;
#ifndef Q_WS_HILDON
    for ( int i=0;i<env.size();++i )
    {
        QStringList ls=env[i].split ( "=" );
        if ( ls[0]=="USER" )

        {
            cuser=ls[1];
            break;
        }
    }
#else
    cuser="user";
#endif
#ifdef Q_OS_WIN
    cuser="sshuser";
#endif
    SshProcess* proc=0l;
    QString cmd;
    QString dirs=dir->dirList;

    if ( !fsInTun && clientSshPort!="22" )
    {
        dirs=dirs+"__SSH_PORT__"+clientSshPort;
    }
    if ( fsInTun )
    {
        dirs=dirs+"__REVERSESSH_PORT__"+resumingSession.fsPort;
    }
    if ( !rem )
        cmd="export HOSTNAME && x2gomountdirs dir "+sessionId+" "+cuser+
            " "+dir->dstKey+" "+dirs;
    else
        cmd="export HOSTNAME && x2gomountdirs rem "+sessionId+" "+cuser+
            " "+dir->dstKey+" "+dirs;

#ifdef Q_OS_WIN

    cmd="chmod 600 "+dir->dstKey+"&&"+cmd;
#endif



    X2goSettings st ( "sessions" );

    if ( !useLdap )
    {
        QString sid;
        if ( !embedMode )
            sid=lastSession->id();
        else
            sid="embedded";
        if ( st.setting()->value (
                    sid+"/useiconv", ( QVariant ) false ).toBool() )
        {
            QString toCode=st.setting()->value (
                               sid+"/iconvto",
                               ( QVariant ) "UTF-8" ).toString();

#ifdef Q_OS_WIN
            QString fromCode=st.setting()->value (
                                 sid+"/iconvfrom",
                                 ( QVariant ) tr (
                                     "WINDOWS-1252" ) ).toString();
#endif
#ifdef Q_OS_DARWIN
            QString fromCode=st.setting()->value (
                                 sid+"/iconvfrom",
                                 ( QVariant )
                                 "UTF-8" ).toString();
#endif
#ifdef Q_OS_LINUX
            QString fromCode=st.setting()->value (
                                 sid+"/iconvfrom",
                                 ( QVariant ) tr (
                                     "ISO8859-1" ) ).toString();
#endif
            cmd="export X2GO_ICONV=modules=iconv,from_code="+
                fromCode+
                ",to_code="+toCode+"&&"+cmd;
        }
    }

    proc=new SshProcess ( sshConnection, this );
    dir->proc=proc;

    connect ( proc,SIGNAL ( sshFinished ( bool, QString,SshProcess* ) ),
              this,SLOT ( slotRetExportDir ( bool,
                                             QString,SshProcess* ) ) );

    proc->startNormal ( cmd );
}

void ONMainWindow::slotCheckPrintSpool()
{
    QDir dir ( spoolDir );
    QStringList list = dir.entryList ( QDir::Files );
    for ( int i=0;i<list.size();++i )
    {
        if ( !list[i].endsWith ( ".ready" ) )
            continue;
        QFile file ( spoolDir+"/"+list[i] );
        if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
            continue;
        bool startProc=false;
        QString fname,title;
        if ( !file.atEnd() )
        {
            QByteArray line = file.readLine();
            QString fn ( line );
            fn.replace ( "\n","" );
            fname=fn;
            if ( !file.atEnd() )
            {
                line = file.readLine();
                title=line;
                title.replace ( "\n","" );
            }
            startProc=true;
        }
        file.close();
        file.remove();
        if ( startProc )
            new PrintProcess ( spoolDir+"/"+fname,title ,this );

    }
}


void ONMainWindow::cleanPrintSpool()
{
    QDir dir ( spoolDir );
    QStringList list = dir.entryList ( QDir::Files );
    for ( int i=0;i<list.size();++i )
    {
        QFile::remove ( spoolDir+"/"+list[i] );
    }
}


void ONMainWindow::cleanAskPass()
{
    QString path=homeDir +"/.x2go/ssh/";
    QDir dir ( path );
    QStringList list = dir.entryList ( QDir::Files );
    for ( int i=0;i<list.size();++i )
    {
        if ( list[i].startsWith ( "askpass" ) )
            QFile::remove ( path+list[i] );
    }

}



bool ONMainWindow::isServerRunning ( int port )
{
    QTcpSocket tcpSocket ( 0 );
    tcpSocket.connectToHost ( "127.0.0.1",port );

    if ( tcpSocket.waitForConnected ( 1000 ) )
    {
        tcpSocket.close();
        return true;
    }
    return false;
}
#ifdef Q_OS_WIN
void ONMainWindow::slotCheckXOrgLog()
{
    xorgLogMutex.lock();
    if ( xorgLogFile.length() <=0 )
    {
        xorgLogMutex.unlock();
        return;
    }
    QFile file ( xorgLogFile );
    if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        xorgLogMutex.unlock();
        return;
    }

    QTextStream in ( &file );
    while ( !in.atEnd() )
    {
        QString line = in.readLine();
        if ( line.indexOf ( "successfully opened the display" ) !=-1 )
        {
            xorgLogTimer->stop();
            slotSetWinServersReady();
            xorgLogMutex.unlock();
            return;
        }
    }
    xorgLogMutex.unlock();
}

void ONMainWindow::startXOrg ()
{
    while ( isServerRunning ( 6000+xDisplay ) )
        ++xDisplay;
    QString dispString;
    QTextStream ( &dispString ) <<":"<<xDisplay;

    QStringList args;
    QString exec=appDir+"\\xming\\Xming.exe";
    bool xming=true;
    winServersReady=false;
#ifdef CFGCLIENT
    xming=useXming;
    if (!xming)
    {
        exec=xorgExe;
        QString cmdLine;
        if (startXorgOnStart)
            cmdLine=xorgOptions;
        else
        {
            switch (xorgMode)
            {
            case FS:
                cmdLine=xorgFSOptions;
                break;
            case SAPP:
                cmdLine=xorgSAppOptions;
                break;
	    case WIN:
                cmdLine=xorgWinOptions;
		x2goDebug<<"WxH:"<<xorgWidth<<"x"<<xorgHeight<<endl;
                cmdLine.replace("%w",xorgWidth);
                cmdLine.replace("%h",xorgHeight);
		x2goDebug<<cmdLine<<endl;
                break;
            }
        }
        QStringList options=cmdLine.split(" ",QString::SkipEmptyParts);
        QString option;
        foreach(option,options)
        {
            args<< option;
        }
        args<<dispString;
    }
#endif
    xorg=new QProcess ( 0 );
    if (xming)
    {
      
        QString workingDir=appDir+"\\xming";
        QStringList env=QProcess::systemEnvironment();
        env<<"GLWIN_ENABLE_DEBUG=0";
        xorgLogMutex.lock();
        xorgLogFile=homeDir+"/.x2go/xorg";
        QDir dr ( homeDir );
        dr.mkpath ( xorgLogFile );
        xorgLogFile=wapiShortFileName ( xorgLogFile ) +"\\xorg.log."+
                    QString::number ( xDisplay );
        if ( QFile::exists ( xorgLogFile ) )
            QFile::remove ( xorgLogFile );
        xorgLogMutex.unlock();
//run xming with clipboard support
        args<<dispString<<"-multiwindow"<<"-notrayicon"<<"-clipboard"<<
        "-logfile"<<xorgLogFile;
        xorg->setEnvironment ( env );
        xorg-> setWorkingDirectory ( workingDir);
    }
    
    x2goDebug<<"running"<<exec;
    xorg->start ( exec, args );


    if ( !xorg->waitForStarted ( 3000 ) )
    {
        QMessageBox::critical (
            0,QString::null,
            tr ( "Can't start X Server\n"
                 "Please check your installation" ) );
        close();
    }
#ifdef CFGCLIENT
    if (!xming)
    {
        x2goDebug<<"servers ready after"<<xorgDelay<<endl;
        QTimer::singleShot(xorgDelay*1000, this, SLOT(slotSetWinServersReady()));
    }
#endif
}


WinServerStarter::WinServerStarter ( daemon server, ONMainWindow * par ) :
QThread ( 0 )
{
    mode=server;
    parent=par;
}

void WinServerStarter::run()
{
    switch ( mode )
    {
    case SSH:
        parent->startSshd();
        break;
    case X:
        parent->startXOrg();
        break;
    case PULSE:
        parent->startPulsed();
        break;
    }
}



void ONMainWindow::startWinServers()
{
    x2goDebug<<"Starting win servers"<<endl;

    QString etcDir=homeDir+"/.x2go/etc";
    QDir dr ( homeDir );

    pulseServer=0l;

    WinServerStarter* xStarter = new WinServerStarter ( WinServerStarter::X,
            this );
    WinServerStarter* sshStarter = new WinServerStarter (
        WinServerStarter::SSH, this );

    WinServerStarter* pulseStarter = new WinServerStarter (
        WinServerStarter::PULSE, this );

    if ( !embedMode || !config.confFS || ( config.confFS && config.useFs ) )
    {

        dr.mkpath ( etcDir );
        generateHostDsaKey();
        generateEtcFiles();
        sshStarter->start();
    }
    if ( !embedMode || !config.confSnd ||
            ( config.confSnd && config.useSnd ) )
    {
        pulseStarter->start();
    }
#ifdef CFGCLIENT
        x2goDebug<<"xorg settings: "<<startXorgOnStart <<" "<< useXming<<endl;
    if (useXming)
    {
#endif
        xStarter->start();
        xorgLogTimer=new QTimer ( this );
        connect ( xorgLogTimer,SIGNAL ( timeout() ),this,
                  SLOT ( slotCheckXOrgLog() ) );
        xorgLogTimer->start ( 500 );
#ifdef CFGCLIENT
    }
    else
    {
        if (startXorgOnStart)
        {
            startXOrg();
        }
    }
#endif
}


bool ONMainWindow::haveCySolEntry()
{
    QSettings CySolSt ( "HKEY_CURRENT_USER\\Software"
                        "\\Cygnus Solutions",
                        QSettings::NativeFormat );
    return ( CySolSt.childGroups().count() > 0 ||
             CySolSt.childKeys().count() > 0 );
}

bool ONMainWindow::haveCygwinEntry()
{
    QSettings CygwSt ( "HKEY_CURRENT_USER\\Software"
                       "\\Cygnus Solutions\\Cygwin",
                       QSettings::NativeFormat );
    return ( CygwSt.childGroups().count() >0||CygwSt.childKeys().count() );
}



void ONMainWindow::saveCygnusSettings()
{
    if ( ONMainWindow::portable )
    {
        if ( haveCySolEntry() )
        {
            x2goDebug<<"Cygnus Solutions entry exist";
            cySolEntry=true;
        }
        else
        {
            cySolEntry=false;
            x2goDebug<<"Cygnus Solutions entry not exist";
        }

        if ( haveCygwinEntry() )
        {
            x2goDebug<<"Cygwin entry exist";
            cyEntry=true;
        }
        else
        {
            x2goDebug<<"Cygwin entry not exist";
            cyEntry=false;
        }
    }

    QSettings etcst ( "HKEY_CURRENT_USER\\Software"
                      "\\Cygnus Solutions\\Cygwin\\mounts v2\\/etc",
                      QSettings::NativeFormat );
    oldEtcDir=QString::null;
    oldEtcDir=etcst.value ( "native",oldEtcDir ).toString();
    x2goDebug<<"old etc:"<<oldEtcDir<<endl;
    QString newEtc=homeDir+"/.x2go/etc";
    QDir d ( newEtc );
    if ( !d.exists() )
        d.mkpath ( newEtc );
    newEtc.replace ( "/","\\" );

    etcst.setValue ( "native",wapiShortFileName ( newEtc ) );
    etcst.sync();
    x2goDebug<<"new etc:"<<wapiShortFileName ( newEtc ) <<endl;

    QSettings binst ( "HKEY_CURRENT_USER\\Software"
                      "\\Cygnus Solutions\\Cygwin\\mounts v2\\/bin",
                      QSettings::NativeFormat );
    oldBinDir=QString::null;
    oldBinDir=binst.value ( "native",oldBinDir ).toString();
    x2goDebug<<"old bin:"<<oldBinDir<<endl;
    QString newBin=appDir;
    newBin.replace ( "/","\\" );
    binst.setValue ( "native",wapiShortFileName ( newBin ) );
    binst.sync();
    x2goDebug<<"new bin:"<<newBin<<endl;

    QSettings tmpst ( "HKEY_CURRENT_USER\\Software"
                      "\\Cygnus Solutions\\Cygwin\\mounts v2\\/tmp",
                      QSettings::NativeFormat );
    oldTmpDir=QString::null;
    oldTmpDir=tmpst.value ( "native",oldTmpDir ).toString();
    x2goDebug<<"old tmp:"<<oldTmpDir<<endl;
    QString newTmp=QDir::tempPath();
    newTmp.replace ( "/","\\" );
    tmpst.setValue ( "native",wapiShortFileName ( newTmp ) );
    tmpst.sync();
    x2goDebug<<"new tmp:"<<newTmp<<endl;
}

void ONMainWindow::restoreCygnusSettings()
{

    if ( oldEtcDir==QString::null )
    {
        QSettings st ( "HKEY_CURRENT_USER\\Software"
                       "\\Cygnus Solutions\\Cygwin\\mounts v2\\/etc",
                       QSettings::NativeFormat );
        x2goDebug<<"Removing /etc from cygwin mounts\n";
        st.remove ( "" );
        st.sync();
    }
    else
    {
        QSettings st ( "HKEY_CURRENT_USER\\Software"
                       "\\Cygnus Solutions\\Cygwin\\mounts v2\\/etc",
                       QSettings::NativeFormat );
        st.setValue ( "native",oldEtcDir );
        st.sync();
        x2goDebug<<"Restoring /etc in cygwin mounts\n";
    }
    if ( oldBinDir==QString::null )
    {
        QSettings st ( "HKEY_CURRENT_USER\\Software"
                       "\\Cygnus Solutions\\Cygwin\\mounts v2\\/bin",
                       QSettings::NativeFormat );
        x2goDebug<<"Removing /bin from cygwin mounts\n";
        st.remove ( "" );
        st.sync();
    }
    else
    {
        QSettings st ( "HKEY_CURRENT_USER\\Software"
                       "\\Cygnus Solutions\\Cygwin\\mounts v2\\/bin",
                       QSettings::NativeFormat );
        st.setValue ( "native",oldBinDir );
        st.sync();
        x2goDebug<<"Restoring /bin in cygwin mounts\n";
    }
    if ( oldTmpDir==QString::null )
    {
        QSettings st ( "HKEY_CURRENT_USER\\Software"
                       "\\Cygnus Solutions\\Cygwin\\mounts v2\\/tmp",
                       QSettings::NativeFormat );
        x2goDebug<<"Removing /tmp from cygwin mounts\n";
        st.remove ( "" );
        st.sync();
    }
    else
    {
        QSettings st ( "HKEY_CURRENT_USER\\Software"
                       "\\Cygnus Solutions\\Cygwin\\mounts v2\\/tmp",
                       QSettings::NativeFormat );
        st.setValue ( "native",oldTmpDir );
        st.sync();
        x2goDebug<<"Restoring /tmp in cygwin mounts\n";
    }
    if ( ONMainWindow::portable )
    {
        if ( !cyEntry )
        {
            removeCygwinEntry();
        }
        if ( !cySolEntry )
        {
            removeCySolEntry();
        }
    }
}

void ONMainWindow::removeCygwinEntry()
{
    QSettings st ( "HKEY_CURRENT_USER\\Software"
                   "\\Cygnus Solutions\\Cygwin",
                   QSettings::NativeFormat );
    x2goDebug<<"Removing cygwin\n";
    st.remove ( "" );
    st.sync();

}

void ONMainWindow::removeCySolEntry()
{
    QSettings st ( "HKEY_CURRENT_USER\\Software"
                   "\\Cygnus Solutions",
                   QSettings::NativeFormat );
    x2goDebug<<"Removing cygnus solutions\n";
    st.remove ( "" );
    st.sync();
}

void ONMainWindow::startPulsed()
{
    while ( isServerRunning ( pulsePort ) )
        ++pulsePort;
    esdPort=pulsePort+1;
    while ( isServerRunning ( esdPort ) )
        ++esdPort;

    pulseDir=homeDir+"/.x2go/pulse";
    QDir dr ( homeDir );
    dr.mkpath ( pulseDir );
    pulseDir=wapiShortFileName ( pulseDir );
    x2goDebug<<"template: "<<pulseDir+"/tmp"<<endl;
    QTemporaryFile* fl=new QTemporaryFile ( pulseDir+"/tmp" );
    fl->open();
    pulseDir=fl->fileName();
    fl->close();
    delete fl;
    QFile::remove ( pulseDir );
    dr.mkpath ( pulseDir );
    x2goDebug<<"pulse tmp file: "<<pulseDir<<endl;
    QStringList pEnv=QProcess::systemEnvironment();
    for ( int i=0; i<pEnv.size();++i )
    {
        if ( pEnv[i].indexOf ( "USERPROFILE=" ) !=-1 )
            pEnv[i]="USERPROFILE="+
                    wapiShortFileName ( homeDir+"/.x2go/pulse" );
        if ( pEnv[i].indexOf ( "TEMP=" ) !=-1 )
            pEnv[i]="TEMP="+pulseDir;
        if ( pEnv[i].indexOf ( "USERNAME=" ) !=-1 )
            pEnv[i]="USERNAME=pulseuser";
    }

    QFile file ( pulseDir+"/config.pa" );
    if ( !file.open ( QIODevice::WriteOnly | QIODevice::Text ) )
        return;
    QTextStream out ( &file );
    out << "load-module module-native-protocol-tcp port="+
    QString::number ( pulsePort ) <<endl;
    out << "load-module module-esound-protocol-tcp port="+
    QString::number ( esdPort ) <<endl;
    out << "load-module module-waveout"<<endl;
    file.close();
    pulseServer=new QProcess ( 0 );
    pulseServer->setEnvironment ( pEnv );
    QStringList args;
    args<<"-n"<<"-F"<<pulseDir+"/config.pa";
    pulseServer->setWorkingDirectory ( wapiShortFileName (
                                           appDir+"\\pulse" ) );
    pulseServer->start ( "pulse\\pulseaudio.exe",args );
    x2goDebug<<"starting pulse\\pulseaudio.exe "<<args.join ( " " ) <<
    " working dir: "<<
    wapiShortFileName ( appDir+"\\pulse" ) <<endl;
}


#ifdef CFGCLIENT
void ONMainWindow::xorgSettings()
{
    x2goDebug<<"getting xorg settings"<<endl;

    X2goSettings st ( "settings" );
    useXming=(st.setting()->value("usexming",true).toBool());
    xorgExe=(st.setting()->value("xexec","C:\\program files\\vcxsrv\\vcxsrv.exe").toString());
    xorgOptions=(st.setting()->value("options","-multiwindow -notrayicon -clipboard").toString());
    startXorgOnStart=(st.setting()->value("onstart",true).toBool());
    xorgWinOptions=(st.setting()->value("optionswin","-screen 0 %wx%h -notrayicon -clipboard").toString());
    xorgFSOptions=(st.setting()->value("optionsfs","-fullscreen -notrayicon -clipboard").toString());
    xorgSAppOptions=(st.setting()->value("optionssingle","-multiwindow -notrayicon -clipboard").toString());
    xorgDelay=(st.setting()->value("delay",3).toInt());

}
#endif

void ONMainWindow::slotSetWinServersReady()
{
    winServersReady=true;
    restoreCygnusSettings();
}

#include <windows.h>
#include<sstream>
#endif

void ONMainWindow::generateEtcFiles()
{
    QString etcDir=homeDir+"/.x2go/etc";
    QDir dr ( homeDir );
    dr.mkpath ( etcDir );
#ifdef Q_OS_WIN
    if ( !QFile::exists ( etcDir+"/passwd" ) )
    {
        QString sid, sys, user, grsid, grname;
        if ( !wapiAccountInfo ( &sid,&user,&grsid, &grname, &sys ) )
        {
// 			x2goDebug<<"Get account info failed\n";
            close();
        }

// 		x2goDebug<<"sid: "<<sid <<" system:"<<
// 		sys<< " user: "<<user<<" group sid:"<<grsid<<
// 		"group name: "<<grname<<endl;

        QStringList sidList=sid.split ( '-' );
        QString rid=sidList[sidList.count()-1];
        QStringList grsidList=grsid.split ( '-' );
        QString grid=grsidList[grsidList.count()-1];
        QFile file ( etcDir +"/passwd" );
        if ( !file.open ( QIODevice::WriteOnly | QIODevice::Text ) )
            return;
        QTextStream out ( &file );
        out <<"sshuser::"<<rid<<":"<<grid<<":"<<sys<<"\\sshuser,"
        <<sid<<":"<<cygwinPath ( wapiShortFileName ( homeDir ) ) <<
        "/.x2go"<<":/bin/bash\n";
        file.close();
    }

    if ( !QFile::exists ( etcDir+"/sshd_config" ) )
    {
#endif
        QFile file ( etcDir +"/sshd_config" );
        if ( !file.open ( QIODevice::WriteOnly | QIODevice::Text ) )
            return;
        QTextStream out ( &file );
        out<<"StrictModes no\n"<<
        "UsePrivilegeSeparation no\n"<<
#ifdef Q_OS_WIN
        "Subsystem sftp /bin/sftp-server\n";
#else
        "Subsystem sftp "
        <<appDir<<"/sftp-server\n";
#endif
        file.close();
        x2goDebug<<etcDir +"/sshd_config created";
#ifdef Q_OS_WIN
    }
#endif
}

void ONMainWindow::generateHostDsaKey()
{
    QString etcDir=homeDir+"/.x2go/etc";
    QDir dr ( homeDir );
    dr.mkpath ( etcDir );
    if ( !QFile::exists ( etcDir+"/ssh_host_dsa_key" ) ||
            !QFile::exists ( etcDir+"/ssh_host_dsa_key.pub" ) )
    {
        /*		x2goDebug<<"Generating host DSA key\n";*/
#ifdef Q_OS_WIN
        QString fname=cygwinPath ( wapiShortFileName ( etcDir ) ) +
                      "/ssh_host_dsa_key";
#else
        QString fname=etcDir+"/ssh_host_dsa_key";
#endif
        QStringList args;
        args<<"-t"<<"dsa"<<"-N"<<""<<"-C"<<
        "x2goclient DSA host key"<<"-f"<<fname;
        QProcess::execute ( "ssh-keygen",args );
    }
}

void ONMainWindow::startSshd()
{
    if ( embedMode && config.confFS && !config.useFs )
    {
        return;
    }
#ifdef Q_OS_LINUX
    clientSshPort="7022";
#endif
    QString etcDir=homeDir+"/.x2go/etc";
    int port=clientSshPort.toInt();
    //clientSshPort have initvalue
    while ( isServerRunning ( port ) )
        ++port;
    clientSshPort=QString::number ( port );
#ifdef Q_OS_WIN
    std::string clientdir=wapiShortFileName ( appDir ).toStdString();
    std::stringstream strm;
    strm<<clientdir<<"\\sshd.exe -D -p"<<clientSshPort.toInt();

    STARTUPINFOA si;
    std::string desktopName="x2go_";
    desktopName+=getenv ( "USERNAME" );
    char* desktop=new char[desktopName.size() +1];
    strcpy ( desktop,desktopName.c_str() );
    x2goDebug<<"Creating desktop: "<<desktop<<endl;
    if ( !CreateDesktopA (
                desktop,
                0,
                0,
                0,
                GENERIC_ALL,
                0
            ) )
    {
        strcpy ( desktop,"" );
        x2goDebug<<"Desktop creation failed, using default\n";
    }
    ZeroMemory ( &si, sizeof ( si ) );
    ZeroMemory ( &sshd, sizeof ( sshd ) );
    si.lpDesktop=desktop;
    si.cb = sizeof ( si );
    CreateProcessA ( NULL,  // No module name (use command line)
                     ( LPSTR ) strm.str().c_str(),  // Command line
                     NULL,           // Process handle not inheritable
                     NULL,           // Thread handle not inheritable
                     TRUE,          // Set handle inheritance to FALSE
                     0/*CREATE_NO_WINDOW|CREATE_NEW_PROCESS_GROUP*/,
                     //creation flags
                     NULL,           // Use parent's environment block
                     clientdir.c_str(), // Starting directory
                     &si,            // Pointer to STARTUPINFO structure
                     &sshd );// Pointer to PROCESS_INFORMATION structure
    delete []desktop;
    winSshdStarted=true;
#else
    userSshd=true;
    sshd=new QProcess ( this );
    QStringList arguments;
    arguments<<"-f"<<etcDir +"/sshd_config"<< "-h" <<
    etcDir+"/ssh_host_dsa_key"<<"-D"<<"-p"<<clientSshPort;
    sshd->start ( appDir+"/sshd",arguments );
    x2goDebug<<"Usermode sshd started";
#endif
}


void ONMainWindow::slotFindProxyWin()
{
    x2goDebug<<"search proxy win: "<<"X2GO-"+resumingSession.sessionId;
#ifndef Q_OS_DARWIN

//#ifdef CFGPLUGIN
    proxyWinId=findWindow ( "X2GO-"+resumingSession.sessionId );
//#endif

    if ( proxyWinId )
    {
        x2goDebug<<"proxy win found:"<<proxyWinId;
        proxyWinTimer->stop();
        if ( embedMode )
        {
            if ( config.rootless )
            {
                x2goDebug<<"win is rootless";
                act_embedContol->setEnabled ( false );
            }
            else
                slotAttachProxyWindow();
        }
#ifdef Q_OS_WIN
        x2goDebug<<"maximizeProxyWin: "<<maximizeProxyWin;
        if ( !startEmbedded )
        {
            if ( maximizeProxyWin )
            {
                QDesktopWidget dw;
                x2goDebug<<"making proxy win full screen";
                wapiSetFSWindow ( ( HWND ) proxyWinId,
                                  dw.screenGeometry ( this ) );
                /*wapiShowWindow ( ( HWND ) proxyWinId,
                   WAPI_SHOWMAXIMIZED );*/
            }
            else
            {
                wapiMoveWindow ( ( HWND ) proxyWinId,0,0,
                                 proxyWinWidth,
                                 proxyWinHeight,true );
            }
        }
#endif
    }
#endif
}


QString ONMainWindow::getCurrentUname()
{
    return login->text();
}

QString ONMainWindow::getCurrentPass()
{
    return pass->text();
}

void ONMainWindow::slotDetachProxyWindow()
{
    proxyWinEmbedded=false;
    bgFrame->show();
    setStatStatus();
    act_embedContol->setText ( tr ( "Attach X2Go window" ) );
    act_embedContol->setIcon ( QIcon ( ":icons/32x32/attach.png" ) );
#ifdef Q_OS_LINUX
    //if QX11EmbedContainer cannot embed window, check if window exists
    //and reconnect
    if ( !embedControlChanged )
    {
        x2goDebug<<"\n";
        slotFindProxyWin();
        x2goDebug<<"proxy win detached, proxywin is:"<<proxyWinId<<endl;
    }
#endif
    embedControlChanged=false;
}


void ONMainWindow::slotAttachProxyWindow()
{
    x2goDebug<<"slotAttachProxy";
    if ( startEmbedded )
    {
        embedControlChanged=false;
        bgFrame->hide();
        proxyWinEmbedded=true;
        setStatStatus();
        act_embedContol->setText ( tr ( "Detach X2Go window" ) );
        act_embedContol->setIcon (
            QIcon ( ":icons/32x32/detach.png" ) );
        QTimer::singleShot ( 100, this, SLOT ( slotEmbedWindow() ) );
    }
    else
    {
        x2goDebug<<"start embedded was false";
        startEmbedded=true;
    }

}

void ONMainWindow::slotEmbedWindow()
{
#ifndef Q_OS_DARWIN
#ifdef CFGPLUGIN
    embedWindow ( proxyWinId );
#endif
    QTimer::singleShot ( 1000, this,
                         SLOT ( slotActivateWindow() ) );

#endif
}

void ONMainWindow::setEmbedSessionActionsEnabled ( bool enable )
{
    act_shareFolder->setEnabled ( enable );
    act_suspend->setEnabled ( enable );
    act_terminate->setEnabled ( enable );
    act_embedContol->setEnabled ( enable );
    act_reconnect->setEnabled ( !enable );
}

void ONMainWindow::slotEmbedControlAction()
{
#ifndef Q_OS_DARWIN
    embedControlChanged=true;
    if ( proxyWinEmbedded )
    {
#ifdef CFGPLUGIN
        detachClient();
#endif
    }
    else
        slotAttachProxyWindow();
#endif
}

void ONMainWindow::slotEmbedIntoParentWindow()
{
#ifndef Q_OS_DARWIN
// 	embedInto ( embedParent );
#endif
}


void ONMainWindow::processSessionConfig()
{
    sshProxy.use=false;
    bool haveKey=false;

    config.command="KDE";
    config.sshport="22";
    config.session=tr ( "X2Go Session" );
    config.checkexitstatus=true;
    config.showtermbutton=true;
    config.showexpbutton=true;
    config.showconfig=true;
    config.showextconfig=true;
    config.showtoolbar=true;
    config.showstatusbar=true;
    config.kbdType=getDefaultKbdType();
    config.kbdLay=getDefaultLayout()[0];


    config.confSnd=false;
    config.confFS=false;
    config.confConSpd=false;
    config.confCompMet=false;
    config.confImageQ=false;
    config.confDPI=false;
    config.confKbd=false;

    QStringList lines=m_x2goconfig.split ( "\n" );


    for ( int i=0;i<lines.count();++i )
    {
        QString line = lines[i];
        if ( ( line=="-----BEGIN DSA PRIVATE KEY-----" ) ||
                ( line=="-----BEGIN RSA PRIVATE KEY-----" ) )
        {
            while ( i<lines.count() )
                config.key+=lines[i++] +"\n";
            haveKey=true;
        }
        else
            processCfgLine ( line );
    }
    act_terminate->setVisible ( config.showtermbutton );
    act_shareFolder->setVisible ( config.showexpbutton );
    act_set->setVisible ( config.showconfig );
    if (!config.showstatusbar)
    {
        statusBar()->hide();
    }
    if ( managedMode )
    {
        QTimer::singleShot ( 500, this, SLOT ( slotStartBroker() ) );
        return;
    }
    slotSelectedFromList ( ( SessionButton* ) 0 );
}


void ONMainWindow::processCfgLine ( QString line )
{
    QStringList lst=line.split ( "=" );
    if ( lst[0]=="command" )
    {
        config.command=lst[1];
        return;
    }
    if ( lst[0]=="server" )
    {
        config.server=lst[1];
        return;
    }
    if ( lst[0]=="session" )
    {
        config.session=lst[1];
        return;
    }
    if ( lst[0]=="sshport" )
    {
        config.sshport=lst[1];
        return;
    }
    if ( lst[0]=="user" )
    {
        config.user=lst[1];
        return;
    }
    if ( lst[0]=="rootless" )
    {
        if ( lst[1]=="true" )
            config.rootless=true;
        else
            config.rootless=false;
        return;
    }
    if ( lst[0]=="checkexitstatus" )
    {
        if ( lst[1]=="true" )
            config.checkexitstatus=true;
        else
            config.checkexitstatus=false;
        return;
    }
    if ( lst[0]=="showtermbutton" )
    {
        if ( lst[1]=="true" )
            config.showtermbutton=true;
        else
            config.showtermbutton=false;
        return;
    }
    if ( lst[0]=="showexpbutton" )
    {
        if ( lst[1]=="true" )
            config.showexpbutton=true;
        else
            config.showexpbutton=false;
        return;
    }
    if ( lst[0]=="showconfig" )
    {
        if ( lst[1]=="true" )
            config.showconfig=true;
        else
            config.showconfig=false;
        return;
    }
    if ( lst[0]=="showextconfig" )
    {
        if ( lst[1]=="true" )
            config.showextconfig=true;
        else
            config.showextconfig=false;
        return;
    }
    if ( lst[0]=="showstatusbar" )
    {
        if ( lst[1]=="true" )
            config.showstatusbar=true;
        else
            config.showstatusbar=false;
        return;
    }
    if ( lst[0]=="showtoolbar" )
    {
        if ( lst[1]=="true" )
            config.showtoolbar=true;
        else
            config.showtoolbar=false;
        return;
    }

    if ( lst[0]=="sound" )
    {
        config.confSnd=true;
        if ( lst[1]=="true" )
            config.useSnd=true;
        else
            config.useSnd=false;
        return;
    }
    if ( lst[0]=="exportfs" )
    {
        config.confFS=true;
        if ( lst[1]=="true" )
            config.useFs=true;
        else
            config.useFs=false;
        return;
    }

    if ( lst[0]=="speed" )
    {
        config.confConSpd=true;
        config.conSpeed=ADSL;

        if ( lst[1]=="modem" )
            config.conSpeed=MODEM;
        else if ( lst[1]=="isdn" )
            config.conSpeed=ISDN;
        else if ( lst[1]=="adsl" )
            config.conSpeed=ADSL;
        else if ( lst[1]=="wan" )
            config.conSpeed=WAN;
        else if ( lst[1]=="lan" )
            config.conSpeed=LAN;
        else
        {
            qCritical (
                "%s",tr (
                    "wrong value for argument\"speed\""
                ).toLocal8Bit().data() );
        }
        return;
    }
    if ( lst[0]=="compression" )
    {
        config.confCompMet=true;
        config.compMet=lst[1];
        return;
    }
    if ( lst[0]=="quality" )
    {
        config.confImageQ=true;
        config.imageQ=lst[1].toInt();
        return;
    }
    if ( lst[0]=="dpi" )
    {
        config.confDPI=true;
        config.dpi=lst[1].toInt();
        return;
    }
    if ( lst[0]=="kbdlayout" )
    {
        config.confKbd=true;
        config.kbdLay=lst[1];
        return;
    }
    if ( lst[0]=="kbdtype" )
    {
        config.confKbd=true;
        config.kbdType=lst[1];
        return;
    }
    if ( lst[0]=="brokerurl" )
    {
        config.brokerurl=lst[1];
        managedMode=true;
        acceptRsa=true;
    }
    if ( lst[0]=="proxy" )
    {
        config.proxy=sshProxy.host=lst[1];
        sshProxy.use=true;
#ifdef Q_OS_WIN
        sshProxy.bin=cygwinPath ( wapiShortFileName ( appDir ) ) +"/ssh";
#else
        sshProxy.bin="ssh";
#endif
        return;
    }
    if ( lst[0]=="proxysshport" )
    {
        config.proxyport=sshProxy.port=lst[1];
        return;
    }
    if ( lst[0]=="cookie" )
    {
        config.cookie=lst[1];
        return;
    }
    if ( lst[0]=="x2gosession" )
    {
        config.sessiondata=lst[1];
        return;
    }
    if ( lst[0]=="connectionts" )
    {
        config.connectionts=lst[1];
        return;
    }
}

void ONMainWindow::slotChangeKbdLayout(const QString& layout)
{
#ifdef Q_OS_LINUX
    QStringList args;
    args<<"-layout"<<layout;
    x2goDebug<<"running setxkbmap with params: "<<args.join(" ");
    QProcess::startDetached("setxkbmap",args);
#endif
}

void ONMainWindow::initPassDlg()
{
    passForm = new SVGFrame ( ":/svg/passform.svg",
                              false,bgFrame );
#ifdef Q_OS_WIN
    passForm->setMainWidget ( ( QWidget* ) this );
#endif
    username->addWidget ( passForm );
    passForm->hide();
    setWidgetStyle ( passForm );
    if ( !miniMode )
        passForm->setFixedSize ( passForm->sizeHint() );
    else
        passForm->setFixedSize ( 310,180 );
    QPalette pal=passForm->palette();
    pal.setBrush ( QPalette::Window, QColor ( 255,255,255,0 ) );
    passForm->setPalette ( pal );

    pal.setColor ( QPalette::Button, QColor ( 255,255,255,0 ) );
    pal.setColor ( QPalette::Window, QColor ( 255,255,255,255 ) );
    pal.setColor ( QPalette::Base, QColor ( 255,255,255,255 ) );

    QFont fnt=passForm->font();
    if ( miniMode )
#ifdef Q_WS_HILDON
        fnt.setPointSize ( 10 );
#else
        fnt.setPointSize ( 9 );
#endif
    passForm->setFont ( fnt );

    fotoLabel=new QLabel ( passForm );
    fotoLabel->hide();

    nameLabel=new QLabel ( "",passForm );
    nameLabel->hide();

    loginPrompt=new QLabel ( tr ( "Login:" ),passForm );
    passPrompt=new QLabel ( tr ( "Password:" ),passForm );
    layoutPrompt=new QLabel ( tr ( "Keyboard layout:" ),passForm );

    login=new ClickLineEdit ( passForm );
    setWidgetStyle ( login );
    login->setFrame ( false );
    login->setEnabled ( false );

    login->hide();
    loginPrompt->hide();

    pass=new ClickLineEdit ( passForm );
    setWidgetStyle ( pass );
    pass->setFrame ( false );
    fnt.setBold ( true );
    pass->setFont ( fnt );
    pass->setEchoMode ( QLineEdit::Password );
    pass->setFocus();

#ifdef Q_OS_LINUX
    connect ( login,SIGNAL ( clicked() ),this,
              SLOT ( slotActivateWindow() ) );
    connect ( pass,SIGNAL ( clicked() ),this,
              SLOT ( slotActivateWindow() ) );
#endif

    pass->hide();
    passPrompt->hide();
    
    
    cbLayout=new QComboBox(passForm);
    cbLayout->addItems(defaultLayout);
    cbLayout->setFocusPolicy(Qt::NoFocus);
    cbLayout->setFrame(false);
    setWidgetStyle(cbLayout);
    cbLayout->hide();
    layoutPrompt->hide();
    QHBoxLayout* cbLayoutLay=new QHBoxLayout();
    cbLayoutLay->addWidget(cbLayout);
    cbLayoutLay->addStretch();

    
    ok=new QPushButton ( tr ( "Ok" ),passForm );
    setWidgetStyle ( ok );
    cancel=new QPushButton ( tr ( "Cancel" ),passForm );
    setWidgetStyle ( cancel );
    ok->hide();
    cancel->hide();
    
    

    pal.setColor ( QPalette::Button, QColor ( 255,255,255,0 ) );
    pal.setColor ( QPalette::Window, QColor ( 255,255,255,255 ) );
    pal.setColor ( QPalette::Base, QColor ( 255,255,255,255 ) );
    cbLayout->setPalette ( pal );
    ok->setPalette ( pal );
    cancel->setPalette ( pal );
    


#ifndef Q_WS_HILDON
    ok->setFixedSize ( ok->sizeHint() );
    cancel->setFixedSize ( cancel->sizeHint() );
#else
    QSize sz=cancel->sizeHint();
    sz.setWidth ( ( int ) ( sz.width() /1.5 ) );
    sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
    cancel->setFixedSize ( sz );
    sz=ok->sizeHint();
    sz.setWidth ( ( int ) ( sz.width() /1.5 ) );
    sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
    ok->setFixedSize ( sz );
#endif

    QVBoxLayout *layout=new QVBoxLayout ( passForm );
    QHBoxLayout *labelLay=new QHBoxLayout();
    QHBoxLayout *inputLay=new QHBoxLayout();
    QHBoxLayout *buttonLay=new QHBoxLayout();

    labelLay->setSpacing ( 20 );
    inputLay->setSpacing ( 10 );
    layout->setContentsMargins ( 20,20,10,10 );
    layout->addLayout ( labelLay );
    layout->addStretch();
    layout->addLayout ( inputLay );
    layout->addStretch();
    layout->addLayout ( buttonLay );

    labelLay->addWidget ( fotoLabel );
    labelLay->addWidget ( nameLabel );
    labelLay->addStretch();

    QVBoxLayout* il1=new QVBoxLayout();
    il1->addWidget ( loginPrompt );
    il1->addWidget ( passPrompt );
    il1->addWidget ( layoutPrompt );

    QVBoxLayout* il2=new QVBoxLayout();
    il2->addWidget ( login );
    il2->addWidget ( pass );
    il2->addLayout ( cbLayoutLay );
    inputLay->addLayout ( il1 );
    inputLay->addLayout ( il2 );
    inputLay->addStretch();

    buttonLay->addStretch();
    buttonLay->addWidget ( ok );
    buttonLay->addWidget ( cancel );
    buttonLay->addStretch();

    pal.setColor ( QPalette::Base, QColor ( 239,239,239,255 ) );
    login->setPalette ( pal );
    pass->setPalette ( pal );

    connect ( ok,SIGNAL ( clicked() ),this, SLOT ( slotSessEnter() ) );
    connect ( cancel,SIGNAL ( clicked() ),this, SLOT ( slotClosePass() ) );
    connect ( pass,SIGNAL ( returnPressed() ),this,
              SLOT ( slotSessEnter() ) );
    connect ( login,SIGNAL ( returnPressed() ),pass, SLOT ( selectAll() ) );
    connect ( login,SIGNAL ( returnPressed() ),pass, SLOT ( setFocus() ) );

    passPrompt->show();
    pass->show();
    ok->show();
    cancel->show();
    fotoLabel->show();
    nameLabel->show();
    if ( !useLdap )
    {
        login->show();
        loginPrompt->show();
    }
    if ( embedMode )
    {
        cancel->setEnabled ( false );
#ifdef Q_OS_WIN
        QRect r;
        wapiWindowRect ( ok->winId(),r );
#endif
    }
    if(defaultLayout.size()>1)
    {
      layoutPrompt->show();
      cbLayout->show();
      slotChangeKbdLayout(cbLayout->currentText());
      connect (cbLayout,SIGNAL(currentIndexChanged(QString)),this,SLOT(slotChangeKbdLayout(QString)));
    }
}


void ONMainWindow::initStatusDlg()
{
    sessionStatusDlg = new SVGFrame ( ":/svg/passform.svg",
                                      false,bgFrame );
    sessionStatusDlg->hide();
    if ( !miniMode )
        sessionStatusDlg->setFixedSize (
            sessionStatusDlg->sizeHint() );
    else
        sessionStatusDlg->setFixedSize ( 310,200 );
    QFont fnt=sessionStatusDlg->font();
    if ( miniMode )
#ifdef Q_WS_HILDON
        fnt.setPointSize ( 10 );
#else
        fnt.setPointSize ( 9 );
#endif
    sessionStatusDlg->setFont ( fnt );
    username->addWidget ( sessionStatusDlg );
    QPalette pal=sessionStatusDlg->palette();
    pal.setBrush ( QPalette::Window, QColor ( 0,0,0,0 ) );
    sessionStatusDlg->setPalette ( pal );

    slName=new QLabel ( sessionStatusDlg );
    slVal=new QLabel ( sessionStatusDlg );

    slName->setText (
        tr (
            "<b>Session ID:<br>Server:<br>Username:"
            "<br>Display:<br>Creation time:<br>Status:</b>" ) );
    slName->setFixedSize ( slName->sizeHint() );
    slName->hide();

    slVal->hide();
    slVal->setFixedHeight ( slName->sizeHint().height() );
    sbSusp=new QPushButton ( tr ( "Abort" ),sessionStatusDlg );
    sbTerm=new QPushButton ( tr ( "Terminate" ),sessionStatusDlg );
    sbExp=new QPushButton ( tr ( "Share folder..." ),
                            sessionStatusDlg );
    sbAdv=new QCheckBox ( tr ( "Show details" ),sessionStatusDlg );
    setWidgetStyle ( sbTerm );
    setWidgetStyle ( sbExp );
    setWidgetStyle ( sbSusp );
    setWidgetStyle ( sbAdv );

    sbAdv->setFixedSize ( sbAdv->sizeHint() );
#ifndef Q_WS_HILDON
    sbSusp->setFixedSize ( sbSusp->sizeHint() );
    sbTerm->setFixedSize ( sbTerm->sizeHint() );
    sbExp->setFixedSize ( sbExp->sizeHint() );
#else
    QSize sz=sbSusp->sizeHint();
    sz.setWidth ( ( int ) ( sz.width() /1.5 ) );
    sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
    sbSusp->setFixedSize ( sz );
    sz=sbExp->sizeHint();
    sz.setWidth ( ( int ) ( sz.width() ) );
    sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
    sbExp->setFixedSize ( sz );
    sz=sbTerm->sizeHint();
    sz.setWidth ( ( int ) ( sz.width() /1.5 ) );
    sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
    sbTerm->setFixedSize ( sz );
#endif
    sbAdv->hide();
    sbSusp->hide();
    sbTerm->hide();
    sbExp->hide();


    pal.setColor ( QPalette::Button, QColor ( 255,255,255,0 ) );
    pal.setColor ( QPalette::Window, QColor ( 255,255,255,255 ) );
    pal.setColor ( QPalette::Base, QColor ( 255,255,255,255 ) );

    sbAdv->setPalette ( pal );
    sbSusp->setPalette ( pal );
    sbTerm->setPalette ( pal );
    sbExp->setPalette ( pal );

    stInfo=new QTextEdit ( sessionStatusDlg );
    setWidgetStyle ( stInfo );
    setWidgetStyle ( stInfo->verticalScrollBar() );
    stInfo->setReadOnly ( true );
    stInfo->hide();
    stInfo->setFrameStyle ( QFrame::StyledPanel|QFrame::Plain );
    stInfo->setPalette ( pal );

    sbExp->setEnabled ( false );

    connect ( sbSusp,SIGNAL ( clicked() ),this,
              SLOT ( slotTestSessionStatus() ) );
    connect ( sbTerm,SIGNAL ( clicked() ),this,
              SLOT ( slotTermSessFromSt() ) );
    connect ( sbAdv,SIGNAL ( clicked() ),this,
              SLOT ( slotShowAdvancedStat() ) );
    connect ( sbExp,SIGNAL ( clicked() ),this,
              SLOT ( slotExportDirectory() ) );

    QVBoxLayout* layout=new QVBoxLayout ( sessionStatusDlg );
    QHBoxLayout* ll=new QHBoxLayout();
    ll->addWidget ( slName );
    ll->addWidget ( slVal );
    ll->addStretch();
    ll->setSpacing ( 10 );
    if ( !miniMode )
        layout->setContentsMargins ( 25,25,10,10 );
    else
        layout->setContentsMargins ( 10,10,10,10 );

    QHBoxLayout* bl=new QHBoxLayout();
    bl->addStretch();
    bl->addWidget ( sbExp );
    bl->addWidget ( sbSusp );
    bl->addWidget ( sbTerm );
    bl->addStretch();
    layout->addLayout ( ll );
    layout->addStretch();
    layout->addWidget ( stInfo );
    layout->addWidget ( sbAdv );
    layout->addStretch();
    layout->addLayout ( bl );


    slName->show();
    slVal->show();
    sbAdv->show();
    if ( !embedMode )
    {
        sbSusp->show();
        sbTerm->show();
        sbExp->show();
    }

    X2goSettings st ( "settings" );


    if ( st.setting()->value ( "showStatus", ( QVariant ) false ).toBool() )
    {
        sbAdv->setChecked ( true );
        slotShowAdvancedStat();
    }
#ifdef Q_OS_WIN
    if ( embedMode )
    {
        QRect r;
        wapiWindowRect ( sbAdv->winId(),r );
        wapiWindowRect ( stInfo->verticalScrollBar ()->winId(),r );
    }
#endif

}


void ONMainWindow::initSelectSessDlg()
{
    selectSessionDlg = new SVGFrame ( ":/svg/passform.svg",
                                      false,bgFrame );
    username->addWidget ( selectSessionDlg );
    setWidgetStyle ( selectSessionDlg );
    if ( !miniMode )
        selectSessionDlg->setFixedSize ( selectSessionDlg->sizeHint() );
    else
        selectSessionDlg->setFixedSize ( 310,180 );
    QPalette pal=selectSessionDlg->palette();
    pal.setBrush ( QPalette::Window, QColor ( 255,255,255,0 ) );
    selectSessionDlg->setPalette ( pal );

    pal.setColor ( QPalette::Button, QColor ( 255,255,255,0 ) );
    pal.setColor ( QPalette::Window, QColor ( 255,255,255,255 ) );
    pal.setColor ( QPalette::Base, QColor ( 255,255,255,255 ) );

    QFont fnt=selectSessionDlg->font();
    if ( miniMode )
#ifdef Q_WS_HILDON
        fnt.setPointSize ( 10 );
#else
        fnt.setPointSize ( 9 );
#endif
    selectSessionDlg->setFont ( fnt );
    selectSessionLabel=new QLabel ( tr ( "Select session:" ),
                                    selectSessionDlg );
    sOk=new QPushButton ( tr ( "Resume" ),selectSessionDlg );
    setWidgetStyle ( sOk );
    sCancel=new QPushButton ( tr ( "Cancel" ),selectSessionDlg );
    setWidgetStyle ( sCancel );
    bCancel=new QPushButton ( tr ( "Cancel" ),selectSessionDlg );
    setWidgetStyle ( bCancel );

    bSusp=new QPushButton ( tr ( "Suspend" ),selectSessionDlg );
    setWidgetStyle ( bSusp );
    bTerm=new QPushButton ( tr ( "Terminate" ),selectSessionDlg );
    setWidgetStyle ( bTerm );

    bNew=new QPushButton ( tr ( "New" ),selectSessionDlg );
    setWidgetStyle ( bNew );

    bShadow=new QPushButton ( tr ( "Full access" ),selectSessionDlg );
    setWidgetStyle ( bShadow );

    bShadowView=new QPushButton ( tr ( "View only" ),selectSessionDlg );
    setWidgetStyle ( bShadowView );

    sOk->setPalette ( pal );
    sCancel->setPalette ( pal );

    connect ( sCancel,SIGNAL ( clicked() ),this,
              SLOT ( slotCloseSelectDlg() ) );
    connect ( bCancel,SIGNAL ( clicked() ),this,
              SLOT ( slotCloseSelectDlg() ) );

    selectSessionDlg->show();
#ifndef Q_WS_HILDON
    sOk->setFixedSize ( ok->sizeHint() );
    sCancel->setFixedSize ( cancel->sizeHint() );
#else
    QSize sz=sCancel->sizeHint();
    sz.setWidth ( ( int ) ( sz.width() /1.5 ) );
    sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
    sCancel->setFixedSize ( sz );
    sz=sOk->sizeHint();
    sz.setWidth ( ( int ) ( sz.width() /1.5 ) );
    sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
    sOk->setFixedSize ( sz );
    sz=bSusp->sizeHint();
    if ( bTerm->sizeHint().width() > sz.width() )
        sz=bTerm->sizeHint();
    if ( bNew->sizeHint().width() > sz.width() )
        sz=bNew->sizeHint();
    sz.setWidth ( ( int ) ( sz.width() /1.5 ) );
    sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
    bSusp->setFixedSize ( sz );
    bTerm->setFixedSize ( sz );
    bNew->setFixedSize ( sz );
#endif
    int bmaxw=bNew->size().width();
    if ( bSusp->size().width() >bmaxw )
        bmaxw=bSusp->size().width();
    if ( bTerm->size().width() >bmaxw )
        bmaxw=bTerm->size().width();

    bNew->setFixedWidth ( bmaxw );
    bSusp->setFixedWidth ( bmaxw );
    bTerm->setFixedWidth ( bmaxw );



    sOk->setEnabled ( true );
    sCancel->setEnabled ( true );
    selectSessionDlg->setEnabled ( true );
    setEnabled ( true );

    sessTv=new QTreeView ( selectSessionDlg );
    setWidgetStyle ( sessTv );
    setWidgetStyle ( sessTv->horizontalScrollBar() );
    setWidgetStyle ( sessTv->verticalScrollBar() );
    sessTv->setItemsExpandable ( false );
    sessTv->setRootIsDecorated ( false );

    model=new QStandardItemModel ( sessions.size(), 8 );
    model->setHeaderData ( S_DISPLAY,Qt::Horizontal,
                           QVariant ( ( QString ) tr ( "Display" ) ) );
    model->setHeaderData ( S_STATUS,Qt::Horizontal,
                           QVariant ( ( QString ) tr ( "Status" ) ) );
    model->setHeaderData ( S_COMMAND,Qt::Horizontal,
                           QVariant ( ( QString ) tr ( "Command" ) ) );
    model->setHeaderData ( S_TYPE,Qt::Horizontal,
                           QVariant ( ( QString ) tr ( "Type" ) ) );
    model->setHeaderData ( S_SERVER,Qt::Horizontal,
                           QVariant ( ( QString ) tr ( "Server" ) ) );
    model->setHeaderData (
        S_CRTIME,Qt::Horizontal,
        QVariant ( ( QString ) tr ( "Creation time" ) ) );
    model->setHeaderData ( S_IP,Qt::Horizontal,
                           QVariant ( ( QString ) tr ( "Client IP" ) ) );
    model->setHeaderData ( S_ID,Qt::Horizontal,
                           QVariant ( ( QString ) tr ( "Session ID" ) ) );

    modelDesktop=new QStandardItemModel ( sessions.size(), 2 );
    modelDesktop->setHeaderData ( D_USER,Qt::Horizontal,
                                  QVariant ( ( QString ) tr ( "User" ) ) );
    modelDesktop->setHeaderData (
        D_DISPLAY,Qt::Horizontal,
        QVariant ( ( QString ) tr ( "Display" ) ) );

    sessTv->setModel ( ( QAbstractItemModel* ) model );

    QFontMetrics fm ( sessTv->font() );
    sessTv->setEditTriggers ( QAbstractItemView::NoEditTriggers );
    sessTv->setPalette ( pal );

    sessTv->setModel ( ( QAbstractItemModel* ) model );

    bNew->setPalette ( pal );
    bShadow->setPalette ( pal );
    bShadowView->setPalette ( pal );
    bSusp->setPalette ( pal );
    bTerm->setPalette ( pal );
    sessTv->setFrameStyle ( QFrame::StyledPanel|QFrame::Plain );
    sOk->setEnabled ( false );
    bSusp->setEnabled ( false );
    bTerm->setEnabled ( false );
    bShadow->setEnabled ( false );
    selectSessionLabel->hide();
    bCancel->setPalette ( pal );
    bCancel->hide();

    desktopFilter=new QLineEdit ( selectSessionDlg );
    setWidgetStyle ( desktopFilter );
// 	desktopFilter->setFrame ( false );

    desktopFilterCb=new QCheckBox ( tr ( "Only my desktops" ),
                                    selectSessionDlg );
    desktopFilterCb->hide();

    QVBoxLayout* layout=new QVBoxLayout ( selectSessionDlg );
    QHBoxLayout* filterLay=new QHBoxLayout();
    QHBoxLayout* blay=new QHBoxLayout();
    QVBoxLayout* alay=new QVBoxLayout();
    QHBoxLayout* tvlay=new QHBoxLayout();

    selectSesDlgLayout=layout;

    layout->addWidget ( selectSessionLabel );
    layout->addLayout ( filterLay );
    layout->addLayout ( tvlay );
    layout->addLayout ( blay );

    filterLay->addWidget ( desktopFilter );
    filterLay->addWidget ( desktopFilterCb );

    alay->addWidget ( bSusp );
    alay->addWidget ( bTerm );
    alay->addWidget ( bShadowView );
    alay->addWidget ( bShadow );
    alay->addStretch();
    alay->addWidget ( bNew );
    alay->addWidget ( bCancel );

    tvlay->addWidget ( sessTv );
    tvlay->addLayout ( alay );

    blay->addStretch();
    blay->addWidget ( sOk );
    blay->addWidget ( sCancel );
    blay->addStretch();
    if ( !miniMode )
        layout->setContentsMargins ( 25,25,10,10 );
    else
        layout->setContentsMargins ( 10,10,10,10 );



    sOk->hide();
    sCancel->hide();
    bNew->hide();
    bSusp->hide();
    bTerm->hide();

    connect ( sessTv,SIGNAL ( clicked ( const QModelIndex& ) ),
              this,SLOT ( slotActivated ( const QModelIndex& ) ) );

    connect ( sessTv,SIGNAL ( doubleClicked ( const QModelIndex& ) ),
              this,SLOT ( slotResumeDoubleClick ( const QModelIndex& ) ) );

    connect ( sOk,SIGNAL ( clicked() ),this, SLOT ( slotResumeSess() ) );
    connect ( bSusp,SIGNAL ( clicked() ),this, SLOT ( slotSuspendSess() ) );
    connect ( bTerm,SIGNAL ( clicked() ),this, SLOT ( slotTermSess() ) );
    connect ( bNew,SIGNAL ( clicked() ),this, SLOT ( slotNewSess() ) );

    connect ( bShadow,SIGNAL ( clicked() ),this,
              SLOT ( slotShadowSess() ) );
    connect ( bShadowView,SIGNAL ( clicked() ),this,
              SLOT ( slotShadowViewSess() ) );

    connect ( desktopFilter,SIGNAL ( textEdited ( const QString& ) ),this,
              SLOT ( slotDesktopFilterChanged ( const QString& ) ) );
    connect ( desktopFilterCb,SIGNAL ( stateChanged ( int ) ),
              this,
              SLOT ( slotDesktopFilterCb ( int ) ) );

    selectSessionLabel->show();
    sOk->show();
    sCancel->show();
    bNew->show();
    bSusp->show();
    bTerm->show();
    sessTv->show();
    selectSessionDlg->hide();
#ifdef Q_OS_WIN
    if ( embedMode )
    {
        QRect r;
        wapiWindowRect ( sOk->winId(),r );
        wapiWindowRect ( sessTv->verticalScrollBar ()->winId(),r );
        wapiWindowRect ( sessTv->horizontalScrollBar ()->winId(),r );
        wapiWindowRect ( sessTv->header ()->viewport()->winId(),r );
    }
#endif

}



void ONMainWindow::printSshDError()
{
    if ( closeEventSent )
        return;
    QMessageBox::critical ( 0l,tr ( "Error" ),
                            tr ( "sshd not started, "
                                 "you'll need sshd for printing "
                                 "and file sharing\n"
                                 "you can install sshd with\n"
                                 "<b>sudo apt-get install "
                                 "openssh-server</b>" ),
                            QMessageBox::Ok,QMessageBox::NoButton );
}

void ONMainWindow::slotStartParec ()
{
    if ( !parecTunnelOk )
    {
// 		wait 1 sec and try again
        QTimer::singleShot ( 1000, this, SLOT ( slotStartParec() ) );
        return;
    }
    SshProcess* paProc;
    QString passwd=getCurrentPass();
    QString user=getCurrentUname();
    QString host=resumingSession.server;
    QString scmd="PULSE_CLIENTCONFIG=~/.x2go/C-"+
                 resumingSession.sessionId+
                 "/.pulse-client.conf "+
                 "parec > /dev/null &sleep 1 && kill %1";

    paProc=new SshProcess ( sshConnection, this );
    paProc->startNormal ( scmd );
}


void ONMainWindow::slotSndTunOk()
{
    parecTunnelOk=true;
}


void ONMainWindow::slotPCookieReady (	bool result,
                                      QString ,
                                      SshProcess* )
{
    if ( result )
        slotStartParec();
}


void ONMainWindow::loadPulseModuleNativeProtocol()
{
    QProcess* proc=new QProcess ( this );
    QStringList args;
    args<<"load-module"<<"module-native-protocol-tcp";
    proc->start ( "pactl",args );
    proc->waitForFinished ( 3000 );
}

void ONMainWindow::slotEmbedToolBar()
{
    if ( statusLabel )
    {
        delete statusLabel;
        statusLabel=0;
    }
    if ( embedTbVisible )
    {
        stb->clear();
        act_embedToolBar->setIcon (
            QIcon ( ":icons/16x16/tbshow.png" ) );
        stb->addAction ( act_embedToolBar );
        stb->setToolButtonStyle ( Qt::ToolButtonIconOnly );
        stb->widgetForAction (
            act_embedToolBar )->setFixedHeight ( 16 );
        act_embedToolBar->setText ( tr ( "Restore toolbar" ) );
        statusLabel=new QLabel;
        stb->addWidget ( statusLabel );
#ifndef Q_OS_WIN
        statusBar()->hide();
#endif
    }
    else
    {
        initEmbedToolBar();
        act_embedToolBar->setIcon (
            QIcon ( ":icons/32x32/tbhide.png" ) );
        act_embedToolBar->setText ( tr ( "Minimize toolbar" ) );
    }
    embedTbVisible=!embedTbVisible;
    if ( proxyWinEmbedded )
        setStatStatus();
    X2goSettings st ( "sessions" );
    st.setting()->setValue ( "embedded/tbvisible", embedTbVisible );
    st.setting()->sync();
}

void ONMainWindow::initEmbedToolBar()
{
    stb->addAction ( act_embedToolBar );
    stb->addSeparator();
    stb->setToolButtonStyle ( Qt::ToolButtonTextUnderIcon );
    stb->addAction ( act_shareFolder );
    stb->addAction ( act_reconnect );
    stb->addAction ( act_suspend );
    stb->addAction ( act_terminate );
    stb->addSeparator();
    stb->addAction ( act_embedContol );
    stb->addSeparator();
    stb->addAction ( act_set );
    stb->addAction ( act_abclient );
}

void ONMainWindow::slotEmbedToolBarToolTip()
{
    if ( !showTbTooltip )
        return;
    QWidget* widg=stb->widgetForAction (
                      act_embedToolBar );
    QToolTip::showText ( this->mapToGlobal ( QPoint ( 6,6 ) ),
                         tr ( "<br><b>&nbsp;&nbsp;&nbsp;Click this "
                              "button&nbsp;&nbsp;&nbsp;<br>"
                              "&nbsp;&nbsp;&nbsp;to restore toolbar"
                              "&nbsp;&nbsp;&nbsp;</b><br>" ),
                         widg );
}


void ONMainWindow::slotActivateWindow()
{
    if ( embedMode )
    {
        QApplication::setActiveWindow ( this ) ;
        activateWindow();

        /*		x2goDebug<<"focus:"<<pass->hasFocus();
        		x2goDebug<<"activ:"<<pass->isActiveWindow();*/
        QTimer::singleShot ( 50, this,
                             SLOT ( slotEmbedToolBarToolTip() ) );
    }
}

#ifndef Q_OS_WIN
void ONMainWindow::mouseReleaseEvent ( QMouseEvent * event )
{
    QMainWindow::mouseReleaseEvent ( event );
    slotActivateWindow();
}
#endif

void ONMainWindow::slotHideEmbedToolBarToolTip()
{
    showTbTooltip=false;
    QToolTip::hideText();
}


void ONMainWindow::slotDesktopFilterChanged ( const QString& text )
{
    filterDesktops ( text );
}


void ONMainWindow::slotDesktopFilterCb ( int state )
{
    if ( state==Qt::Checked )
    {
        filterDesktops ( getCurrentUname(),true );
        desktopFilter->setEnabled ( false );
    }
    else
    {
        filterDesktops ( desktopFilter->text() );
        desktopFilter->setEnabled ( true );
    }
}


void ONMainWindow::filterDesktops ( const QString& filter, bool strict )
{
    modelDesktop->setRowCount ( 0 );
    bShadow->setEnabled ( false );
    bShadowView->setEnabled ( false );
    QFontMetrics fm ( sessTv->font() );
    uint nextRow=0;
    for ( int row = 0; row < selectedDesktops.size(); ++row )
    {
        QStringList desktop=selectedDesktops[row].split ( "@" );
        if ( filter==tr ( "Filter" ) ||filter.length() <=0||
                ( strict && desktop[0]==filter )  ||
                ( !strict && desktop[0].startsWith ( filter ) ) )
        {
            QStandardItem *item;
            item= new QStandardItem ( desktop[0] );
            modelDesktop->setItem ( nextRow,D_USER,item );
            item= new QStandardItem ( desktop[1] );
            modelDesktop->setItem ( nextRow++,D_DISPLAY,item );
            for ( int j=0;j<2;++j )
            {
                QString txt=
                    modelDesktop->index (
                        row,j ).data().toString();
                if ( sessTv->header()->sectionSize ( j ) <
                        fm.width ( txt ) +6 )
                {
                    sessTv->header()->resizeSection (
                        j,fm.width ( txt ) +6 );
                }
            }
        }
    }
}


void ONMainWindow::slotShadowSess()
{
    shadowMode=SHADOW_FULL;
    slotShadowViewSess();
}

void ONMainWindow::slotShadowViewSess()
{
    shadowUser=sessTv->model()->index ( sessTv->currentIndex().row(),
                                        D_USER ).data().toString();
    shadowDisplay=sessTv->model()->index ( sessTv->currentIndex().row(),
                                           D_DISPLAY ).data().toString();
    startNewSession();
}


void ONMainWindow::slotReconnectSession()
{
    if ( !managedMode )
        slotSelectedFromList ( ( SessionButton* ) 0 );
    else
    {
        broker->getSInfoFromBroker();
        setEnabled ( false );
    }
}


QSize ONMainWindow::getEmbedAreaSize()
{
    if ( embedTbVisible && config.showstatusbar )
        statusBar()->show();
    QSize sz=bgFrame->size();
//     sz.setHeight(sz.height()-statusBar()->size().height());
    statusBar()->hide();
    return sz;
}


void ONMainWindow::slotStartBroker()
{
    broker=new HttpBrokerClient ( this, &config );
    connect ( broker,SIGNAL ( haveSshKey ( QString ) ),this,
              SLOT ( slotStartSshAgent ( QString ) ) );
    connect ( broker,SIGNAL ( haveAgentInfo () ),this,
              SLOT ( slotStartNewBrokerSession () ) );
    connect ( broker,SIGNAL ( fatalHttpError() ),this,
              SLOT ( close() ) );
    connect ( broker,SIGNAL ( cmdReconnect() ),this,
              SLOT ( slotReconnectSession() ) );
    setStatStatus ( tr ( "Connecting to broker" ) );
    stInfo->insertPlainText ( "broker url: "+config.brokerurl );
    setEnabled ( false );

}


void ONMainWindow::slotStartNewBrokerSession ( )
{
    if ( managedMode )
    {
        setEnabled ( true );
        slotSelectedFromList ( ( SessionButton* ) 0 );
    }
}

#ifdef Q_OS_WIN
QString ONMainWindow::u3DataPath()
{
    QStringList env=QProcess::systemEnvironment();
    QString dpath;
    for ( int i=0;i<env.size();++i )
    {
// 		x2goDebug << env[i];
        //check if we have U3 System
        if ( env[i].indexOf ( "U3_APP_DATA_PATH=" ) ==0 )
        {
            dpath=env[i];
        }
        if ( env[i].indexOf ( "U3_DEVICE_PATH=" ) ==0 )
        {
            u3Device=env[i];
            u3Device.replace ( "U3_DEVICE_PATH=","" );
        }

    }

    if ( dpath.length() >0 )
    {
        dpath.replace ( "U3_APP_DATA_PATH=","" );
        portableDataPath=dpath;
        return dpath;
    }
    return QString::null;
}

#endif

void ONMainWindow::cleanPortable()
{
    QDir dr;
    dr.rmdir ( QDir::homePath() +"/.ssh" );
    removeDir ( homeDir+"/.x2go/" );
}

void ONMainWindow::removeDir ( QString path )
{
    x2goDebug<<"entering path";
    QDir dr ( path );
    QStringList files=dr.entryList ( QDir::Files );
    for ( int i=0;i<files.size();++i )
    {
        if ( files[i]!="known_hosts" )
        {
            x2goDebug<<"cleaning file:"<<path+"/"+files[i];
            dr.remove ( path+"/"+files[i] );
        }
    }
    QStringList dirs=dr.entryList ( QDir::AllDirs|QDir::NoDotAndDotDot );
    for ( int i=0;i<dirs.size();++i )
    {
        removeDir ( path+"/"+dirs[i] );
    }
    dr.rmdir ( path );
}

#ifdef Q_OS_LINUX
long ONMainWindow::X11FindWindow ( QString text, long rootWin )
{
    Window    wParent;
    Window    wRoot;
    Window   *child_list;
    unsigned  nChildren;
    long proxyId=0;
    if ( !rootWin )
        rootWin= XDefaultRootWindow ( QX11Info::display() );

    if ( XQueryTree ( QX11Info::display(),rootWin,&wRoot,&wParent,
                      &child_list,&nChildren ) )
    {
        for ( uint i=0;i<nChildren;++i )
        {
            char *wname;
            if ( XFetchName ( QX11Info::display(),
                              child_list[i],&wname ) )
            {
                QString title ( wname );
                XFree ( wname );
                if ( title==text )
                {
                    proxyId=child_list[i];
                    break;
                }
            }
            proxyId=X11FindWindow ( text, child_list[i] );
            if ( proxyId )
                break;
        }
        XFree ( child_list );
    }
    return proxyId;
}
#endif

long ONMainWindow::findWindow ( QString text )
{
#ifdef Q_OS_LINUX
    return X11FindWindow ( text );
#endif
#ifdef Q_OS_WIN
    return ( long ) wapiFindWindow ( 0,text.utf16() );
#endif
}

//////////////////////////plugin stuff//////////////

#ifdef CFGPLUGIN
void ONMainWindow::setX2goconfig ( const QString& text )
{
    m_x2goconfig=text;
    x2goDebug<<"have session config";
    initWidgetsEmbed();
}

void ONMainWindow::doPluginInit()
{
#ifdef Q_OS_LINUX
    Dl_info info;
    dladdr ( ( void* ) & ( ONMainWindow::getPortable ),&info );
    QString fname=info.dli_fname;
    x2goDebug<<"Application name:" <<fname;
    QString clientDir;
    QString pluginDir;
    int pos=fname.lastIndexOf ( "/" );
    pluginDir=fname.left ( pos );
    x2goDebug<<"Plugin Dir:" <<pluginDir;
    QDir dr ( pluginDir );
    if ( dr.exists ( "x2goclient/x2goclient" ) )
    {
        clientDir=pluginDir+"/x2goclient";
    }
    else if ( dr.exists ( "x2goclient" ) )
    {
        clientDir=pluginDir;
    }
    else
    {
        dr.cdUp();
        if ( dr.exists ( "x2goclient/x2goclient" ) )
        {
            clientDir=dr.absolutePath() +"/x2goclient";
        }
        else if ( dr.exists ( "x2goclient" ) )
        {
            clientDir=dr.absolutePath();
        }
        else
        {
            clientDir=pluginDir;
        }
    }
    x2goDebug<<"Client Dir:"<<clientDir;
    QString path=getenv ( "PATH" );
    path=clientDir+":"+pluginDir+":"+path;
    setenv ( "PATH",path.toAscii (),1 );

    path=getenv ( "LD_LIBRARY_PATH" );
    path=clientDir+":"+pluginDir+":"+path;
    setenv ( "LD_LIBRARY_PATH",path.toAscii () ,1 );

    setenv ( "X2GO_LIB",clientDir.toAscii () ,1 );

    QFile::setPermissions (
        clientDir+"/x2goclient",
        QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner|
        QFile::ReadGroup|QFile::WriteGroup|QFile::ExeGroup|
        QFile::ReadOther|QFile::WriteOther|QFile::ExeOther );
    QFile::setPermissions (
        clientDir+"/nxproxy",
        QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner|
        QFile::ReadGroup|QFile::WriteGroup|QFile::ExeGroup|
        QFile::ReadOther|QFile::WriteOther|QFile::ExeOther );
    QFile::setPermissions (
        clientDir+"/sshd",
        QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner|
        QFile::ReadGroup|QFile::WriteGroup|QFile::ExeGroup|
        QFile::ReadOther|QFile::WriteOther|QFile::ExeOther );
    QFile::setPermissions (
        clientDir+"/sftp-server",
        QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner|
        QFile::ReadGroup|QFile::WriteGroup|QFile::ExeGroup|
        QFile::ReadOther|QFile::WriteOther|QFile::ExeOther );

#endif
}


#ifndef Q_OS_DARWIN


QSize ONMainWindow::getWindowSize ( long winId )
{

#ifdef Q_OS_LINUX
    XWindowAttributes atr;
    if ( XGetWindowAttributes ( QX11Info::display(),winId,&atr ) )
        return QSize ( atr.width,atr.height );
    return QSize ( 0,0 );
#endif
#ifdef Q_OS_WIN
    QRect rec;
    if ( wapiClientRect ( ( HWND ) winId,rec ) )
        return rec.size();
    else
        return QSize ( 0,0 );
#endif
}

#ifdef Q_OS_WIN
void ONMainWindow::slotUpdateEmbedWindow()
{
    if ( oldContainerSize!=embedContainer->size() ||
            oldChildPos!= mapToGlobal (
                QPoint ( 0,0 ) ) )
    {
        QRect geom=embedContainer->geometry();
        if ( gcor==1 )
            gcor=0;
        else
            gcor=1;
        geom.setWidth ( geom.width()-gcor );
        wapiSetFSWindow ( ( HWND ) childId,
                          geom );
        wapiUpdateWindow ( ( HWND ) childId );
        oldContainerSize=embedContainer->size();
        oldChildPos= mapToGlobal (
                         QPoint ( 0,0 ) );
        x2goDebug<<"updating embedded window"<<endl;
    }
}

#endif



void ONMainWindow::embedWindow ( long wndId )
{
    childId=wndId;
    embedContainer->show();
#ifdef Q_OS_LINUX
    x2goDebug<<"embedding "<<wndId<<" in container"<<endl;
    embedContainer->embedClient ( wndId );
#endif
#ifdef Q_OS_WIN
    wapiSetParent ( ( HWND ) childId,
                    ( HWND ) ( embedContainer->winId() ) );
    oldContainerSize=embedContainer->size();
    oldChildPos= ( mapToGlobal ( QPoint ( 0,0 ) ));
    winFlags=wapiSetFSWindow ( ( HWND ) childId,
                               embedContainer->geometry() );
    updateTimer->start ( 500 );

#endif
}


void ONMainWindow::detachClient()
{
    if ( !childId )
        return;
#ifdef Q_OS_LINUX
    if ( embedContainer )
    {
        embedContainer->discardClient();
    }
#endif
#ifdef Q_OS_WIN
    wapiSetParent ( ( HWND ) childId, ( HWND ) 0 );
    slotDetachProxyWindow();
    updateTimer->stop();
    if ( childId )
    {
        wapiRestoreWindow ( ( HWND ) childId, winFlags,
                            embedContainer->geometry() );
        wapiMoveWindow ( ( HWND ) childId,0,0,
                         oldContainerSize.width(),
                         oldContainerSize.height(),true );

    }
#endif
    childId=0;
}

#endif //(Q_OS_DARWIN)





QTNPFACTORY_BEGIN ( "X2GoClient Plug-in "VERSION,
                    "Allows you to start X2Go session in a webbrowser" )
QTNPCLASS ( ONMainWindow )
QTNPFACTORY_END()

#ifdef QAXSERVER
#include <ActiveQt/QAxFactory>
QAXFACTORY_BEGIN ( "{aa3216bf-7e20-482c-84c6-06167bacb616}", "{08538ca5-eb7a-4f24-a3c4-a120c6e04dc4}" )
QAXCLASS ( ONMainWindow )
QAXFACTORY_END()
#endif
#endif
