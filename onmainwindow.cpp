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

void x2goSession::operator = ( const x2goSession& s )
{
    agentPid=s.agentPid;
    clientIp=s.clientIp;
    cookie=s.cookie;
    crTime=s.crTime;
    display=s.display;
    grPort=s.grPort;
    server=s.server;
    sessionId=s.sessionId;
    sndPort=s.sndPort;
    fsPort=s.fsPort;
    status=s.status;
}

bool	ONMainWindow::portable=false;
QString ONMainWindow::homeDir;

#ifdef Q_OS_WIN
QString ONMainWindow::u3Device;
#endif


ONMainWindow::ONMainWindow ( QWidget *parent ) :QMainWindow ( parent )
{
    x2goDebug<<"ONMainWindow constructor"<<endl;
    setFocusPolicy ( Qt::StrongFocus );
    installTranslator();
    drawMenu=true;
    usePGPCard=false;
    extLogin=false;
    startMaximized=false;
    startHidden=false;
    defaultUseSound=true;
    defaultSetKbd=true;
    defaultSetDPI=false;
    defaultDPI=96;
    extStarted=false;
    defaultLink=2;
    defaultFullscreen=false;
    acceptRsa=false;
    cardStarted=false;
    cardReady=false;
// 	useSshAgent=false;
    closeEventSent=false;
    miniMode=false;
    embedMode=false;
    proxyWinEmbedded=false;
    proxyWinId=0;
    embedParent=embedChild=0l;
    defaultSession=false;
    defaultUser=false;
    defaultWidth=800;
    defaultHeight=600;
    defaultPack="16m-jpeg";
    defaultQuality=9;
    defaultLayout=tr ( "us" );
    defaultKbdType=tr ( "pc105/us" );
    defaultCmd="KDE";
    defaultSshPort=sshPort=clientSshPort="22";
    LDAPPrintSupport=false;
    managedMode=false;
    sshProxy.use=false;
    startEmbedded=false;
    sshConnection=0;
    sessionStatusDlg=0;
    noSessionEdit=false;

#ifdef Q_OS_WIN
    clientSshPort="7022";
    pulsePort=4713;
    winSshdStarted=false;
#else
    userSshd=false;
    sshd=0l;
#endif

    appDir=QApplication::applicationDirPath();

#if defined Q_OS_WIN && defined CFGPLUGIN
    wchar_t pluginpath[1024];
    HMODULE module;
    module=GetModuleHandleW ( L"npx2goplugin.dll" );
    GetModuleFileNameW ( module,pluginpath, 1024 );
    QString ppstr=QString::fromUtf16 ( ( const ushort* ) pluginpath );
    ppstr.replace ( "\\npx2goplugin.dll","" );
    appDir=wapiShortFileName ( ppstr );
    QDir::setCurrent ( appDir );
#endif
    homeDir=QDir::homePath();

#ifdef Q_OS_WIN
    pulseServer=0l;
    xorg=0l;
    xDisplay=0;
#endif


    cleanAskPass();
    setWindowTitle ( tr ( "X2Go client" ) );
    ld=0;
    tunnel=0l;
    sndTunnel=0l;
    fsTunnel=0l;
    nxproxy=0l;
    soundServer=0l;
    scDaemon=0l;
    gpgAgent=0l;
    statusLabel=0;
    gpg=0l;
    restartResume=false;
    isPassShown=true;
    readExportsFrom=QString::null;
    spoolTimer=0l;
    ldapOnly=false;
    embedControlChanged=false;
    statusString=tr ( "connecting" );



    hide();
    kdeIconsPath=getKdeIconsPath();

    addToAppNames ( "WWWBROWSER",tr ( "Internet browser" ) );
    addToAppNames ( "MAILCLIENT",tr ( "Email client" ) );
    addToAppNames ( "OFFICE",tr ( "OpenOffice.org" ) );
    addToAppNames ( "TERMINAL",tr ( "Terminal" ) );


#ifndef Q_OS_LINUX
    widgetExtraStyle =new QPlastiqueStyle();
#endif

    QDesktopWidget wd;
// 	x2goDebug<<"Desktop geometry:"<<wd.screenGeometry();
    if ( wd.screenGeometry().width() <1024 ||
            wd.screenGeometry().height() <768 )
    {
        miniMode=true;
        x2goDebug<<"Switching to \"mini\" mode";
    }


    agentCheckTimer=new QTimer ( this );
    connect ( agentCheckTimer,SIGNAL ( timeout() ),this,
              SLOT ( slotCheckAgentProcess() ) );

#ifdef CFGCLIENT
    QStringList args=QCoreApplication::arguments();
    for ( int i=1;i<args.size();++i )
    {
        if ( !parseParameter ( args[i] ) )
        {
            close();
            exit ( -1 );
        }
    }
#endif

#ifdef CFGPLUGIN
    embedMode=true;
#endif

#ifdef Q_OS_WIN
    portableDataPath=u3DataPath();
//we have U3 System
    if ( portableDataPath.length() >0 )
    {
        ONMainWindow::portable=true;
        setWindowTitle ( "X2Go client - U3" );
    }
#endif

    if ( ONMainWindow::portable )
    {
        if ( portableDataPath.length() <=0 )
            portableDataPath=QDir::currentPath();

        homeDir=portableDataPath;
        x2goDebug<<"running in \"portable\" mode\n"<<
        "Data Dir is "<<portableDataPath;
    }

    loadSettings();
    trayIconActiveConnectionMenu = NULL;
    trayIcon = NULL;
    trayIconMenu=NULL;
    trayAutoHidden=false;

    trayEnabled=trayMinToTray=trayNoclose=trayMinCon=trayMaxDiscon=false;

    trayIconInit();



    if ( embedMode )
    {
        miniMode=false;
        useLdap=false;
    }

    if ( readExportsFrom!=QString::null )
    {
        exportTimer=new QTimer ( this );
        connect ( exportTimer,SIGNAL ( timeout() ),this,
                  SLOT ( slotExportTimer() ) );
    }
    if ( extLogin )
    {
        extTimer=new QTimer ( this );
        extTimer->start ( 2000 );
        connect ( extTimer,SIGNAL ( timeout() ),this,
                  SLOT ( slotExtTimer() ) );
    }

    if ( startMaximized )
    {
        QTimer::singleShot ( 10, this, SLOT ( slotResize() ) );
    }

    if ( usePGPCard )
    {
        QTimer::singleShot ( 10, this, SLOT ( slotStartPGPAuth() ) );
    }


    //fr=new SVGFrame(QString::null,true,this);
    fr=new IMGFrame ( ( QImage* ) 0l,this );
    setCentralWidget ( fr );


#ifndef Q_WS_HILDON
    bgFrame=new SVGFrame ( ( QString ) ":/svg/bg.svg",true,fr );
#else
    bgFrame=new SVGFrame ( ( QString ) ":/svg/bg_hildon.svg",true,fr );
#endif
    //bgFrame=new SVGFrame((QString)"/home/admin/test.svg",false,fr);


    SVGFrame* x2g=new SVGFrame ( ( QString ) ":/svg/x2gologo.svg",
                                 false,fr );

    QPalette pl=x2g->palette();
    pl.setColor ( QPalette::Base, QColor ( 255,255,255,0 ) );
    pl.setColor ( QPalette::Window, QColor ( 255,255,255,0 ) );
    x2g->setPalette ( pl );

    SVGFrame* on=new SVGFrame ( ( QString ) ":/svg/onlogo.svg",false,fr );
    on->setPalette ( pl );

    if ( !miniMode )
    {
        x2g->setFixedSize ( 100,100 );
        on->setFixedSize ( 100,100 );
    }
    else
    {
        x2g->setFixedSize ( 50,50 );
        on->setFixedSize ( 50,50 );
    }

    mainL=new QHBoxLayout ( fr );
    QVBoxLayout* onlay=new QVBoxLayout();
    onlay->addStretch();
    onlay->addWidget ( on );

    QVBoxLayout* x2golay=new QVBoxLayout();
    x2golay->addStretch();
    x2golay->addWidget ( x2g );


    QHBoxLayout* bgLay=new QHBoxLayout ( bgFrame );
    bgLay->setSpacing ( 0 );
    bgLay->setMargin ( 0 );
    bgLay->addLayout ( onlay );
    bgLay->addStretch();
    username=new QHBoxLayout();
    bgLay->addLayout ( username );
    if ( embedMode )
        bgLay->addStretch();
    bgLay->addLayout ( x2golay );



    act_set=new QAction (
        QIcon ( iconsPath ( "/32x32/edit_settings.png" ) ),
        tr ( "&Settings ..." ),this );

    act_abclient=new QAction ( QIcon ( ":icons/32x32/x2goclient.png" ),
                               tr ( "About X2GO client" ),this );



    connect ( act_set,SIGNAL ( triggered ( bool ) ),this,
              SLOT ( slotConfig() ) );
    connect ( act_abclient,SIGNAL ( triggered ( bool ) ),this,
              SLOT ( slotAbout() ) );


#ifdef Q_OS_DARWIN
    embedMode=false;
#endif

#ifdef Q_OS_WIN
    winServersReady=false;
    saveCygnusSettings();
#endif
    initPassDlg();
    initSelectSessDlg();
    initStatusDlg();

#if defined(CFGPLUGIN) && defined(Q_OS_LINUX)
    x2goDebug<<"create embedContainer"<<endl;
    embedContainer=new QX11EmbedContainer ( fr );
#endif
    if ( !embedMode )
    {
        initWidgetsNormal();
    }

#ifdef Q_OS_WIN
    QTimer::singleShot ( 500, this, SLOT ( startWinServers() ) );

#endif

    mainL->setSpacing ( 0 );
    mainL->setMargin ( 0 );
    mainL->insertWidget ( 0, bgFrame );
    hide();
    QTimer::singleShot ( 1, this, SLOT ( slotResize() ) );
    connect ( fr,SIGNAL ( resized ( const QSize ) ),this,
              SLOT ( slotResize ( const QSize ) ) );
    slotResize ( fr->size() );
    x2goDebug<<"ONMainWindows constructor finished"<<endl;
}


