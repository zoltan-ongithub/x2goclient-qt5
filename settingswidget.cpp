//
// C++ Implementation: settingswidget
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "settingswidget.h"
#include "onmainwindow.h"
#include <QBoxLayout>
#include <QRadioButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QButtonGroup>
#include <QLabel>
#include "x2gosettings.h"
#include <QDir>
#include <QApplication>
#include <QDesktopWidget>
#include <QTimer>
#include <QSplashScreen>
#include "x2gologdebug.h"
#include <QGridLayout>

SettingsWidget::SettingsWidget ( QString id, ONMainWindow * mw,
                                 QWidget * parent, Qt::WindowFlags f )
        : ConfigWidget ( id,mw,parent,f )
{
    multiDisp=(QApplication::desktop()->screenCount()>1);
#ifdef Q_WS_HILDON
    QTabWidget* tabSettings=new QTabWidget ( this );
    QFrame* dgb=new QFrame();
    QFrame* kgb=new QFrame();
    QFrame* sbgr=new QFrame();
    tabSettings->addTab ( dgb, tr ( "&Display" ) );
    tabSettings->addTab ( kgb,tr ( "&Keyboard" ) );
    tabSettings->addTab ( sbgr,tr ( "Sound" ) );
#else
    QGroupBox *dgb=new QGroupBox ( tr ( "&Display" ),this );
    kgb=new QGroupBox ( tr ( "&Keyboard" ),this );
    sbgr=new QGroupBox ( tr ( "Sound" ),this );
    rdpBox=new QGroupBox ( tr ( "RDP Client" ),this );
#endif
    QVBoxLayout *dbLay = new QVBoxLayout ( dgb );
    QVBoxLayout  *sndLay=new QVBoxLayout ( sbgr );
    QHBoxLayout* sLay=new QHBoxLayout ( );
    QVBoxLayout* sLay_sys=new QVBoxLayout ( );
    QVBoxLayout* sLay_opt=new QVBoxLayout ( );
    sLay->addLayout ( sLay_sys );
    sLay->addLayout ( sLay_opt );
    QVBoxLayout* setLay=new QVBoxLayout ( this );
    QButtonGroup* radio = new QButtonGroup ( dgb );
    fs=new QRadioButton ( tr ( "Fullscreen" ),dgb );
#ifndef Q_WS_HILDON
    custom=new QRadioButton ( tr ( "Custom" ),dgb );
#else
    custom=new QRadioButton ( tr ( "Window" ),dgb );
#endif
    display=new QRadioButton ( tr ( "Use whole display" ),dgb );
    maxRes=new QRadioButton ( tr ( "Maximum available" ),dgb );

    radio->addButton ( fs );
    radio->addButton ( custom );
    radio->setExclusive ( true );
    radio->addButton(display);
    radio->addButton(maxRes);
    width=new QSpinBox ( dgb );
    height=new QSpinBox ( dgb );
    cbSetDPI=new QCheckBox ( tr ( "Set display DPI" ),dgb );

    DPI=new QSpinBox ( dgb );
    DPI->setRange ( 1,1000 );

    cbXinerama= new QCheckBox(tr( "Xinerama extension (support for two or more physical displays)"),dgb);

    QHBoxLayout *dgLay =new QHBoxLayout();
    QHBoxLayout *dwLay =new QHBoxLayout();
    QHBoxLayout *ddLay =new QHBoxLayout();
    QHBoxLayout *dispLay= new QHBoxLayout();
    ddLay->addWidget ( cbSetDPI );
    ddLay->addWidget ( DPI );
    ddLay->addStretch();
    ddLay->setSpacing ( 15 );

    dgLay->addWidget ( fs );
    dgLay->addStretch();

    dwLay->addWidget ( custom );
    dwLay->addSpacing ( 15 );
    dwLay->addWidget ( widthLabel=new QLabel ( tr ( "Width:" ),dgb ) );
    dwLay->addWidget ( width );
    width->setRange ( 0,10000 );
    dwLay->addWidget ( heightLabel=new QLabel ( tr ( "Height:" ),dgb ) );
    dwLay->addWidget ( height );
    height->setRange ( 0,10000 );
    dwLay->addStretch();

    dispLay->addWidget(display);
    dispLay->addWidget(maxRes);
    dispLay->addSpacing(15);
    dispLay->addWidget(lDisplay=new QLabel(tr("&Display:"),dgb));
    dispLay->addWidget(displayNumber=new QSpinBox(dgb));
    pbIdentDisp=new QPushButton(tr("&Identify all displays"), dgb);
    dispLay->addWidget(pbIdentDisp);
    dispLay->addStretch();
    lDisplay->setBuddy(displayNumber);

    displayNumber->setMinimum(1);
    displayNumber->setMaximum(QApplication::desktop()->screenCount());
    if (!multiDisp)
    {
        displayNumber->hide();
        lDisplay->hide();
        pbIdentDisp->hide();
        display->hide();
    }
    lDisplay->setEnabled ( false );
    displayNumber->setEnabled ( false );


    dbLay->addLayout ( dgLay );
    dbLay->addLayout ( dwLay );
    dbLay->addLayout(dispLay);
    QFrame* dhl=new QFrame ( dgb );
    hLine1=dhl;
    dhl->setFrameStyle ( QFrame::HLine | QFrame::Sunken );
    dbLay->addWidget ( dhl );
    dbLay->addLayout ( ddLay );
    dhl=new QFrame ( dgb );
    hLine2=dhl;
    dhl->setFrameStyle ( QFrame::HLine | QFrame::Sunken );
    dbLay->addWidget ( dhl );
    dbLay->addWidget ( cbXinerama );

#ifdef Q_WS_HILDON
    width->hide();
    height->hide();
    widthLabel->hide();
    heightLabel->hide();
#endif


    kbd=new QCheckBox ( tr ( "Keep current keyboard Settings" ),kgb );
    layout=new QLineEdit ( kgb );
    type=new QLineEdit ( kgb );
    QVBoxLayout *kbLay = new QVBoxLayout ( kgb );

    QVBoxLayout *klLay = new QVBoxLayout();
    QVBoxLayout *kwLay = new QVBoxLayout();
    QHBoxLayout *ksLay = new QHBoxLayout();

    klLay->addWidget ( layoutLabel= new QLabel (
        tr ( "Keyboard layout:" ),kgb ) );
    klLay->addWidget ( typeLabel= new QLabel (
        tr ( "Keyboard model:" ),kgb ) );

    kwLay->addWidget ( layout );
    kwLay->addWidget ( type );

    ksLay->addLayout ( klLay );
    ksLay->addLayout ( kwLay );

    kbLay->addWidget ( kbd );
    kbLay->addLayout ( ksLay );

    sound=new QCheckBox ( tr ( "Enable sound support" ),sbgr );
    QButtonGroup* sndsys=new QButtonGroup;
    pulse=new QRadioButton ( "PulseAudio",sbgr );
    arts=new QRadioButton ( "arts",sbgr );
    esd=new QRadioButton ( "esd",sbgr );
    sndsys->addButton ( pulse,PULSE );
    sndsys->addButton ( arts,ARTS );
    sndsys->addButton ( esd,ESD );
    sndsys->setExclusive ( true );
    rbStartSnd=new QRadioButton ( tr ( "Start sound daemon" ),sbgr );
    rbNotStartSnd=new QRadioButton (
        tr ( "Use running sound daemon" ),sbgr );
    cbSndSshTun=new QCheckBox (
        tr ( "Use SSH port forwarding to tunnel\n"
             "sound system connections through firewalls" ),sbgr );
    cbDefSndPort=new QCheckBox ( tr ( "Use default sound port" ),sbgr );
    sbSndPort=new QSpinBox ( sbgr );
    sbSndPort->setMinimum ( 1 );
    sbSndPort->setMaximum ( 99999999 );


    QHBoxLayout *sndPortLay = new QHBoxLayout();
    lSndPort=new QLabel ( tr ( "Sound port:" ),sbgr );
    sndPortLay->addWidget ( lSndPort );
    sndPortLay->addWidget ( sbSndPort );

    sLay_sys->addWidget ( pulse );
    sLay_sys->addWidget ( arts );
    sLay_sys->addWidget ( esd );

    sLay_opt->addWidget ( rbStartSnd );
    sLay_opt->addWidget ( rbNotStartSnd );
    sLay_opt->addWidget ( cbSndSshTun );
    QFrame* hl=new QFrame ( sbgr );
    hl->setFrameStyle ( QFrame::HLine | QFrame::Sunken );
    sLay_opt->addWidget ( hl );
    sLay_opt->addWidget ( cbDefSndPort );
    sLay_opt->addLayout ( sndPortLay );
    sndLay->addWidget ( sound );
    sndLay->addLayout ( sLay );
#ifdef Q_OS_WIN
    arts->hide();
    hl->hide();
    cbDefSndPort->hide();
    lSndPort->hide();
    sbSndPort->hide();
#endif


    cbClientPrint=new QCheckBox ( tr ( "Client side printing support" ),
                                  this );
#ifdef	Q_OS_DARWIN
    arts->hide();
    pulse->hide();
    esd->setChecked ( true );
#endif

#ifndef Q_WS_HILDON
    setLay->addWidget ( dgb );
    setLay->addWidget ( kgb );
    setLay->addWidget ( sbgr );
    setLay->addWidget ( rdpBox );


    rRdesktop=new QRadioButton ("rdesktop",rdpBox );
    rRdesktop->setChecked(true);
    rXfreeRDP=new QRadioButton ( "xfreerdp",rdpBox);

    QButtonGroup* rClient=new QButtonGroup(rdpBox);
    rClient->addButton ( rRdesktop );
    rClient->addButton ( rXfreeRDP );
    rClient->setExclusive ( true );
    QGridLayout *rdpLay=new QGridLayout(rdpBox);
    rdpLay->addWidget(rRdesktop,0,0);
    rdpLay->addWidget(rXfreeRDP,1,0);
    rdpLay->addWidget(new QLabel(tr("Additional parameters:")),2,0);
    rdpLay->addWidget(new QLabel(tr("Command line:")),3,0);
    cmdLine=new QLineEdit(rdpBox);
    cmdLine->setReadOnly(true);
    params=new QLineEdit(rdpBox);
    rdpLay->addWidget(cmdLine,4,0,1,2);
    rdpLay->addWidget(params,2,1);
#else
    setLay->addWidget ( tabSettings );
// 	cbClientPrint->hide();
#endif
    setLay->addWidget ( cbClientPrint );
    setLay->addStretch();

    connect ( custom,SIGNAL ( toggled ( bool ) ),width,
              SLOT ( setEnabled ( bool ) ) );
    connect ( custom,SIGNAL ( toggled ( bool ) ),height,
              SLOT ( setEnabled ( bool ) ) );
    connect ( custom,SIGNAL ( toggled ( bool ) ),widthLabel,
              SLOT ( setEnabled ( bool ) ) );
    connect ( custom,SIGNAL ( toggled ( bool ) ),heightLabel,
              SLOT ( setEnabled ( bool ) ) );

    connect ( display,SIGNAL ( toggled ( bool ) ),displayNumber,
              SLOT ( setEnabled ( bool ) ) );
    connect ( display,SIGNAL ( toggled ( bool ) ),lDisplay,
              SLOT ( setEnabled ( bool ) ) );
    connect(pbIdentDisp, SIGNAL(clicked()), this, SLOT (slot_identDisplays()));



    connect ( cbSetDPI,SIGNAL ( toggled ( bool ) ),DPI,
              SLOT ( setEnabled ( bool ) ) );
    connect ( kbd,SIGNAL ( toggled ( bool ) ),layout,
              SLOT ( setDisabled ( bool ) ) );
    connect ( kbd,SIGNAL ( toggled ( bool ) ),layoutLabel,
              SLOT ( setDisabled ( bool ) ) );
    connect ( kbd,SIGNAL ( toggled ( bool ) ),type,
              SLOT ( setDisabled ( bool ) ) );
    connect ( kbd,SIGNAL ( toggled ( bool ) ),typeLabel,
              SLOT ( setDisabled ( bool ) ) );
    connect ( sound,SIGNAL ( toggled ( bool ) ),this,
              SLOT ( slot_sndToggled ( bool ) ) );
    connect ( sndsys,SIGNAL ( buttonClicked ( int ) ),this,
              SLOT ( slot_sndSysSelected ( int ) ) );
    connect ( rbStartSnd,SIGNAL ( clicked ( ) ),this,
              SLOT ( slot_sndStartClicked() ) );
    connect ( rbNotStartSnd,SIGNAL ( clicked ( ) ),this,
              SLOT ( slot_sndStartClicked() ) );
    connect ( cbDefSndPort,SIGNAL ( toggled ( bool ) ),this,
              SLOT ( slot_sndDefPortChecked ( bool ) ) );

    connect (rClient, SIGNAL(buttonClicked(int)), this, SLOT(updateCmdLine()));
    connect (radio, SIGNAL(buttonClicked(int)), this, SLOT(updateCmdLine()));
    connect (params, SIGNAL(textChanged(QString)), this, SLOT(updateCmdLine()));
    connect (width, SIGNAL(valueChanged(int)), this, SLOT(updateCmdLine()));
    connect (height, SIGNAL(valueChanged(int)), this, SLOT(updateCmdLine()));
    kbd->setChecked ( true );
    custom->setChecked ( true );
    readConfig();
}


