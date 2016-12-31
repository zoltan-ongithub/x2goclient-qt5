/**************************************************************************
*   Copyright (C) 2005-2016 by Oleksandr Shneyder                         *
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

#include "mediawidget.h"
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

MediaWidget::MediaWidget ( QString id, ONMainWindow * mw,
                           QWidget * parent, Qt::WindowFlags f )
    : ConfigWidget ( id,mw,parent,f )
{
#ifdef Q_WS_HILDON
    QTabWidget* tabSettings=new QTabWidget ( this );
    QFrame* sbgr=new QFrame();
    tabSettings->addTab ( sbgr,tr ( "Sound" ) );
#else
    sbgr=new QGroupBox ( tr ( "Sound" ),this );
#endif
    QVBoxLayout *sndLay=new QVBoxLayout ( sbgr );
    QHBoxLayout* sLay=new QHBoxLayout ( );
    QVBoxLayout* sLay_sys=new QVBoxLayout ( );
    QVBoxLayout* sLay_opt=new QVBoxLayout ( );
    sLay->addLayout ( sLay_sys );
    sLay->addLayout ( sLay_opt );
    QVBoxLayout* setLay=new QVBoxLayout ( this );

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
    sbSndPort->setMaximum ( 65535 );


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
#if defined (Q_OS_WIN) || defined (Q_OS_DARWIN)
    arts->hide();
    hl->hide();
    cbDefSndPort->hide();
    lSndPort->hide();
    sbSndPort->hide();
#endif /* defined (Q_OS_WIN) || defined (Q_OS_DARWIN) */


    cbClientPrint=new QCheckBox ( tr ( "Client side printing support" ),
                                  this );
    setLay->addWidget(sbgr);
    setLay->addWidget ( cbClientPrint );
    setLay->addStretch();

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

    setDefaults();
    readConfig();
}


MediaWidget::~MediaWidget()
{
}

void MediaWidget::slot_sndSysSelected ( int system )
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
        QMessageBox::warning (NULL, tr ("Deprecation Warning"),
                              tr ("ARTS support is scheduled to be removed soon.\n\n"

                                  "Please upgrade to PulseAudio."),
                              QMessageBox::Ok, QMessageBox::NoButton);
        break;
    }
    case ESD:
    {
        rbStartSnd->hide ();
        rbNotStartSnd->hide ();
        cbSndSshTun->show ();
#ifdef Q_OS_WIN
        cbSndSshTun->setEnabled ( false );
        cbSndSshTun->setChecked ( true );
#elif defined (Q_OS_DARWIN)
        cbSndSshTun->setEnabled (true);
#endif /* defined (Q_OS_WIN) */
        sbSndPort->setValue ( 16001 );
        break;
    }
    }
    slot_sndStartClicked();
}

void MediaWidget::slot_sndToggled ( bool val )
{
    arts->setEnabled ( val );
    pulse->setEnabled ( val );
    esd->setEnabled ( val );

    rbStartSnd->setEnabled ( val );
    rbNotStartSnd->setEnabled ( val );

    cbSndSshTun->setEnabled ( false );
    /* ESD is also handled by PA on Windows and OS X. */
#if defined (Q_OS_WIN) || defined (Q_OS_DARWIN)
    if ((pulse->isChecked ()) || (esd->isChecked ())) {
        cbSndSshTun->setEnabled (val);
    }
#else
    if (pulse->isChecked ()) {
        cbSndSshTun->setEnabled ( val );
    }
#endif /* defined (Q_OS_WIN) || defined (Q_OS_DARWIN) */
    lSndPort->setEnabled ( val );
    if ( !arts->isChecked() )
        cbDefSndPort->setEnabled ( val );
    sbSndPort->setEnabled ( val );
    if ( val )
        slot_sndStartClicked();

}

void MediaWidget::slot_sndStartClicked()
{
    bool start=rbStartSnd->isChecked();
#ifdef Q_OS_WIN
    start=false;
#endif
    if ( pulse->isChecked() )
    {
        lSndPort->setEnabled ( true );
        sbSndPort->setEnabled ( true );
        cbDefSndPort->setEnabled (sound->isChecked());
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

void MediaWidget::slot_sndDefPortChecked ( bool val )
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

void MediaWidget::readConfig()
{

    X2goSettings st ( "sessions" );

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
#if defined (Q_OS_WIN) || defined (Q_OS_DARWIN)
    if ( sndsys=="arts" )
    {
        sndsys="pulse";
    }
#endif /* defined (Q_OS_WIN) || defined (Q_OS_DARWIN) */
    if ( sndsys=="arts" )
    {
        arts->setChecked ( true );
        slot_sndSysSelected ( ARTS );
    }
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

    if(!sound)
        cbDefSndPort->setEnabled(false);

    cbClientPrint->setChecked ( st.setting()->value ( sessionId+"/print",
                                true ).toBool() );
}

void MediaWidget::setDefaults()
{
    sound->setChecked ( true );
    pulse->setChecked ( true );
    slot_sndToggled ( true );
    slot_sndSysSelected ( PULSE );
    cbSndSshTun->setChecked ( true );
    rbStartSnd->setChecked ( true );
    cbClientPrint->setChecked ( true );
}

void MediaWidget::saveSettings()
{
    X2goSettings st ( "sessions" );

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