ONMainWindow::~ONMainWindow()
{
    x2goDebug<<"ONMainWindow destructor";
    if ( !closeEventSent )
        closeClient();
    x2goDebug<<"end of ONMainWindow destructor";
}







void ONMainWindow::installTranslator()
{
    QTranslator* x2goclientTranslator=new QTranslator();
    QString filename=QString ( ":/x2goclient_%1" ).arg (
                         QLocale::system().name() );
    filename=filename.toLower();
    if ( !x2goclientTranslator->load ( filename ) )
    {
        qDebug ( "Can't load translator (%s) !\n",
                 filename.toLocal8Bit().data() );
    }
    else
    {
        QCoreApplication::installTranslator ( x2goclientTranslator );
// 		x2goDebug<<"translator "<<filename<< " installed";
    }


    QTranslator* qtTranslator=new QTranslator;
    filename=QString ( ":/qt_%1" ).arg ( QLocale::system().name() );
    if ( !qtTranslator->load ( filename ) )
    {
        x2goDebug<< "Can't load translator "<<
        filename.toLocal8Bit().data() ;
    }
    else
    {
        QCoreApplication::installTranslator ( qtTranslator );
// 		x2goDebug<<"translator "<<filename<< " installed";
    }
}

void ONMainWindow::initWidgetsEmbed()
{
#ifdef	CFGPLUGIN
    doPluginInit();
    stb=new QToolBar ( this );
    addToolBar ( stb );
    stb->toggleViewAction()->setEnabled ( false );
    stb->toggleViewAction()->setVisible ( false );
    stb->setFloatable ( false );
    stb->setMovable ( false );
    statusBar()->setSizeGripEnabled ( false );
#ifndef Q_OS_WIN
    statusBar()->hide();
#endif
    proxyWinTimer=new QTimer ( this );
    connect ( proxyWinTimer, SIGNAL ( timeout() ), this,
              SLOT ( slotFindProxyWin() ) );


    act_shareFolder=new QAction ( QIcon ( ":icons/32x32/file-open.png" ),
                                  tr ( "Share folder..." ),this );

    act_suspend=new QAction ( QIcon ( ":icons/32x32/suspend.png" ),
                              tr ( "Suspend" ),this );

    act_terminate=new QAction ( QIcon ( ":icons/32x32/stop.png" ),
                                tr ( "Terminate" ),this );
    act_reconnect=new QAction ( QIcon ( ":icons/32x32/reconnect.png" ),
                                tr ( "Reconnect" ),this );
    act_reconnect->setEnabled ( false );

    act_embedContol=new QAction ( QIcon ( ":icons/32x32/detach.png" ),
                                  tr ( "Detach X2Go window" ),this );

    act_embedToolBar=new QAction ( QIcon ( ":icons/32x32/tbhide.png" ),
                                   tr ( "Minimize toolbar" ),this );


    setEmbedSessionActionsEnabled ( false );

    connect ( act_shareFolder,SIGNAL ( triggered ( bool ) ),this,
              SLOT ( slotExportDirectory() ) );

    connect ( act_suspend,SIGNAL ( triggered ( bool ) ),this,
              SLOT ( slotSuspendSessFromSt() ) );

    connect ( act_terminate,SIGNAL ( triggered ( bool ) ),this,
              SLOT ( slotTermSessFromSt() ) );

    connect ( act_reconnect,SIGNAL ( triggered ( bool ) ),this,
              SLOT ( slotReconnectSession() ) );

    connect ( act_embedContol,SIGNAL ( triggered ( bool ) ),this,
              SLOT ( slotEmbedControlAction() ) );

    connect ( act_embedToolBar,SIGNAL ( triggered ( bool ) ),this,
              SLOT ( slotEmbedToolBar() ) );



    processSessionConfig();

////embed container////////
#ifndef Q_OS_DARWIN

    oldParentSize=QSize ( 0,0 );

#ifdef Q_OS_WIN
    oldParentPos=QPoint ( 0,0 );
#endif
    childId=0l;
#ifdef Q_OS_LINUX

    connect ( embedContainer, SIGNAL ( clientClosed() ), this,
              SLOT ( slotDetachProxyWindow() ) );


    embedContainer->connect ( embedContainer,  SIGNAL ( clientClosed() ),
                              embedContainer,   SLOT ( hide() ) );
#endif
#ifdef Q_OS_WIN
    embedContainer=new QWidget ( mainWidget() );

    updateTimer = new QTimer ( this );
    connect ( updateTimer,  SIGNAL ( timeout() ), this,
              SLOT ( slotUpdateEmbedWindow() ) );
#endif
    embedContainer->hide();
    mainLayout()->addWidget ( embedContainer );
#endif
//end of embed container

    X2goSettings st ( "sessions" );

    embedTbVisible=!st.setting()->value (
                       "embedded/tbvisible", true ).toBool();

    slotEmbedToolBar();


    showTbTooltip=false;
    if ( !embedTbVisible )
    {
        showTbTooltip=true;
        QTimer::singleShot ( 500, this,
                             SLOT ( slotEmbedToolBarToolTip() ) );
        QTimer::singleShot ( 3000, this,
                             SLOT ( slotHideEmbedToolBarToolTip() ) );
    }
    if ( !config.showtoolbar )
    {
        stb->hide();
    }
    if ( config.confFS&& ( !config.useFs ) )
    {
        x2goDebug<<"hide share";
        act_shareFolder->setVisible ( false );
    }


    if ( !managedMode )
    {

#ifdef Q_OS_LINUX
        QTimer::singleShot ( 500, this,
                             SLOT ( slotActivateWindow() ) );
#endif

    }
#endif//CFGPLUGIN

}