SettingsWidget::~SettingsWidget()
{
}

void SettingsWidget::setDirectRdp(bool direct)
{
    cbClientPrint->setVisible(!direct);
    kgb->setVisible(!direct);
    sbgr->setVisible(!direct);
    cbSetDPI->setVisible(!direct);
    cbXinerama->setVisible(!direct);
    display->setVisible(!direct);
    maxRes->setVisible(direct);
    DPI->setVisible(!direct);
    lDisplay->setVisible(!direct);
    displayNumber->setVisible(!direct);
    pbIdentDisp->setVisible(!direct);
    hLine1->setVisible(!direct);
    hLine2->setVisible(!direct);
    rdpBox->setVisible(direct);
    if (direct)
    {
        if (display->isChecked())
        {
            display->setChecked(false);
            custom->setChecked(true);
        }
    }
    else
    {
        if (maxRes->isChecked())
        {
            maxRes->setChecked(false);
            custom->setChecked(true);
        }
    }
}

void SettingsWidget::slot_identDisplays()
{
    pbIdentDisp->setEnabled(false);
    identWins.clear();
    for (int i=0;i<QApplication::desktop()->screenCount();++i)
    {
        QMainWindow *mw=new QMainWindow(
            this, Qt::FramelessWindowHint|Qt::X11BypassWindowManagerHint|Qt::WindowStaysOnTopHint);
        mw->setFixedSize(150,200);
        QLabel* fr=new QLabel(QString::number(i+1), mw);
        QFont f=fr->font();
        f.setBold(true);
        f.setPointSize(56);
        fr->setFont(f);
        fr->setAlignment(Qt::AlignCenter);
        mw->setCentralWidget(fr);
        fr->setFrameStyle(QFrame::Box);
        QRect geom=QApplication::desktop()->screenGeometry(i);
        int x_pos=geom.width()/2-75;
        int y_pos=geom.height()/2-100;
        x_pos=565;
        identWins<<mw;
        mw ->move(geom.x()+x_pos, geom.y()+y_pos);
        mw->show();
        mw->raise();
    }
    QTimer::singleShot(1200,this, SLOT(slot_hideIdentWins()));
}

