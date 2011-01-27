//
// C++ Implementation: sharewidget
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "sharewidget.h"
#include "onmainwindow.h"

#include <QTreeView>
#include <QStandardItemModel>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QCheckBox>
#include <QBoxLayout>
#include <QHeaderView>
#include <QComboBox>
#include <QTextStream>

#include "x2gologdebug.h"

#include <QFileDialog>
#include <QDir>
#include <QSettings>

ShareWidget::ShareWidget ( QString id, ONMainWindow * mw,
                           QWidget * parent, Qt::WindowFlags f )
		: ConfigWidget ( id,mw,parent,f )
{
	QGroupBox *egb=new QGroupBox ( tr ( "&Folders" ),this );
	expTv=new QTreeView ( egb );
	expTv->setItemsExpandable ( false );
	expTv->setRootIsDecorated ( false );

	model=new QStandardItemModel ( 0,2 );
	ldir=new QLabel ( egb );


	model->setHeaderData ( 0,Qt::Horizontal,QVariant (
	                           ( QString ) tr ( "Path" ) ) );
	model->setHeaderData ( 1,Qt::Horizontal,QVariant (
	                           ( QString ) tr ( "Automount" ) ) );
	expTv->setEditTriggers ( QAbstractItemView::NoEditTriggers );



	QPushButton* openDir=new QPushButton (
	    QIcon ( mainWindow->iconsPath ( "/16x16/file-open.png" ) ),
	    QString::null,egb );

	QPushButton* addDir=new QPushButton ( tr ( "Add" ),egb );
	QPushButton* delDir=new QPushButton ( tr ( "Delete" ),egb );
#ifdef Q_WS_HILDON
	QSize sz=addDir->sizeHint();
	sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
	addDir->setFixedSize ( sz );
	sz=delDir->sizeHint();
	sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
	delDir->setFixedSize ( sz );
#endif

	QLabel *dirPrompt=new QLabel ( tr ( "Path:" ),egb );
	dirPrompt->setFixedSize ( dirPrompt->sizeHint() );
	openDir->setFixedSize ( openDir->sizeHint() );

	ldir->setFrameStyle ( QFrame::StyledPanel|QFrame::Sunken );

	cbFsConv=new QCheckBox (
	    tr ( "Filename encoding"
	       ),egb );

	QHBoxLayout* enclay=new QHBoxLayout;
	cbFrom=new QComboBox ( egb );
	cbTo=new QComboBox ( egb );
	lFrom=new QLabel ( tr ( "local:" ),egb );
	lTo=new QLabel ( tr ( "remote:" ),egb );

	enclay->addWidget ( cbFsConv );
	enclay->addWidget ( lFrom );
	enclay->addWidget ( cbFrom );
	enclay->addWidget ( lTo );
	enclay->addWidget ( cbTo );
	enclay->addStretch();
	loadEnc ( cbFrom );
	loadEnc ( cbTo );

	cbFsSshTun=new QCheckBox (
	    tr ( "Use ssh port forwarding to tunnel file system "
	         "connections through firewalls" ),egb );

	QVBoxLayout* expLay=new QVBoxLayout ( this );
	expLay->addWidget ( egb );

	QHBoxLayout *tvLay=new QHBoxLayout ( egb );

	QHBoxLayout *dirLAy=new QHBoxLayout();
	dirLAy->addWidget ( dirPrompt );
	dirLAy->addWidget ( ldir );
	dirLAy->addWidget ( openDir );

	QVBoxLayout* leftLay=new QVBoxLayout();
	leftLay->addLayout ( dirLAy );
	leftLay->addSpacing ( 10 );
	leftLay->addWidget ( expTv );
	expLay->addLayout ( enclay );
	expLay->addWidget ( cbFsSshTun );

	QVBoxLayout* rightLay=new QVBoxLayout();
	rightLay->addWidget ( addDir );
	rightLay->addStretch();
	rightLay->addWidget ( delDir );
	rightLay->addStretch();


	tvLay->addLayout ( leftLay );
	tvLay->addSpacing ( 10 );
	tvLay->addLayout ( rightLay );



	expTv->setModel ( ( QAbstractItemModel* ) model );
	QFontMetrics fm1 ( expTv->font() );
	expTv->header()->resizeSection ( 1,
	                                 fm1.width ( tr ( "Automount" ) ) +10 );
	connect ( openDir,SIGNAL ( clicked() ),this,SLOT ( slot_openDir() ) );
	connect ( addDir,SIGNAL ( clicked() ),this,SLOT ( slot_addDir() ) );
	connect ( delDir,SIGNAL ( clicked() ),this,SLOT ( slot_delDir() ) );
	connect ( cbFsConv,SIGNAL ( clicked() ),this
	          ,SLOT ( slot_convClicked() ) );
	readConfig();
}


ShareWidget::~ShareWidget()
{
}

void ShareWidget::slot_openDir()
{
	QString path= QFileDialog::getExistingDirectory (
	                  this,
	                  tr ( "Select folder" ),
	                  QDir::homePath() );
	if ( path!=QString::null )
	{
		ldir->setText ( path );
	}
}