void ONMainWindow::initWidgetsNormal()
{
    username->setSpacing ( 10 );
    username->addStretch();
    username->addStretch();
    ln=new SVGFrame ( ( QString ) ":/svg/line.svg",true,fr );
    ln->setFixedWidth ( ln->sizeHint().width() );
    uname=new QLineEdit ( bgFrame );
    setWidgetStyle ( uname );

    uname->hide();
    uname->setFrame ( false );
    u=new QLabel ( tr ( "Session:" ),bgFrame );
    u->hide();
    QFont fnt=u->font();
    fnt.setPointSize ( 16 );
#ifndef Q_WS_HILDON
    if ( miniMode )
    {
        fnt.setPointSize ( 12 );
    }
#endif

    u->setFont ( fnt );

    connect ( uname,SIGNAL ( returnPressed() ),this,
              SLOT ( slotUnameEntered() ) );

    QPalette pal=u->palette();
    pal.setColor ( QPalette::WindowText,
                   QColor ( 200,200,200,255 ) );
    u->setPalette ( pal );
    uname->setFont ( fnt );
    pal=uname->palette();
    pal.setColor ( QPalette::Base, QColor ( 255,255,255,0 ) );
    pal.setColor ( QPalette::Text, QColor ( 200,200,200,255 ) );
    uname->setPalette ( pal );

    u->show();
    uname->show();

    users=new QScrollArea ( fr );
    pal=users->verticalScrollBar()->palette();
    pal.setBrush ( QPalette::Window, QColor ( 110,112,127,255 ) );
    pal.setBrush ( QPalette::Base, QColor ( 110,112,127,255 ) );
    pal.setBrush ( QPalette::Button, QColor ( 110,112,127,255 ) );
    users->verticalScrollBar()->setPalette ( pal );
    users->setFrameStyle ( QFrame::Plain );
    users->setFocusPolicy ( Qt::NoFocus );


    pal=users->palette();
    pal.setBrush ( QPalette::Window, QColor ( 110,112,127,255 ) );
    users->setPalette ( pal );
    users->setWidgetResizable ( true );

    uframe=new QFrame();
    users->setWidget ( uframe );

    mainL->insertWidget ( 1, ln );
    mainL->addWidget ( users );

    QAction *act_exit=new QAction (
        QIcon ( iconsPath ( "/32x32/exit.png" ) ),
        tr ( "&Quit" ),this );
    act_exit->setShortcut ( tr ( "Ctrl+Q" ) );
    act_exit->setStatusTip ( tr ( "Quit" ) );

    act_new=new QAction ( QIcon ( iconsPath ( "/32x32/new_file.png" ) ),
                          tr ( "&New session ..." ),this );
    act_new->setShortcut ( tr ( "Ctrl+N" ) );


    setWindowIcon ( QIcon ( ":icons/128x128/x2go.png" ) );
    act_edit=new QAction ( QIcon ( iconsPath ( "/32x32/edit.png" ) ),
                           tr ( "Session management..." ),this );
    act_edit->setShortcut ( tr ( "Ctrl+E" ) );
    
    if(noSessionEdit)
    {
      act_edit->setEnabled(false);
      act_new->setEnabled(false);
    }

    act_sessicon=new QAction (
        QIcon ( iconsPath ( "/32x32/create_file.png" ) ),
        tr ( "&Create session icon on desktop..." ),
        this );


    QAction *act_tb=new QAction ( tr ( "Show toolbar" ),this );
    act_tb->setCheckable ( true );
    act_tb->setChecked ( showToolBar );



    QAction *act_abqt=new QAction ( tr ( "About Qt" ),this );

    connect ( act_abqt,SIGNAL ( triggered ( bool ) ),this,
              SLOT ( slotAboutQt() ) );
    connect ( act_new,SIGNAL ( triggered ( bool ) ),this,
              SLOT ( slotNewSession() ) );
    connect ( act_sessicon,SIGNAL ( triggered ( bool ) ),this,
              SLOT ( slotCreateSessionIcon() ) );
    connect ( act_edit,SIGNAL ( triggered ( bool ) ),this,
              SLOT ( slotManage() ) );
    connect ( act_exit,SIGNAL ( triggered ( bool ) ),this,
              SLOT ( trayQuit()) ) ;
    connect ( act_tb,SIGNAL ( toggled ( bool ) ),this,
              SLOT ( displayToolBar ( bool ) ) );
    stb=addToolBar ( tr ( "Show toolbar" ) );

    QShortcut* ex=new QShortcut ( QKeySequence ( tr ( "Ctrl+Q","exit" ) ),
                                  this );
    connect ( ex,SIGNAL ( activated() ),this,SLOT ( close() ) );

    if ( drawMenu )
    {
        QMenu* menu_sess=menuBar()->addMenu ( tr ( "&Session" ) );
        QMenu* menu_opts=menuBar()->addMenu ( tr ( "&Options" ) );

        menu_sess->addAction ( act_new );
        menu_sess->addAction ( act_edit );
#if (!defined Q_WS_HILDON) && (!defined Q_OS_DARWIN)
        if ( !portable )
            menu_sess->addAction ( act_sessicon );
#endif
        menu_sess->addSeparator();
        menu_sess->addAction ( act_exit );
        menu_opts->addAction ( act_set );
        menu_opts->addAction ( act_tb );

        QMenu* menu_help=menuBar()->addMenu ( tr ( "&Help" ) );
        menu_help->addAction ( act_abclient );
        menu_help->addAction ( act_abqt );

        stb->addAction ( act_new );
        stb->addAction ( act_edit );
#if (!defined Q_WS_HILDON) && (!defined Q_OS_DARWIN)
        if ( !portable )
            stb->addAction ( act_sessicon );
#endif
        stb->addSeparator();
        stb->addAction ( act_set );

        if ( !showToolBar )
            stb->hide();
        connect ( act_tb,SIGNAL ( toggled ( bool ) ),stb,
                  SLOT ( setVisible ( bool ) ) );
    }
    else
        stb->hide();

#if !defined USELDAP

    useLdap=false;
#endif

    if ( useLdap )
    {
        act_new->setEnabled ( false );
        act_edit->setEnabled ( false );
        u->setText ( tr ( "Login:" ) );
        QTimer::singleShot ( 1500, this, SLOT ( readUsers() ) );
    }
    else
        QTimer::singleShot ( 1, this, SLOT ( slotReadSessions() ) );
    QTimer* t=new QTimer ( this );
    connect ( t,SIGNAL ( timeout() ),this,SLOT ( slotRereadUsers() ) );
    t->start ( 20000 );
#ifdef Q_OS_WIN
    proxyWinTimer=new QTimer ( this );
    connect ( proxyWinTimer, SIGNAL ( timeout() ), this,
              SLOT ( slotFindProxyWin() ) );
#endif

}


void ONMainWindow::trayIconInit()
{

#ifndef CFGPLUGIN
    X2goSettings st ( "settings" );
    trayEnabled=st.setting()->value ( "trayicon/enabled", false ).toBool();
    trayMinCon=st.setting()->value ( "trayicon/mincon", false ).toBool();
    trayMaxDiscon=st.setting()->value ( "trayicon/maxdiscon", false ).toBool();
    trayNoclose=st.setting()->value ( "trayicon/noclose", false ).toBool();
    trayMinToTray=st.setting()->value ( "trayicon/mintotray", false ).toBool();



    if (!trayEnabled)
    {
        trayMinCon=trayMaxDiscon=trayNoclose=trayMinToTray=false;
        if (trayIcon)
        {
            delete trayIcon;
            delete trayIconMenu;
            trayIcon=0l;
            trayIconMenu=0l;

        }
    }
    else
    {
        if (!trayIcon)
        {
            trayIconMenu = new QMenu(this);
            trayIconMenu->addAction(tr("Restore"),this, SLOT(showNormal()));


            trayIconActiveConnectionMenu = trayIconMenu->addMenu(tr("Not connected"));


            trayIconActiveConnectionMenu->addAction(tr ("Share folder..." ),this, SLOT(slotExportDirectory()));
            trayIconActiveConnectionMenu->addAction(tr("Suspend"),this, SLOT(slotSuspendSessFromSt()));
            trayIconActiveConnectionMenu->addAction(tr("Terminate"),this, SLOT(slotTermSessFromSt()));

            if (sessionStatusDlg && sessionStatusDlg->isVisible())
            {
                if (!useLdap)
                    trayIconActiveConnectionMenu->setTitle(lastSession->name());
                else
                    trayIconActiveConnectionMenu->setTitle(lastUser->username());
            }
            else
                trayIconActiveConnectionMenu->setEnabled(false);
            trayIconMenu->addSeparator();
            trayIconMenu->addAction(tr("Quit"),this, SLOT(trayQuit()));

            // setup the tray icon itself
            trayIcon = new QSystemTrayIcon(this);

            connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                    this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
            connect(trayIcon, SIGNAL(messageClicked()), this, SLOT(trayMessageClicked()));

            trayIcon->setContextMenu(trayIconMenu);
            trayIcon->setIcon(QIcon ( ":icons/128x128/x2go.png") );
            trayIcon->setToolTip(tr("Left mouse button to hide/restore - Right mouse button to display context menu"));
        }
        trayIcon->show();
    }
#endif
}


QString ONMainWindow::findTheme ( QString /*theme*/ )
{
    return QString::null;
}

QString ONMainWindow::getKdeIconsPath()
{
    return ":/icons";
}