void SettingsWidget::slot_hideIdentWins()
{
    QMainWindow* mw;
    foreach(mw,identWins)
    {
        mw->close();
    }
    pbIdentDisp->setEnabled(true);
}


void SettingsWidget::slot_sndSysSelected ( int system )
{
    rbStartSnd->show();
    rbNotStartSnd->show();
    cbSndSshTun->hide();
    cbDefSndPort->setChecked ( true );
    cbDefSndPort->setEnabled ( true );

    switch ( system )
    {
    case PULSE:
    {
        rbStartSnd->hide();
        rbNotStartSnd->hide();
        cbSndSshTun->show();
        cbSndSshTun->setEnabled ( true );
        break;
    }
    case ARTS:
    {
        cbDefSndPort->setChecked ( false );
        cbDefSndPort->setEnabled ( false );
        sbSndPort->setValue ( 20221 );
        break;
    }
    case ESD:
    {
#ifdef Q_OS_WIN
        rbStartSnd->hide();
        rbNotStartSnd->hide();
        cbSndSshTun->show();
        cbSndSshTun->setEnabled ( false );
        cbSndSshTun->setChecked ( true );
#endif
        sbSndPort->setValue ( 16001 );
        break;
    }
    }
    slot_sndStartClicked();
}

void SettingsWidget::slot_sndToggled ( bool val )
{
    arts->setEnabled ( val );
    pulse->setEnabled ( val );
    esd->setEnabled ( val );

    rbStartSnd->setEnabled ( val );
    rbNotStartSnd->setEnabled ( val );

    cbSndSshTun->setEnabled ( false );
    if ( pulse->isChecked() )
        cbSndSshTun->setEnabled ( val );
    lSndPort->setEnabled ( val );
    if ( !arts->isChecked() )
        cbDefSndPort->setEnabled ( val );
    sbSndPort->setEnabled ( val );
    if ( val )
        slot_sndStartClicked();

}