void ShareWidget::slot_addDir()
{
	QString path=ldir->text();
	if ( path.length() <1 )
		return;
	for ( int i=0;i<model->rowCount();++i )
		if ( model->index ( i,0 ).data().toString() ==path )
			return;
	QStandardItem *item;
	item= new QStandardItem ( path );
	model->setItem ( model->rowCount(),0,item );
	item= new QStandardItem();
	item->setCheckable ( true );
	model->setItem ( model->rowCount()-1,1,item );
	ldir->setText ( QString::null );
}


void ShareWidget::slot_delDir()
{
	model->removeRow ( expTv->currentIndex().row() );
}


void ShareWidget::readConfig()
{

#ifndef Q_OS_WIN
	QSettings st ( QDir::homePath() +"/.x2goclient/sessions",
	               QSettings::NativeFormat );
#else
	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "sessions" );
#endif

	QString exportDir=st.value ( sessionId+"/export",
	                             ( QVariant ) QString::null ).toString();

	cbFsSshTun->setChecked ( st.value ( sessionId+"/fstunnel",
	                                    true ).toBool() );
	QStringList lst=exportDir.split ( ";",QString::SkipEmptyParts );

	QString toCode=st.value ( sessionId+"/iconvto",
	                          ( QVariant ) "UTF-8" ).toString();

#ifdef Q_OS_WIN
	QString fromCode=st.value ( sessionId+"/iconvfrom",
	                            ( QVariant ) tr (
	                                "WINDOWS-1252" ) ).toString();
#endif
#ifdef Q_OS_DARWIN
	QString fromCode=st.value ( sessionId+"/iconvfrom",
	                            ( QVariant )
	                            "UTF-8" ).toString();
#endif
#ifdef Q_OS_LINUX
	QString fromCode=st.value ( sessionId+"/iconvfrom",
	                            ( QVariant ) tr (
	                                "ISO8859-1" ) ).toString();
#endif

	cbFsConv->setChecked ( st.value ( sessionId+"/useiconv",
	                                  ( QVariant ) false ).toBool() );
	slot_convClicked();

	int ind=cbFrom->findText ( fromCode );
	if ( ind !=-1 )
		cbFrom->setCurrentIndex ( ind );

	ind=cbTo->findText ( toCode );
	if ( ind !=-1 )
		cbTo->setCurrentIndex ( ind );

	for ( int i=0;i<lst.size();++i )
	{
#ifndef Q_OS_WIN
		QStringList tails=lst[i].split ( ":",QString::SkipEmptyParts );
#else
		QStringList tails=lst[i].split ( "#",QString::SkipEmptyParts );
#endif
		QStandardItem *item;
		item= new QStandardItem ( tails[0] );
		model->setItem ( model->rowCount(),0,item );
		item= new QStandardItem();
		item->setCheckable ( true );
		if ( tails[1]=="1" )
			item->setCheckState ( Qt::Checked );
		model->setItem ( model->rowCount()-1,1,item );
	}
}

void ShareWidget::setDefaults()
{
	cbFsSshTun->setChecked ( true );

	QString toCode="UTF-8";

#ifdef Q_OS_WIN
	QString fromCode=tr ( "WINDOWS-1252" );
#endif
#ifdef Q_OS_DARWIN
	QString fromCode="UTF-8";
#endif
#ifdef Q_OS_LINUX
	QString fromCode=tr ( "ISO8859-1" );
#endif

	cbFsConv->setChecked ( false );
	slot_convClicked();

	int ind=cbFrom->findText ( fromCode );
	if ( ind !=-1 )
		cbFrom->setCurrentIndex ( ind );
	ind=cbTo->findText ( toCode );
	if ( ind !=-1 )
		cbTo->setCurrentIndex ( ind );
}


void ShareWidget::saveSettings()
{

#ifndef Q_OS_WIN
	QSettings st ( QDir::homePath() +"/.x2goclient/sessions",
	               QSettings::NativeFormat );
#else
	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "sessions" );
#endif
	st.setValue ( sessionId+"/fstunnel",
	              ( QVariant ) cbFsSshTun->isChecked() );

	QString exportDirs;
	for ( int i=0;i<model->rowCount();++i )
	{
#ifndef Q_OS_WIN
		exportDirs+=model->index ( i,0 ).data().toString() +":";
#else
		exportDirs+=model->index ( i,0 ).data().toString() +"#";
#endif

		if ( model->item ( i,1 )->checkState() ==Qt::Checked )
			exportDirs+="1;";
		else
			exportDirs+="0;";
	}
	st.setValue ( sessionId+"/export", ( QVariant ) exportDirs );
	
	
	st.setValue(sessionId+"/iconvto",cbTo->currentText());
	st.setValue(sessionId+"/iconvfrom",cbFrom->currentText());
	st.setValue(sessionId+"/useiconv",cbFsConv->isChecked());
	st.sync();
}


void ShareWidget::loadEnc ( QComboBox* cb )
{
	QFile file ( ":/txt/encodings" );
	if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
		return;

	QTextStream in ( &file );
	while ( !in.atEnd() )
	{
		QString line = in.readLine();
		line=line.replace ( "//","" );
		cb->addItem ( line );
	}
}

void ShareWidget::slot_convClicked()
{
	bool val=cbFsConv->isChecked();
	cbTo->setEnabled ( val );
	cbFrom->setEnabled ( val );
	lTo->setEnabled ( val );
	lFrom->setEnabled ( val );
}