void ONMainWindow::slotResize ( const QSize sz )
{
    if ( startHidden )
    {
        return;
    }
    int height;
    int usize;
    height=sz.height();
    if ( !embedMode )
    {
        if ( !miniMode )
        {
            usize=sz.width()-800;
            if ( usize<360 )
                usize=360;
            if ( usize>500 )
                usize=500;
        }
        else
        {
            usize=285;
        }

        if ( users->width() !=usize )
        {
            users->setFixedWidth ( usize );
            if ( useLdap )
            {
                QList<UserButton*>::iterator it;
                QList<UserButton*>::iterator end=names.end();
                for ( it=names.begin();it!=end;it++ )
                {
                    if ( !miniMode )
                        ( *it )->move (
                            ( usize-360 ) /2,
                            ( *it )->pos().y() );
                    else
                        ( *it )->move (
                            ( usize-250 ) /2,
                            ( *it )->pos().y() );
                }
            }
            else
            {
                QList<SessionButton*>::iterator it;
                QList<SessionButton*>::iterator end=
                    sessions.end();
                for ( it=sessions.begin();it!=end;it++ )
                {
                    if ( !miniMode )
                        ( *it )->move (
                            ( usize-360 ) /2,
                            ( *it )->pos().y() );
                    else
                        ( *it )->move (
                            ( usize-250 ) /2,
                            ( *it )->pos().y() );
                }
            }
        }
        u->setFixedWidth ( u->sizeHint().width() );

        int bwidth=bgFrame->width();
        int upos= ( bwidth-u->width() ) /2;
        if ( upos<0 )
            upos=0;
        int rwidth=bwidth- ( upos+u->width() +5 );
        if ( rwidth<0 )
            rwidth=1;
        uname->setMinimumWidth ( rwidth );
        u->move ( upos,height/2 );
        uname->move ( u->pos().x() +u->width() +5,u->pos().y() );
    }
}

void ONMainWindow::closeClient()
{
    closeEventSent=true;
    if ( !startMaximized && !startHidden && !embedMode )
    {
        x2goDebug<<"saving settings";

        X2goSettings st ( "sizes" );

        st.setting()->setValue ( "mainwindow/size",
                                 QVariant ( size() ) );
        st.setting()->setValue ( "mainwindow/pos",QVariant ( pos() ) );
        st.setting()->setValue ( "mainwindow/maximized",
                                 QVariant ( isMaximized() ) );
        st.setting()->sync();
        x2goDebug<<"done";
    }
    if ( nxproxy!=0l )
    {
        if ( nxproxy->state() ==QProcess::Running )
        {
            x2goDebug<<"terminate proxy";
            nxproxy->terminate();
            x2goDebug<<"done";
        }
        x2goDebug<<"delete proxy process";
        delete nxproxy;
        x2goDebug<<"done";

    }
    if ( sshConnection )
    {
        sshConnection->disconnectSession();
        x2goDebug<<"waiting sshConnection to finish\n";
        sshConnection->wait ( 10000 );
        x2goDebug<<"sshConnection is closed\n";
    }

    /*	if ( tunnel!=0l )
    	{
    		x2goDebug<<"delete tunnel";
    		delete tunnel;
    		x2goDebug<<"done";
    	}
    	if ( sndTunnel!=0l )
    	{
    		x2goDebug<<"delete snd tunnel";
    		delete sndTunnel;
    		x2goDebug<<"done";
    	}
    	if ( fsTunnel!=0l )
    	{
    		x2goDebug<<"delete fs tunnel";
    		delete fsTunnel;
    		x2goDebug<<"done";
    	}
    	*/
    if ( soundServer )
    {
        x2goDebug<<"delete snd server";
        delete soundServer;
        x2goDebug<<"done";
    }
    if ( gpgAgent!=0l )
    {
        x2goDebug<<"terminate gpg-agent";
        if ( gpgAgent->state() ==QProcess::Running )
            gpgAgent->terminate();
        x2goDebug<<"done";
    }
    /*	if ( useSshAgent )
    	{
    		x2goDebug<<"terminate ssh-agent";
    		finishSshAgent();
    		x2goDebug<<"done";
    	}*/
#ifndef Q_OS_WIN
    if ( agentPid.length() >0 )
    {
        if ( checkAgentProcess() )
        {
            QStringList arg;
            arg<<"-9"<<agentPid;
            QProcess::execute ( "kill",arg );
        }
    }
#endif
#ifdef Q_OS_WIN
    if ( pulseServer )
    {
        x2goDebug<<"terminate pulse";
        pulseServer->kill();
        x2goDebug<<"done\ndelete pulse process";
        delete pulseServer;
        x2goDebug<<"done";

        QDir dr ( homeDir );
        dr.remove ( pulseDir+"/config.pa" );
        dr.remove ( pulseDir+"/pulse-pulseuser/pid" );
        dr.rmdir ( pulseDir+"/pulse-pulseuser" );
        dr.rmdir ( pulseDir );
    }
    if ( xorg )
    {
        x2goDebug<<"terminate xorg";
        xorg->terminate();
        x2goDebug<<"done\ndelete xorg process";
        delete xorg;
        x2goDebug<<"done";
    }

    if ( winSshdStarted )
    {
        TerminateProcess ( sshd.hProcess,0 );
        CloseHandle ( sshd.hProcess );
        CloseHandle ( sshd.hThread );
    }
#else
    if ( userSshd && sshd )
    {
        sshd->terminate();
        x2goDebug<<"terminating userspace sshd";
        delete sshd;
    }

#endif
    if ( embedMode )
    {
        passForm->close();
        selectSessionDlg->close();
#ifndef Q_OS_DARWIN
// 		closeEmbedWidget();
#endif
    }

    if ( ONMainWindow::portable )
    {
#ifdef Q_OS_WIN
        if ( !cyEntry )
        {
            removeCygwinEntry();
        }
        if ( !cySolEntry )
        {
            removeCySolEntry();
        }
#endif
        cleanPortable();
    }
    SshMasterConnection::finalizeLibSsh();
}

void ONMainWindow::closeEvent ( QCloseEvent* event )
{
    x2goDebug<<"close event";
    if (trayNoclose)
    {
        hide();
        event->ignore();
    }
    else
    {
        trayQuit();
    }
}


void ONMainWindow::hideEvent(QHideEvent* event)
{


    QMainWindow::hideEvent(event);
    if (event->spontaneous() && trayMinToTray)
        hide();
}



void ONMainWindow::trayQuit()
{
    closeClient();
    qApp->quit();

}

void ONMainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason )
{
    switch (reason) {
// use single left click on unix
// and double click on windows (Is it standard behaviour conform?)
#ifdef Q_OS_UNIX
    case QSystemTrayIcon::Trigger:
#else
    case QSystemTrayIcon::DoubleClick:
#endif
        if (isVisible())
            hide();
        else
        {
            showNormal();
        }
        break;
    default:
        break;
    }
}



void ONMainWindow::trayMessageClicked()
{

}

void ONMainWindow::loadSettings()
{

    X2goSettings st ( "sizes" );

    mwSize=st.setting()->value ( "mainwindow/size",
                                 ( QVariant ) QSize ( 800,600 ) ).toSize();
    mwPos=st.setting()->value ( "mainwindow/pos",
                                ( QVariant ) QPoint ( 20,20 ) ).toPoint();
    mwMax=st.setting()->value ( "mainwindow/maximized",
                                ( QVariant ) false ).toBool();
    // tray stuff
// 	trayQuitInfoShown = st1.value( "trayQuitInfoShown", false ).toBool();



    X2goSettings st1 ( "settings" );

    if ( !ldapOnly )
    {
        useLdap=st1.setting()->value ( "LDAP/useldap",
                                       ( QVariant ) false ).toBool();
        ldapServer=st1.setting()->value ( "LDAP/server",
                                          ( QVariant ) "localhost" ).toString();
        ldapPort=st1.setting()->value ( "LDAP/port",
                                        ( QVariant ) 389 ).toInt();
        ldapDn=st1.setting()->value ( "LDAP/basedn",
                                      ( QVariant ) QString::null ).toString();
        ldapServer1=st1.setting()->value ( "LDAP/server1",
                                           ( QVariant ) QString::null ).toString();
        ldapPort1=st1.setting()->value ( "LDAP/port1",
                                         ( QVariant ) 0 ).toInt();
        ldapServer2=st1.setting()->value ( "LDAP/server2",
                                           ( QVariant ) QString::null ).toString();
        ldapPort2=st1.setting()->value ( "LDAP/port2",
                                         ( QVariant ) 0 ).toInt();
    }
#ifndef Q_OS_WIN
    if ( !userSshd )
        clientSshPort=st1.setting()->value ( "clientport",
                                             ( QVariant ) 22 ).toString();
#endif
    showToolBar=st1.setting()->value ( "toolbar/show",
                                       ( QVariant ) true ).toBool();

}

