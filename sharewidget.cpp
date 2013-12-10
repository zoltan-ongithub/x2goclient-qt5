/**************************************************************************
*   Copyright (C) 2005-2012 by Oleksandr Shneyder                         *
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
#include <QMessageBox>

#include "x2gologdebug.h"

#include <QFileDialog>
#include <QDir>
#include "x2gosettings.h"

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
    QString startDir=ONMainWindow::getHomeDirectory();
#ifdef Q_OS_WIN
    if ( ONMainWindow::getPortable() &&
            ONMainWindow::U3DevicePath().length() >0 )
    {
        startDir=ONMainWindow::U3DevicePath() +"/";
    }
#endif

    QString path= QFileDialog::getExistingDirectory (
                      this,
                      tr ( "Select folder" ),
                      startDir );
    if ( path!=QString::null )
    {
#ifdef Q_OS_WIN
        if ( ONMainWindow::getPortable() &&
                ONMainWindow::U3DevicePath().length() >0 )
        {
            if ( path.indexOf ( ONMainWindow::U3DevicePath() ) !=0 )
            {
                QMessageBox::critical (
                    0l,tr ( "Error" ),
                    tr ( "x2goclient is running in "
                         "portable mode. You should "
                         "use a path on your usb device "
                         "to be able to access your data "
                         "whereever you are" ),
                    QMessageBox::Ok,QMessageBox::NoButton );
                slot_openDir();
                return;
            }
            path.replace ( ONMainWindow::U3DevicePath(),
                           "(U3)" );
        }
#endif
        ldir->setText ( path );
    }
}


void ShareWidget::slot_addDir()
{
    QString path=ldir->text();
    if ( path.length() <1 )
        return;
    for ( int i=0; i<model->rowCount(); ++i )
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

    X2goSettings st ( "sessions" );

    QString exportDir=st.setting()->value ( sessionId+"/export",
                                            ( QVariant ) QString::null ).toString();

    cbFsSshTun->setChecked ( st.setting()->value ( sessionId+"/fstunnel",
                             true ).toBool() );
    QStringList lst=exportDir.split ( ";",QString::SkipEmptyParts );

    QString toCode=st.setting()->value ( sessionId+"/iconvto",
                                         ( QVariant ) "UTF-8" ).toString();

#ifdef Q_OS_WIN
    QString fromCode=st.setting()->value ( sessionId+"/iconvfrom",
                                           ( QVariant ) tr (
                                                   "WINDOWS-1252" ) ).toString();
#endif
#ifdef Q_OS_DARWIN
    QString fromCode=st.setting()->value ( sessionId+"/iconvfrom",
                                           ( QVariant )
                                           "UTF-8" ).toString();
#endif
#ifdef Q_OS_LINUX
    QString fromCode=st.setting()->value ( sessionId+"/iconvfrom",
                                           ( QVariant ) tr (
                                                   "ISO8859-1" ) ).toString();
#endif

    cbFsConv->setChecked ( st.setting()->value ( sessionId+"/useiconv",
                           ( QVariant ) false ).toBool() );
    slot_convClicked();

    int ind=cbFrom->findText ( fromCode );
    if ( ind !=-1 )
        cbFrom->setCurrentIndex ( ind );

    ind=cbTo->findText ( toCode );
    if ( ind !=-1 )
        cbTo->setCurrentIndex ( ind );

    for ( int i=0; i<lst.size(); ++i )
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

    X2goSettings st ( "sessions" );
    st.setting()->setValue ( sessionId+"/fstunnel",
                             ( QVariant ) cbFsSshTun->isChecked() );

    QString exportDirs;
    for ( int i=0; i<model->rowCount(); ++i )
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
    st.setting()->setValue ( sessionId+"/export", ( QVariant ) exportDirs );


    st.setting()->setValue ( sessionId+"/iconvto",cbTo->currentText() );
    st.setting()->setValue ( sessionId+"/iconvfrom",cbFrom->currentText() );
    st.setting()->setValue ( sessionId+"/useiconv",cbFsConv->isChecked() );
    st.setting()->sync();
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