void SettingsWidget::slot_sndStartClicked()
{
    bool start=rbStartSnd->isChecked();
#ifdef Q_OS_WIN
    start=false;
#endif
    if ( pulse->isChecked() )
    {
        lSndPort->setEnabled ( true );
        sbSndPort->setEnabled ( true );
        cbDefSndPort->setEnabled ( true );
    }
    else
    {
        lSndPort->setEnabled ( !start );
        sbSndPort->setEnabled ( !start );
        cbDefSndPort->setEnabled ( !start );
    }
    if ( arts->isChecked() )
        cbDefSndPort->setEnabled ( false );
    if ( ( !start  && esd->isChecked() ) ||pulse->isChecked() )
        slot_sndDefPortChecked ( cbDefSndPort->isChecked() );

}

void SettingsWidget::slot_sndDefPortChecked ( bool val )
{
    sbSndPort->setEnabled ( !val );
    lSndPort->setEnabled ( !val );
    if ( val )
    {
        if ( pulse->isChecked() )
            sbSndPort->setValue ( 4713 );
        if ( arts->isChecked() )
            sbSndPort->setValue ( 20221 );
        if ( esd->isChecked() )
            sbSndPort->setValue ( 16001 );
    }

}

void SettingsWidget::readConfig()
{

    X2goSettings st ( "sessions" );

    fs->setChecked (
        st.setting()->value ( sessionId+"/fullscreen",
                              ( QVariant ) mainWindow->getDefaultFullscreen() ).toBool() );

    custom->setChecked ( ! st.setting()->value (
                             sessionId+"/fullscreen",
                             ( QVariant ) mainWindow->getDefaultFullscreen()
                         ).toBool() );


    width->setValue (
        st.setting()->value ( sessionId+"/width",
                              ( QVariant ) mainWindow->getDefaultWidth() ).toInt() );
    height->setValue (
        st.setting()->value ( sessionId+"/height",
                              ( QVariant ) mainWindow->getDefaultHeight() ).toInt() );

    if (multiDisp)
    {
        bool md=st.setting()->value ( sessionId+"/multidisp",
                                      ( QVariant ) false).toBool();
        if (md)
            display->setChecked(true);
        int disp=st.setting()->value ( sessionId+"/display",
                                       ( QVariant ) 1).toUInt();
        if (disp<= displayNumber->maximum())
            displayNumber->setValue(disp);
        else
            displayNumber->setValue(1);
    }


    maxRes->setChecked(st.setting()->value ( sessionId+"/maxdim", false).toBool());

    QString client=st.setting()->value ( sessionId+"/rdpclient","rdesktop").toString();
    
    if(client=="rdesktop")
      rRdesktop->setChecked(true);
    else
      rXfreeRDP->setChecked(true);

    params->setText(st.setting()->value ( sessionId+"/directrdpsettings","").toString());



    cbSetDPI->setChecked (
        st.setting()->value ( sessionId+"/setdpi",
                              ( QVariant ) mainWindow->getDefaultSetDPI() ).toBool() );

    cbXinerama->setChecked (
        st.setting()->value ( sessionId+"/xinerama",
                              ( QVariant ) false).toBool());


    DPI->setEnabled ( cbSetDPI->isChecked() );
    DPI->setValue (
        st.setting()->value ( sessionId+"/dpi",
                              ( QVariant ) mainWindow->getDefaultDPI() ).toUInt() );

    kbd->setChecked ( !st.setting()->value (
                          sessionId+"/usekbd",
                          ( QVariant ) mainWindow->getDefaultSetKbd()
                      ).toBool() );
    layout->setText (
        st.setting()->value ( sessionId+"/layout",
                              ( QVariant ) mainWindow->getDefaultLayout()
                            ).toString() );
    type->setText (
        st.setting()->value ( sessionId+"/type",
                              ( QVariant ) mainWindow->getDefaultKbdType()
                            ).toString() );
    bool snd=st.setting()->value (
                 sessionId+"/sound",
                 ( QVariant ) mainWindow->getDefaultUseSound()
             ).toBool();
    QString sndsys=st.setting()->value ( sessionId+"/soundsystem",
                                         "pulse" ).toString();
    bool startServ=st.setting()->value ( sessionId+"/startsoundsystem",
                                         true ).toBool();
    bool sndInTun=st.setting()->value ( sessionId+"/soundtunnel",
                                        true ).toBool();
    bool defSndPort=st.setting()->value ( sessionId+"/defsndport",
                                          true ).toBool();
    int sndPort= st.setting()->value ( sessionId+"/sndport",4713 ).toInt();
    if ( startServ )
        rbStartSnd->setChecked ( true );
    else
        rbNotStartSnd->setChecked ( true );

    pulse->setChecked ( true );
    slot_sndSysSelected ( PULSE );
#ifdef Q_OS_WIN
    if ( sndsys=="arts" )
    {
        sndsys="pulse";
    }
#endif
    if ( sndsys=="arts" )
    {
        arts->setChecked ( true );
        slot_sndSysSelected ( ARTS );
    }
#ifdef	Q_OS_DARWIN
    sndsys="esd";
#endif
    if ( sndsys=="esd" )
    {
        esd->setChecked ( true );
        slot_sndSysSelected ( ESD );
    }
    cbSndSshTun->setChecked ( sndInTun );
    sound->setChecked ( snd );
    if ( !defSndPort )
        sbSndPort->setValue ( sndPort );
    cbDefSndPort->setChecked ( defSndPort );
    if ( sndsys=="arts" )
        cbDefSndPort->setChecked ( false );
    slot_sndToggled ( snd );
    slot_sndStartClicked();

    cbClientPrint->setChecked ( st.setting()->value ( sessionId+"/print",
                                true ).toBool() );
}