QString ONMainWindow::iconsPath ( QString fname )
{
    /*    QFile fl(this->kdeIconsPath+fname);
    	if(fl.exists())
    	return kdeIconsPath+fname;*/
    return ( QString ) ":/icons"+fname;
}

void ONMainWindow::displayUsers()
{

    QPixmap pix;
    if ( !miniMode )
        pix=QPixmap ( ":/png/ico.png" );
    else
        pix=QPixmap ( ":/png/ico_mini.png" );
    QPixmap foto=QPixmap ( iconsPath ( "/64x64/personal.png" ) );

    QPalette pal=palette();
    pal.setBrush ( QPalette::Window, QBrush ( pix ) );
    pal.setBrush ( QPalette::Base, QBrush ( pix ) );
    pal.setBrush ( QPalette::Button, QBrush ( pix ) );
    QFont fnt=font();
    fnt.setPointSize ( 12 );
    uframe->setFont ( fnt );
    QList<user>::iterator it;
    QList<user>::iterator end=userList.end();
    int i=0;
    for ( it=userList.begin();it!=end;it++ )
    {
        int val=i+1;
        UserButton* l;
        if ( ( *it ).foto.isNull() )
            l=new UserButton ( this, uframe, ( *it ).uid,
                               ( *it ).name,foto,pal );
        else
            l=new UserButton ( this, uframe, ( *it ).uid,
                               ( *it ).name, ( *it ).foto,pal );
        connect ( l,SIGNAL ( userSelected ( UserButton* ) ),this,
                  SLOT ( slotSelectedFromList ( UserButton* ) ) );
        if ( !miniMode )
            l->move ( ( users->width()-360 ) /2,
                      i*120+ ( val-1 ) *25+5 );
        else
            l->move ( ( users->width()-260 ) /2,
                      i*120+ ( val-1 ) *25+5 );
        l->show();
        names.append ( l );
        i++;
    }
    int val=i;
    uframe->setFixedHeight ( val*120+ ( val-1 ) *25 );
    uname->setText ( "" );
    disconnect ( uname,SIGNAL ( textEdited ( const QString& ) ),this,
                 SLOT ( slotSnameChanged ( const QString& ) ) );
    connect ( uname,SIGNAL ( textEdited ( const QString& ) ),this,
              SLOT ( slotUnameChanged ( const QString& ) ) );
}

void ONMainWindow::showPass ( UserButton* user )
{
    QPalette pal=users->palette();
    setUsersEnabled ( false );
    QString fullName;
    QPixmap foto;
    if ( user )
    {
        foto=user->foto();
        nick=user->username();
        fullName=user->fullName();
        user->hide();
        lastUser=user;
    }
    else
    {
        lastUser=0;
        foto.load ( iconsPath ( "/64x64/personal.png" ) );
        foto=foto.scaled ( 100,100 );
        nick=uname->text();
        fullName="User Unknown";
    }

    fotoLabel->setPixmap ( foto );

    QString text="<b>"+nick+"</b><br>("+fullName+")";
    nameLabel->setText ( text );
    login->setText ( nick );
    login->hide();

    pass->setEchoMode ( QLineEdit::Password );
    pass->setFocus();
    slotShowPassForm();
}



void ONMainWindow::slotSelectedFromList ( UserButton* user )
{
    pass->setText ( "" );
    showPass ( user );
}

void ONMainWindow::slotClosePass()
{
    passForm->hide();
    if ( !embedMode )
    {
        u->show();
        uname->show();
        if ( useLdap )
        {
            if ( lastUser )
            {
                lastUser->show();
                uname->setText ( lastUser->username() );
            }
        }
        else
        {
            lastSession->show();
            uname->setText ( lastSession->name() );
        }
        uname->setEnabled ( true );
        u->setEnabled ( true );
        setUsersEnabled ( true );
        uname->selectAll();
        uname->setFocus();
    }
}


void ONMainWindow::slotPassEnter()
{
    shadowSession=false;
#if defined ( Q_OS_WIN ) || defined (Q_OS_DARWIN )
    QString disp=getXDisplay();
    if ( disp==QString::null )
        return;
#endif
#ifdef USELDAP

    if ( ! initLdapSession() )
    {
        QMessageBox::critical ( 0l,tr ( "Error" ),
                                tr ( "Please check LDAP settings" ),
                                QMessageBox::Ok,QMessageBox::NoButton );

        slotConfig();
        return;
    }

    passForm->setEnabled ( false );

    x2goServers.clear();

    list<string> attr;
    attr.push_back ( "cn" );
    attr.push_back ( "serialNumber" );
    list<LDAPStringEntry> res;
    QString searchBase="ou=Servers,ou=ON,"+ldapDn;

    try
    {
        ld->stringSearch ( searchBase.toStdString(),attr,
                           "objectClass=ipHost",res );
    }
    catch ( LDAPExeption e )
    {
        QString message="Exeption in: ";
        message=message+e.err_type.c_str();
        message=message+" : "+e.err_str.c_str();
        QMessageBox::critical ( 0l,tr ( "Error" ),message,
                                QMessageBox::Ok,QMessageBox::NoButton );
        slotConfig();
        return;
    }
    if ( res.size() ==0 )
    {
        QString message=tr ( "no X2Go server found in LDAP " );
        QMessageBox::critical ( 0l,tr ( "Error" ),message,
                                QMessageBox::Ok,QMessageBox::NoButton );
        slotConfig();
        return;
    }


    list<LDAPStringEntry>::iterator it=res.begin();
    list<LDAPStringEntry>::iterator end=res.end();
    QString freeServer;
    QString firstServer;
    bool isFirstServerSet=false;
    for ( ;it!=end;++it )
    {
        serv server;
        server.name=LDAPSession::getStringAttrValues (
                        *it,"cn" ).front().c_str();
        if ( !isFirstServerSet )
        {
            isFirstServerSet=true;
            firstServer=server.name;
        }
        QString sFactor="1";
        list<string> serialNumber=LDAPSession::getStringAttrValues (
                                      *it,"serialNumber" );
        if ( serialNumber.size() >0 )
        {
            sFactor=serialNumber.front().c_str();
        }
        x2goDebug<<server.name<<": factor is "<<sFactor;
        server.factor=sFactor.toFloat();
        server.sess=0;
        server.connOk=true;
        x2goServers.append ( server );
    }
    if ( ld )
    {
        delete ld;
        ld=0;
    }
    setEnabled ( false );
    passForm->setEnabled ( false );

    QString passwd;
    if ( !extLogin )
        currentKey=QString::null;
    QString user=getCurrentUname();
//      get x2gogetservers not from ldap server, but from first x2goserver
// 	QString host=ldapServer;
    QString host=firstServer;
    passwd=getCurrentPass();
    startSshConnection ( host,sshPort,acceptRsa,user,passwd,true );

#endif
}


void ONMainWindow::slotUnameChanged ( const QString& text )
{
    if ( prevText==text )
        return;
    if ( text=="" )
        return;
    QList<UserButton*>::iterator it;
    QList<UserButton*>::iterator endit=names.end();
    for ( it=names.begin();it!=endit;it++ )
    {
        QString username= ( *it )->username();
        if ( username.indexOf ( text,0,Qt::CaseInsensitive ) ==0 )
        {
            QPoint pos= ( *it )->pos();
            uname->setText ( username );
            QScrollBar* bar=users->verticalScrollBar();
            int docLang=bar->maximum()-bar->minimum() +
                        bar->pageStep();
            double position= ( double ) ( pos.y() ) /
                             ( double ) ( uframe->height() );
            bar->setValue ( ( int ) ( docLang*position-height() /2+
                                      ( *it )->height() /2 ) );
            uname->setSelection ( username.length(),text.length()-
                                  username.length() );
            break;
        }
    }
    prevText=text;
}

