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
#include <QSettings>
#include <QDir>
SettingsWidget::SettingsWidget ( QString id, ONMainWindow * mw,
                                 QWidget * parent, Qt::WindowFlags f )
		: ConfigWidget ( id,mw,parent,f )
{
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
	QGroupBox *kgb=new QGroupBox ( tr ( "&Keyboard" ),this );
	QGroupBox *sbgr=new QGroupBox ( tr ( "Sound" ),this );
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
	radio->addButton ( fs );
	radio->addButton ( custom );
	radio->setExclusive ( true );
	width=new QSpinBox ( dgb );
	height=new QSpinBox ( dgb );
	cbSetDPI=new QCheckBox ( tr ( "Set display DPI" ),dgb );

	DPI=new QSpinBox ( dgb );
	DPI->setRange ( 1,1000 );

	QHBoxLayout *dgLay =new QHBoxLayout();
	QHBoxLayout *dwLay =new QHBoxLayout();
	QHBoxLayout *ddLay =new QHBoxLayout();
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
	dbLay->addLayout ( dgLay );
	dbLay->addLayout ( dwLay );
	QFrame* dhl=new QFrame ( dgb );
	dhl->setFrameStyle ( QFrame::HLine | QFrame::Sunken );
	dbLay->addWidget ( dhl );
	dbLay->addLayout ( ddLay );

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
	kbd->setChecked ( true );
	custom->setChecked ( true );
	readConfig();
}


SettingsWidget::~SettingsWidget()
{
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

#ifndef Q_OS_WIN
	QSettings st ( QDir::homePath() +"/.x2goclient/sessions",
	               QSettings::NativeFormat );
#else
	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "sessions" );
#endif

	fs->setChecked (
	    st.value ( sessionId+"/fullscreen",
	               ( QVariant ) mainWindow->getDefaultFullscreen() ).toBool() );

	custom->setChecked ( ! st.value (
	                         sessionId+"/fullscreen",
	                         ( QVariant ) mainWindow->getDefaultFullscreen()
	                     ).toBool() );


	width->setValue (
	    st.value ( sessionId+"/width",
	               ( QVariant ) mainWindow->getDefaultWidth() ).toInt() );
	height->setValue (
	    st.value ( sessionId+"/height",
	               ( QVariant ) mainWindow->getDefaultHeight() ).toInt() );

	cbSetDPI->setChecked (
	    st.value ( sessionId+"/setdpi",
	               ( QVariant ) mainWindow->getDefaultSetDPI() ).toBool() );
	DPI->setEnabled ( cbSetDPI->isChecked() );
	DPI->setValue (
	    st.value ( sessionId+"/dpi",
	               ( QVariant ) mainWindow->getDefaultDPI() ).toUInt() );

	kbd->setChecked ( !st.value (
	                      sessionId+"/usekbd",
	                      ( QVariant ) mainWindow->getDefaultSetKbd()
	                  ).toBool() );
	layout->setText (
	    st.value ( sessionId+"/layout",
	               ( QVariant ) mainWindow->getDefaultLayout()
	             ).toString() );
	type->setText (
	    st.value ( sessionId+"/type",
	               ( QVariant ) mainWindow->getDefaultKbdType()
	             ).toString() );
	bool snd=st.value (
	             sessionId+"/sound",
	             ( QVariant ) mainWindow->getDefaultUseSound()
	         ).toBool();
	QString sndsys=st.value ( sessionId+"/soundsystem","pulse" ).toString();
	bool startServ=st.value ( sessionId+"/startsoundsystem",
	                          true ).toBool();
	bool sndInTun=st.value ( sessionId+"/soundtunnel", true ).toBool();
	bool defSndPort=st.value ( sessionId+"/defsndport", true ).toBool();
	int sndPort= st.value ( sessionId+"/sndport",4713 ).toInt();
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

	cbClientPrint->setChecked ( st.value ( sessionId+"/print",
	                                       true ).toBool() );
}

void SettingsWidget::setDefaults()
{
	fs->setChecked ( false );
	custom->setChecked ( true );
	width->setValue ( 800 );
	height->setValue ( 600 );

	cbSetDPI->setChecked ( mainWindow->getDefaultSetDPI() );
	DPI->setValue ( mainWindow->getDefaultDPI() );
	DPI->setEnabled ( mainWindow->getDefaultSetDPI() );

	kbd->setChecked (!mainWindow->getDefaultSetKbd());
	layout->setText ( tr ( "us" ) );
	type->setText ( tr ( "pc105/us" ) );
	sound->setChecked ( true );
	pulse->setChecked ( true );
	slot_sndToggled ( true );
	slot_sndSysSelected ( PULSE );
	cbSndSshTun->setChecked ( true );
	rbStartSnd->setChecked ( true );
	cbClientPrint->setChecked ( true );
}

void SettingsWidget::saveSettings()
{
#ifndef Q_OS_WIN
	QSettings st ( QDir::homePath() +"/.x2goclient/sessions",
	               QSettings::NativeFormat );
#else
	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "sessions" );
#endif
	st.setValue ( sessionId+"/fullscreen", ( QVariant ) fs->isChecked() );
	st.setValue ( sessionId+"/width", ( QVariant ) width->value() );
	st.setValue ( sessionId+"/height", ( QVariant ) height->value() );
	st.setValue ( sessionId+"/dpi", ( QVariant ) DPI->value() );
	st.setValue ( sessionId+"/setdpi", ( QVariant ) cbSetDPI->isChecked() );
	st.setValue ( sessionId+"/usekbd", ( QVariant ) !kbd->isChecked() );
	st.setValue ( sessionId+"/layout", ( QVariant ) layout->text() );
	st.setValue ( sessionId+"/type", ( QVariant ) type->text() );
	st.setValue ( sessionId+"/sound", ( QVariant ) sound->isChecked() );
	if ( arts->isChecked() )
		st.setValue ( sessionId+"/soundsystem", ( QVariant ) "arts" );
	if ( esd->isChecked() )
		st.setValue ( sessionId+"/soundsystem", ( QVariant ) "esd" );
	if ( pulse->isChecked() )
		st.setValue ( sessionId+"/soundsystem", ( QVariant ) "pulse" );

	st.setValue ( sessionId+"/startsoundsystem",
	              ( QVariant ) rbStartSnd->isChecked() );
	st.setValue ( sessionId+"/soundtunnel",
	              ( QVariant ) cbSndSshTun->isChecked() );
	st.setValue ( sessionId+"/defsndport",
	              ( QVariant ) cbDefSndPort->isChecked() );
	st.setValue ( sessionId+"/sndport", ( QVariant ) sbSndPort->value() );
	st.setValue ( sessionId+"/print",
	              ( QVariant ) cbClientPrint->isChecked() );
	st.sync();
}