void SettingsWidget::setDefaults()
{
    fs->setChecked ( false );
    display->setChecked ( false );
    lDisplay->setEnabled ( false );
    displayNumber->setEnabled ( false );
    custom->setChecked ( true );
    width->setValue ( 800 );
    height->setValue ( 600 );

    cbSetDPI->setChecked ( mainWindow->getDefaultSetDPI() );
    DPI->setValue ( mainWindow->getDefaultDPI() );
    DPI->setEnabled ( mainWindow->getDefaultSetDPI() );

    kbd->setChecked ( !mainWindow->getDefaultSetKbd() );
    layout->setText ( tr ( "us" ) );
    type->setText ( tr ( "pc105/us" ) );
    sound->setChecked ( true );
    pulse->setChecked ( true );
    slot_sndToggled ( true );
    slot_sndSysSelected ( PULSE );
    cbSndSshTun->setChecked ( true );
    rbStartSnd->setChecked ( true );
    cbClientPrint->setChecked ( true );
    cbXinerama->setChecked ( false );
}

void SettingsWidget::saveSettings()
{
    X2goSettings st ( "sessions" );

    st.setting()->setValue ( sessionId+"/fullscreen",
                             ( QVariant ) fs->isChecked() );
    st.setting()->setValue ( sessionId+"/width",
                             ( QVariant ) width->value() );
    st.setting()->setValue ( sessionId+"/multidisp",
                             ( QVariant ) display->isChecked() );
    st.setting()->setValue ( sessionId+"/display",
                             ( QVariant ) displayNumber->value() );

    st.setting()->setValue ( sessionId+"/maxdim",
                             ( QVariant ) maxRes->isChecked() );

    if (rXfreeRDP->isChecked())
        st.setting()->setValue ( sessionId+"/rdpclient",
                                 ( QVariant ) "xfreerdp" );
    else
        st.setting()->setValue ( sessionId+"/rdpclient",
                                 ( QVariant ) "rdesktop" );
    st.setting()->setValue ( sessionId+"/directrdpsettings",
                             ( QVariant ) params->text());

    st.setting()->setValue ( sessionId+"/height",
                             ( QVariant ) height->value() );
    st.setting()->setValue ( sessionId+"/dpi",
                             ( QVariant ) DPI->value() );
    st.setting()->setValue ( sessionId+"/setdpi",
                             ( QVariant ) cbSetDPI->isChecked() );
    st.setting()->setValue ( sessionId+"/xinerama",
                             ( QVariant ) cbXinerama->isChecked() );

    st.setting()->setValue ( sessionId+"/usekbd",
                             ( QVariant ) !kbd->isChecked() );
    st.setting()->setValue ( sessionId+"/layout",
                             ( QVariant ) layout->text() );
    st.setting()->setValue ( sessionId+"/type",
                             ( QVariant ) type->text() );
    st.setting()->setValue ( sessionId+"/sound",
                             ( QVariant ) sound->isChecked() );
    if ( arts->isChecked() )
        st.setting()->setValue ( sessionId+"/soundsystem",
                                 ( QVariant ) "arts" );
    if ( esd->isChecked() )
        st.setting()->setValue ( sessionId+"/soundsystem",
                                 ( QVariant ) "esd" );
    if ( pulse->isChecked() )
        st.setting()->setValue ( sessionId+"/soundsystem",
                                 ( QVariant ) "pulse" );

    st.setting()->setValue ( sessionId+"/startsoundsystem",
                             ( QVariant ) rbStartSnd->isChecked() );
    st.setting()->setValue ( sessionId+"/soundtunnel",
                             ( QVariant ) cbSndSshTun->isChecked() );
    st.setting()->setValue ( sessionId+"/defsndport",
                             ( QVariant ) cbDefSndPort->isChecked() );
    st.setting()->setValue ( sessionId+"/sndport",
                             ( QVariant ) sbSndPort->value() );
    st.setting()->setValue ( sessionId+"/print",
                             ( QVariant ) cbClientPrint->isChecked() );
    st.setting()->sync();
}

void SettingsWidget::setServerSettings(QString server, QString port, QString user)
{
    this->server=server;
    this->port=port;
    this->user=user;
    updateCmdLine();
}

void SettingsWidget::updateCmdLine()
{
    QString client="xfreerdp";
    QString userOpt;
    if (user.length()>0)
    {
        userOpt=" -u ";
        userOpt+=user;
    }
    if (rRdesktop->isChecked())
    {
        client="rdesktop";
    }

    QString grOpt;

    if (fs->isChecked())
    {
        grOpt=" -f ";
    }
    if (maxRes->isChecked())
    {
        grOpt=" -D -g <maxW>x<maxH>";
    }
    if (custom->isChecked())
    {
        grOpt=" -g "+QString::number(width->value())+"x"+QString::number(height->value());
    }
    cmdLine->setText(client +" "+params->text()+ grOpt +userOpt+" -p <"+tr("password")+"> "+ server+":"+port );
}