void ONMainWindow::slotUnameEntered()
{
    QString text=uname->text();
    if ( useLdap )
    {
        UserButton* user=NULL;
        QList<UserButton*>::iterator it;
        QList<UserButton*>::iterator endit=names.end();
        for ( it=names.begin();it!=endit;it++ )
        {
            QString username= ( *it )->username();
            if ( username==text )
            {
                user=*it;
                break;
            }
        }
        showPass ( user );
    }
    else
    {
        SessionButton* sess=NULL;
        QList<SessionButton*>::iterator it;
        QList<SessionButton*>::iterator endit=sessions.end();
        for ( it=sessions.begin();it!=endit;it++ )
        {
            QString name= ( *it )->name();
            if ( name==text )
            {
                sess=*it;
                break;
            }
        }
        if ( sess )
            slotSelectedFromList ( sess );
    }
}


void ONMainWindow::readUsers()
{
#ifdef USELDAP
    if ( ! initLdapSession() )
    {
        QMessageBox::critical ( 0l,tr ( "Error" ),
                                tr ( "Please check LDAP settings" ),
                                QMessageBox::Ok,QMessageBox::NoButton );

        slotConfig();
        return;
    }


    list<string> attr;
    attr.push_back ( "uidNumber" );
    attr.push_back ( "uid" );
    attr.push_back ( "cn" );
    attr.push_back ( "jpegPhoto" );


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
        QMessageBox::critical ( 0l,tr ( "Error" ),
                                message,QMessageBox::Ok,
                                QMessageBox::NoButton );
        QMessageBox::critical ( 0l,tr ( "Error" ),
                                tr ( "Please check LDAP settings" ),
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
        u.uid=LDAPSession::getBinAttrValues ( *it,
                                              "uid" ).front().getData();
        u.name=u.name.fromUtf8 ( LDAPSession::getBinAttrValues ( *it,
                                 "cn" ).front().getData() );
        list<ByteArray> lst=LDAPSession::getBinAttrValues (
                                *it,"jpegPhoto" );
        if ( lst.size() )
        {
            u.foto.loadFromData ( ( const uchar* ) (
                                      lst.front().getData() ),
                                  lst.front().length() );
        }
        userList.append ( u );
    }
    qSort ( userList.begin(),userList.end(),user::lessThen );
    delete ld;
    ld=0;
    displayUsers();
    if ( defaultUser )
    {
        defaultUser=false;
        for ( int i=0;i<userList.size();++i )
        {
            if ( userList[i].uid ==defaultUserName )
            {
                uname->setText ( defaultUserName );
                slotUnameChanged ( defaultUserName );
                QTimer::singleShot (
                    100, this,
                    SLOT ( slotUnameEntered() ) );
                break;
            }
        }
    }
#endif
}


void ONMainWindow::slotConfig()
{
    if ( !startMaximized && !startHidden && !embedMode )
    {
        X2goSettings st ( "sizes" );

        st.setting()->setValue ( "mainwindow/size",
                                 QVariant ( size() ) );
        st.setting()->setValue ( "mainwindow/pos",QVariant ( pos() ) );
        st.setting()->sync();
    }
    if ( ld )
        delete ld;
    ld=0;

    ConfigDialog dlg ( this );
    if ( dlg.exec() ==QDialog::Accepted )
    {
        int i;

        if ( passForm->isVisible() && !embedMode )
            slotClosePass();
        if ( sessionStatusDlg->isVisible() || embedMode )
        {
            trayIconInit();
            //if session is running or embed mode, save changes,
            //but not accept
            //
            return;
        }
        if ( !embedMode )
        {

            for ( i=0;i<names.size();++i )
                names[i]->close();
            for ( i=0;i<sessions.size();++i )
                sessions[i]->close();

            userList.clear();
            sessions.clear();
        }
        loadSettings();
        trayIconInit();
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
            QTimer::singleShot ( 1, this,
                                 SLOT ( slotReadSessions() ) );
        }
        slotResize ( fr->size() );
    }
}

void ONMainWindow::slotEdit ( SessionButton* bt )
{
    EditConnectionDialog dlg ( bt->id(),this );
    if ( dlg.exec() ==QDialog::Accepted )
    {
        bt->redraw();
        placeButtons();
        users->ensureVisible ( bt->x(),bt->y(),50,220 );
    }
}

void ONMainWindow::slotCreateDesktopIcon ( SessionButton* bt )
{
    bool crHidden= ( QMessageBox::question (
                         this,
                         tr ( "Create session icon on desktop" ),
                         tr ( "Desktop icons can be configured "
                              "not to show x2goclient (hidden mode). "
                              "If you like to use this feature you'll "
                              "need to configure login by a gpg key "
                              "or gpg smart card.\n\n"
                              "Use x2goclient hidden mode?" ),
                         QMessageBox::Yes|QMessageBox::No ) ==
                     QMessageBox::Yes );
    X2goSettings st ( "sessions" );

    QString name=st.setting()->value ( bt->id() +"/name",
                                       ( QVariant ) tr ( "New Session" ) ).toString() ;
    QString sessIcon=st.setting()->value (
                         bt->id() +"/icon",
                         ( QVariant )
                         ":icons/128x128/x2gosession.png"
                     ).toString();
    if ( sessIcon.startsWith ( ":icons",Qt::CaseInsensitive ) ||
            !sessIcon.endsWith ( ".png",Qt::CaseInsensitive ) )
    {
        sessIcon="/usr/share/x2goclient/icons/x2gosession.png";
    }
#ifndef Q_OS_WIN
    QFile file (
        QDesktopServices::storageLocation (
            QDesktopServices::DesktopLocation ) +"/"+name+".desktop" );
    if ( !file.open ( QIODevice::WriteOnly | QIODevice::Text ) )
        return;

    QString cmd="x2goclient";
    if ( crHidden )
        cmd="x2goclient --hide";
    QTextStream out ( &file );
    out << "[Desktop Entry]\n"<<
    "Exec[$e]="<<cmd<<" --sessionid="<<bt->id() <<"\n"<<
    "Icon="<<sessIcon<<"\n"<<
    "Name="<<name<<"\n"<<
    "StartupNotify=true\n"<<
    "Terminal=false\n"<<
    "Type=Application\n"<<
    "X-KDE-SubstituteUID=false\n";
    file.close();
#else
    QString scrname=QDir::tempPath() +"\\mklnk.vbs";
    QFile file ( scrname );
    if ( !file.open ( QIODevice::WriteOnly | QIODevice::Text ) )
        return;

    QSettings xst ( "HKEY_LOCAL_MACHINE\\SOFTWARE\\x2goclient",
                    QSettings::NativeFormat );
    QString workDir=xst.value ( "Default" ).toString();
    workDir+="\\bin";
    QString progname=workDir+"\\x2goclient.exe";
    QString args="--sessionid="+bt->id();
    if ( crHidden )
        args+=" --hide";
    QTextStream out ( &file );
    out << "Set Shell = CreateObject(\"WScript.Shell\")\n"<<
    "DesktopPath = Shell.SpecialFolders(\"Desktop\")\n"<<
    "Set link = Shell.CreateShortcut(DesktopPath & \"\\"<<name<<
    ".lnk\")\n"<<
    "link.Arguments = \""<<args<<"\"\n"<<
    "link.Description = \""<<tr ( "X2Go Link to session " ) <<
    "--"<<name<<"--"<<"\"\n"<<
    "link.TargetPath = \""<<progname<<"\"\n"<<
    "link.iconLocation = \""<<progname<<"\"\n"<<
    "link.WindowStyle = 1\n"<<
    "link.WorkingDirectory = \""<<workDir<<"\"\n"<<
    "link.Save\n";
    file.close();
    system ( scrname.toAscii() );
    QFile::remove ( scrname );
#endif
}


