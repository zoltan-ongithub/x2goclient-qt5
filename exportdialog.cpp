//
// C++ Implementation: exportdialog
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@treuchtlingen.de>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "x2goclientconfig.h"
#include "exportdialog.h"
#include "editconnectiondialog.h"
#include <QBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QSettings>
#include <QListView>
#include <QDir>
#include <QStringListModel>
#include <QShortcut>
#include "sessionbutton.h"
#include "onmainwindow.h"
#include <QFileDialog>

ExportDialog::ExportDialog(QString sid,QWidget * par, Qt::WFlags f)
        : QDialog(par,f)
{
    sessionId=sid;
    QVBoxLayout* ml=new QVBoxLayout(this);
    QFrame *fr=new QFrame(this);
    QHBoxLayout* frLay=new QHBoxLayout(fr);

    parent=(ONMainWindow*)par;

    QPushButton* cancel=new QPushButton(tr("&Cancel"),this);
    QHBoxLayout* bLay=new QHBoxLayout();

    sessions=new QListView(fr);
    frLay->addWidget(sessions);

    exportDir=new QPushButton(tr("&share"),fr);
    editSession=new QPushButton(tr("&Preferences ..."),fr);
    newDir=new QPushButton(tr("&Custom Folder ..."),fr);


    QVBoxLayout* actLay=new QVBoxLayout();
    actLay->addWidget(exportDir);
    actLay->addWidget(editSession);
    actLay->addWidget(newDir);
    actLay->addStretch();
    frLay->addLayout(actLay);

    QShortcut* sc=new QShortcut(QKeySequence(tr("Delete","Delete")),this);
    connect(cancel,SIGNAL(clicked()),this,SLOT(close()));
    connect(sc,SIGNAL(activated()),exportDir,SIGNAL(clicked()));
    connect(editSession,SIGNAL(clicked()),this,SLOT(slot_edit()));
    connect(newDir,SIGNAL(clicked()),this,SLOT(slotNew()));
    connect(exportDir,SIGNAL(clicked()),this,SLOT(slot_accept()));
    bLay->setSpacing(5);
    bLay->addStretch();
    bLay->addWidget(cancel);
    ml->addWidget(fr);
    ml->addLayout(bLay);

    fr->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    fr->setLineWidth(2);

    setSizeGripEnabled ( true );
    setWindowTitle(tr("share Folders"));
    connect(sessions,SIGNAL(clicked(const QModelIndex&)),
            this,SLOT(slot_activated(const QModelIndex&)));
    connect(sessions,SIGNAL(doubleClicked(const QModelIndex&)),
            this,SLOT(slot_dclicked(const QModelIndex&)));
    loadSessions();
}


ExportDialog::~ExportDialog()
{}

void ExportDialog::loadSessions()
{
    QStringListModel *model=(QStringListModel*)sessions->model();
    if(!model)
        model=new QStringListModel();
    sessions->setModel(model);

    QStringList dirs;
    model->setStringList(dirs);

#ifndef WINDOWS
    QSettings st(QDir::homePath()+"/.x2goclient/sessions",QSettings::NativeFormat);
#else
    QSettings st("Obviously Nice","x2goclient");
    st.beginGroup("sessions");
#endif


    QString exports=st.value(sessionId+"/export",(QVariant)QString::null).toString();

    QStringList lst=exports.split(";",QString::SkipEmptyParts);
    for(int i=0;i<lst.size();++i)
    {
#ifndef WINDOWS    
        QStringList tails=lst[i].split(":",QString::SkipEmptyParts);
#else
        QStringList tails=lst[i].split("#",QString::SkipEmptyParts);
#endif
        dirs<<tails[0];
    }


    model->setStringList(dirs);


    //     removeSession->setEnabled(false);
    exportDir->setEnabled(false);
    sessions->setEditTriggers(QAbstractItemView::NoEditTriggers);
}


void ExportDialog::slot_activated(const QModelIndex&)
{
    //     removeSession->setEnabled(true);
    exportDir->setEnabled(true);
}

void ExportDialog::slot_dclicked(const QModelIndex& )
{
    slot_accept();
}


void ExportDialog::slotNew()
{
    directory=QString::null;
    directory= QFileDialog::getExistingDirectory(
                   this,
                   tr("Select Folder"),
                   QDir::homePath());

    if(directory!=QString::null)
        accept();

}


void ExportDialog::slot_edit()
{
    const QList<SessionButton*>* sess=parent->getSessionsList();
    for(int i=0;i< sess->size();++i)
    {
        if(sess->at(i)->id()==sessionId)
        {
            parent->exportsEdit(sess->at(i));
            break;
        }
    }
    loadSessions();
}



void ExportDialog::slot_accept()
{
    int ind=sessions->currentIndex().row();
    if(ind<0)
        return;
    QStringListModel *model=(QStringListModel*)sessions->model();
    directory=model->stringList()[ind];
    accept();
}