void ONMainWindow::slotReadSessions()
{
    X2goSettings st ( "sessions" );

    QStringList slst=st.setting()->childGroups();
    for ( int i=0;i<slst.size();++i )
    {
        if ( slst[i]!="embedded" )
            createBut ( slst[i] );
    }
    placeButtons();
    if ( slst.size() ==0 )
        slotNewSession();
    uname->setText ( "" );
    disconnect ( uname,SIGNAL ( textEdited ( const QString& ) ),this,
                 SLOT ( slotUnameChanged ( const QString& ) ) );
    connect ( uname,SIGNAL ( textEdited ( const QString& ) ),this,
              SLOT ( slotSnameChanged ( const QString& ) ) );
    if ( !defaultSession&& startHidden )
    {
        startHidden=false;
        slotResize();
        show();
        activateWindow();
        raise();

    }
    if ( defaultSession )
    {
        bool sfound=false;
        defaultSession=false;
        if ( defaultSessionId.length() >0 )
        {
            for ( int i=0;i<sessions.size();++i )
            {
                if ( sessions[i]->id() ==defaultSessionId )
                {
                    sfound=true;
                    slotSelectedFromList ( sessions[i] );
                    break;
                }
            }
        }
        else
        {
            for ( int i=0;i<sessions.size();++i )
            {
                if ( sessions[i]->name() ==defaultSessionName )
                {
                    sfound=true;
                    uname->setText ( defaultSessionName );
                    QTimer::singleShot (
                        100, this,
                        SLOT ( slotUnameEntered() ) );
                    slotSnameChanged ( defaultSessionName );
                    break;
                }
            }
        }
        if ( !sfound && startHidden )
        {
            startHidden=false;
            slotResize();
            show();
            activateWindow();
            raise();
        }
    }
}


void ONMainWindow::slotNewSession()
{
    QString id=QDateTime::currentDateTime().
               toString ( "yyyyMMddhhmmsszzz" );
    EditConnectionDialog dlg ( id, this );
    if ( dlg.exec() ==QDialog::Accepted )
    {
        SessionButton* bt=createBut ( id );
        placeButtons();
        users->ensureVisible ( bt->x(),bt->y(),50,220 );
    }
}

void ONMainWindow::slotManage()
{
    SessionManageDialog dlg ( this );
    dlg.exec();
}

void ONMainWindow::slotCreateSessionIcon()
{
    SessionManageDialog dlg ( this,true );
    dlg.exec();
}

SessionButton* ONMainWindow::createBut ( const QString& id )
{
    SessionButton* l;
    l=new SessionButton ( this,uframe,id );
    sessions.append ( l );
    connect ( l,SIGNAL ( signal_edit ( SessionButton* ) ),
              this,SLOT ( slotEdit ( SessionButton* ) ) );

    connect ( l,SIGNAL ( signal_remove ( SessionButton* ) ),
              this,SLOT ( slotDeleteButton ( SessionButton* ) ) );

    connect ( l,SIGNAL ( sessionSelected ( SessionButton* ) ),this,
              SLOT ( slotSelectedFromList ( SessionButton* ) ) );

    return l;
}


void ONMainWindow::placeButtons()
{
    qSort ( sessions.begin(),sessions.end(),SessionButton::lessThen );
    for ( int i=0;i<sessions.size();++i )
    {
        if ( !miniMode )
            sessions[i]->move ( ( users->width()-360 ) /2,
                                i*220+i*25+5 );
        else
            sessions[i]->move ( ( users->width()-260 ) /2,
                                i*155+i*20+5 );
        sessions[i]->show();
    }
    if ( sessions.size() )
    {
        if ( !miniMode )
            uframe->setFixedHeight (
                sessions.size() *220+ ( sessions.size()-1 ) *25 );
        else
            uframe->setFixedHeight (
                sessions.size() *155+ ( sessions.size()-1 ) *20 );
    }

}

void ONMainWindow::slotDeleteButton ( SessionButton * bt )
{
    if ( QMessageBox::warning (
                this,bt->name(),
                tr ( "Are you sure you want to delete this session?" ),
                QMessageBox::Yes,QMessageBox::No ) !=QMessageBox::Yes )
        return;

    X2goSettings st ( "sessions" );

    st.setting()->beginGroup ( bt->id() );
    st.setting()->remove ( "" );
    st.setting()->sync();
    sessions.removeAll ( bt );
    bt->close();
    placeButtons();
    users->ensureVisible ( 0,0,50,220 );
}


void ONMainWindow::displayToolBar ( bool show )
{
    X2goSettings st1 ( "settings" );

    st1.setting()->setValue ( "toolbar/show",show );
    st1.setting()->sync();
}


bool ONMainWindow::initLdapSession ( bool showError )
{
#ifdef USELDAP
    x2goDebug<<"initing LDAP Session"<<endl;
    try
    {
        ld=new LDAPSession ( ldapServer.toStdString(),
                             ldapPort,"","",true,false );
    }
    catch ( LDAPExeption e )
    {
        QString message="Exeption0 in: ";
        message=message+e.err_type.c_str();
        message=message+" : "+e.err_str.c_str();
        x2goDebug <<message<<endl;
        if ( ldapServer1.length() )
        {
            try
            {
                ld=new LDAPSession ( ldapServer1.toStdString(),
                                     ldapPort1,"","",
                                     true,false );
            }
            catch ( LDAPExeption e )
            {
                QString message="Exeption1 in: ";
                message=message+e.err_type.c_str();
                message=message+" : "+e.err_str.c_str();
                x2goDebug <<message<<endl;
                if ( ldapServer2.length() )
                {
                    try
                    {
                        ld=new LDAPSession (
                            ldapServer2.toStdString(),
                            ldapPort2,"","",
                            true,false );
                    }
                    catch ( LDAPExeption e )
                    {
                        QString message=
                            "Exeption2 in: ";
                        message=message+
                                e.err_type.c_str();
                        message=message+" : "+
                                e.err_str.c_str();
                        x2goDebug <<message<<endl;
                        x2goDebug<<"return false"<<endl;
                        if ( showError )
                            QMessageBox::critical (
                                0l,tr ( "Error" ),
                                message,
                                QMessageBox::Ok,
                                QMessageBox::
                                NoButton );

                        return false;
                    }
                }
                else
                {
                    x2goDebug<<"return false"<<endl;
                    if ( showError )
                        QMessageBox::critical (
                            0l,tr ( "Error" ),
                            message, QMessageBox::Ok,
                            QMessageBox::NoButton );

                    return false;
                }

            }
        }
        else
        {
            x2goDebug<<"return false"<<endl;
            if ( showError )
                QMessageBox::critical ( 0l,tr ( "Error" ),
                                        message,QMessageBox::Ok,
                                        QMessageBox::NoButton );

            return false;
        }

    }
    sessionCmd="/usr/bin/startkde";
    LDAPSndSys="ARTS_SERVER";
    LDAPSndStartServer=true;
    startSound=false;
    firstUid=0;
    lastUid=65535;


    list<string> attr;
    attr.push_back ( SESSIONCMD );
    attr.push_back ( FIRSTUID );
    attr.push_back ( LASTUID );

    list<LDAPStringEntry> res;
    QString searchBase="ou=Settings,ou=ON,"+ldapDn;
    QString srch="cn=session settings";
    try
    {
        ld->stringSearch ( searchBase.toStdString(),attr,
                           srch.toStdString(),res );
    }
    catch ( LDAPExeption e )
    {
        QString message="Exeption in: ";
        message=message+e.err_type.c_str();
        message=message+" : "+e.err_str.c_str();
        QMessageBox::critical ( 0l,tr ( "Error" ),message,
                                QMessageBox::Ok,
                                QMessageBox::NoButton );
        return false;
    }

    if ( res.size() !=0 )
    {
        LDAPStringEntry entry=res.front();
        list<string> str=LDAPSession::getStringAttrValues (
                             entry,SESSIONCMD );
        if ( str.size() )
        {
            sessionCmd=str.front().c_str();
        }
        str=LDAPSession::getStringAttrValues ( entry,FIRSTUID );
        if ( str.size() )
        {
            firstUid= ( ( QString ) str.front().c_str() ).toInt();
        }
        str=LDAPSession::getStringAttrValues ( entry,LASTUID );
        if ( str.size() )
        {
            lastUid= ( ( QString ) str.front().c_str() ).toInt();
        }
    }
    attr.clear();
    res.clear();
    attr.push_back ( NETSOUNDSYSTEM );
    attr.push_back ( SNDSUPPORT );
    attr.push_back ( SNDPORT );
    attr.push_back ( STARTSNDSERVER );


    srch="cn=sound settings";
    try
    {
        ld->stringSearch ( searchBase.toStdString(),attr,
                           srch.toStdString(),res );
    }
    catch ( LDAPExeption e )
    {
        QString message="Exeption in: ";
        message=message+e.err_type.c_str();
        message=message+" : "+e.err_str.c_str();
        QMessageBox::critical ( 0l,tr ( "Error" ),message,
                                QMessageBox::Ok,
                                QMessageBox::NoButton );
        return false;
    }

    if ( res.size() !=0 )
    {
        LDAPStringEntry entry=res.front();
        list<string> str=LDAPSession::getStringAttrValues (
                             entry,NETSOUNDSYSTEM );
        if ( str.size() )
        {
            LDAPSndSys=str.front().c_str();
        }
        if ( LDAPSndSys=="PULSE" )
        {
            LDAPSndSys="pulse";
            LDAPSndStartServer=false;
            LDAPSndPort="4713";
        }
        if ( LDAPSndSys=="ARTS_SERVER" )
        {
            LDAPSndPort="20221";
            LDAPSndSys="arts";
        }
        if ( LDAPSndSys=="ESPEAKER" )
        {
            LDAPSndPort="16001";
            LDAPSndSys="esd";
        }
        str=LDAPSession::getStringAttrValues ( entry,SNDSUPPORT );
        if ( str.size() )
        {
            startSound= ( str.front() =="yes" ) ?true:false;
        }
        str=LDAPSession::getStringAttrValues ( entry,SNDPORT );
        if ( str.size() )
        {
            LDAPSndPort=str.front().c_str();
        }
        str=LDAPSession::getStringAttrValues ( entry,STARTSNDSERVER );
        if ( str.size() )
        {
            LDAPSndStartServer=
                ( str.front() =="yes" ) ?true:false;
        }
    }
#endif
    return true;

}



void ONMainWindow::slotSnameChanged ( const QString& text )
{
    if ( prevText==text )
        return;
    if ( text=="" )
        return;
    QList<SessionButton*>::iterator it;
    QList<SessionButton*>::iterator endit=sessions.end();
    for ( it=sessions.begin();it!=endit;it++ )
    {
        QString name= ( *it )->name();
        if ( name.indexOf ( text,0,Qt::CaseInsensitive ) ==0 )
        {
            QPoint pos= ( *it )->pos();
            uname->setText ( name );
            QScrollBar* bar=users->verticalScrollBar();
            int docLang=bar->maximum()-bar->minimum() +
                        bar->pageStep();
            double position= ( double ) ( pos.y() ) /
                             ( double ) ( uframe->height() );
            bar->setValue ( ( int ) ( docLang*position-height() /
                                      2+ ( *it )->height() /2 ) );
            uname->setSelection ( name.length(),
                                  text.length()-name.length() );
            break;
        }
    }
    prevText=text;
}


void ONMainWindow::slotSelectedFromList ( SessionButton* session )
{
    pass->setText ( "" );
    lastSession=session;
    QString command;
    QString server;
    QString userName;
    bool autologin=false;
    QString sessIcon;
    QPalette pal;
    QString sessionName;
    if ( !embedMode )
    {
        session->hide();
        pal=users->palette();
        setUsersEnabled ( false );
        sessionName=session->name();

        QString sid=session->id();

        X2goSettings st ( "sessions" );

        sessIcon=st.setting()->value (
                     sid+"/icon",
                     ( QVariant ) ":icons/128x128/x2gosession.png"
                 ).toString();


        command=st.setting()->value (
                    sid+"/command",
                    ( QVariant ) tr ( "KDE" ) ).toString();

        server=st.setting()->value ( sid+"/host",
                                     ( QVariant ) QString::null
                                   ).toString();
        userName=st.setting()->value (
                     sid+"/user",
                     ( QVariant ) QString::null ).toString();
        sshPort=st.setting()->value (
                    sid+"/sshport",
                    ( QVariant ) defaultSshPort ).toString();
        currentKey=st.setting()->value (
                       sid+"/key",
                       ( QVariant ) QString::null ).toString();
        autologin=st.setting()->value (
                      sid+"/autologin",
                      ( QVariant ) QString::null ).toBool();
#ifdef Q_OS_WIN
        if ( portable &&
                u3Device.length() >0 )
        {
            currentKey.replace ( "(U3)",u3Device );
        }
#endif
    }
    else
    {
        command=config.command;
        server=config.server;
        userName=config.user;
        sshPort=config.sshport;
        sessIcon=":icons/128x128/x2gosession.png";
        sessionName=config.session;
        currentKey=config.key;
    }
    selectedCommand=command;
    command=transAppName ( command );
    login->setText ( userName );
    QPixmap pix ( sessIcon );
    if ( !miniMode )
    {
        fotoLabel->setPixmap (
            pix.scaled ( 64,64,
                         Qt::IgnoreAspectRatio,
                         Qt::SmoothTransformation ) );
        fotoLabel->setFixedSize ( 64,64 );
    }
    else
    {
        fotoLabel->setPixmap (
            pix.scaled ( 48,48,
                         Qt::IgnoreAspectRatio,
                         Qt::SmoothTransformation ) );
        fotoLabel->setFixedSize ( 48,48 );
    }


    if ( command=="RDP" )
    {
        command=tr ( "RDP connection" );
    }
    if ( command=="XDMCP" )
    {
        command=tr ( "XDMCP" );
    }
    if ( command=="SHADOW" )
    {
        command=tr ( "Connection to local desktop" );
    }

    QString text="<b>"+sessionName +"</b><br>"+
                 command+tr ( " on " ) +server;
    nameLabel->setText ( text );
    if ( userName.length() <=0 )
        login->setFocus();

    bool nopass=false;
    if ( !embedMode )
        slotShowPassForm();


///////////////////////////////////////////////////

    if ( currentKey.length() >0 )
    {
        if ( currentKey.indexOf ( "PRIVATE KEY" ) !=-1 )
        {
            if ( currentKey.indexOf ( "ENCRYPTED" ) ==-1 )
                nopass=true;
            else
                nopass=false;
        }
        else
        {
            QFile file ( currentKey );
            if ( file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
            {
                nopass=true;
                while ( !file.atEnd() )
                {
                    QString line = file.readLine();
                    if ( line.indexOf ( "ENCRYPTED" ) !=-1 )
                    {
                        nopass=false;
                        break;
                    }

                }
                file.close();
            }
            else
                currentKey=QString::null;
        }
    }
    if ( currentKey != QString::null && currentKey != "" && nopass )
    {
        x2goDebug<<"Have key, starting session"<<endl;
        slotSessEnter();
    }
    if ( cardReady ||/* useSshAgent ||*/ autologin )
    {
        nopass=true;
        if ( cardReady )
            login->setText ( cardLogin );
        slotSessEnter();
        return;
    }
    if ( startHidden && nopass==false )
    {
        startHidden=false;
        slotResize();
        show();
        activateWindow();
        raise();
    }
    if ( embedMode )
    {
        QTimer::singleShot ( 50, this,
                             SLOT ( slotShowPassForm() ) );
    }
}


void ONMainWindow::startSshConnection ( QString host, QString port, bool acceptUnknownHosts, QString login,
                                        QString password, bool autologin )
{
    x2goDebug<<"start new ssh connection"<<endl;


    for ( int i=0;i<sshEnv.size();++i )
    {
#ifndef Q_OS_WIN
        QStringList args=sshEnv[i].split ( "=" );
        x2goDebug<<"set Env "<<args[0]<<" to "<<args[1]<<endl;
        setenv ( args[0].toAscii(),args[1].toAscii(),1 );
#else
        x2goDebug<<"set Env "<<sshEnv[i]<<endl;
        _putenv ( sshEnv[i].toAscii() );
#endif
    }

    if ( usePGPCard/*||useSshAgent*/ )
        autologin=true;
    if ( cardReady )
        cardStarted=true;
    if ( sshConnection )
    {
        sshConnection->disconnectSession();
    }

    /////key/sshagent/env/

    passForm->setEnabled ( false );
    sshConnection=new SshMasterConnection ( host, port.toInt(),acceptUnknownHosts,
                                            login, password,currentKey, autologin, this );
    connect ( sshConnection,SIGNAL ( connectionOk() ), this, SLOT ( slotSshConnectionOk() ) );
    connect ( sshConnection, SIGNAL ( serverAuthError ( int,QString ) ),this,SLOT ( slotSshServerAuthError ( int,QString ) ) );
    connect ( sshConnection, SIGNAL ( userAuthError ( QString ) ),this,SLOT ( slotSshUserAuthError ( QString ) ) );
    connect ( sshConnection, SIGNAL ( connectionError ( QString,QString ) ), this, SLOT ( slotSshConnectionError ( QString,QString ) ) );
    sshConnection->start();
}

void ONMainWindow::slotSshConnectionError ( QString message, QString lastSessionError )
{
    if ( sshConnection )
    {
        delete sshConnection;
        sshConnection=0l;
    }

    QMessageBox::critical ( 0l,message,lastSessionError,
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


