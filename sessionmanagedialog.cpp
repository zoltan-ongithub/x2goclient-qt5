//
// C++ Implementation: sessionmanagedialog
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
#include "sessionmanagedialog.h"
#include "onmainwindow.h"

#include <QPushButton>
#include <QSettings>
#include <QDir>
#include <QFrame>
#include <QBoxLayout>
#include <QListView>
#include <QStringListModel>
#include <QShortcut>
#include "sessionbutton.h"



SessionManageDialog::SessionManageDialog(QWidget * parent, Qt::WFlags f)
        : QDialog(parent, f)
{
    QVBoxLayout* ml=new QVBoxLayout(this);
    QFrame *fr=new QFrame(this);
    QHBoxLayout* frLay=new QHBoxLayout(fr);

    QPushButton* ok=new QPushButton(tr("E&xit"),this);
    QHBoxLayout* bLay=new QHBoxLayout();

    sessions=new QListView(fr);
    frLay->addWidget(sessions);

    QPushButton* newSession=new QPushButton(tr("&New Session"),fr);
    editSession=new QPushButton(tr("&Session Preferences"),fr);
    removeSession=new QPushButton(tr("&Delete Session"),fr);

    par=(ONMainWindow*)parent;
    newSession->setIcon(QIcon(par->iconsPath("/16x16/new_file.png")));
    editSession->setIcon(QIcon(par->iconsPath("/16x16/edit.png")));
    removeSession->setIcon(QIcon(par->iconsPath("/16x16/delete.png")));

    QVBoxLayout* actLay=new QVBoxLayout();
    actLay->addWidget(newSession);
    actLay->addWidget(editSession);
    actLay->addWidget(removeSession);
    actLay->addStretch();
    frLay->addLayout(actLay);
    
    QShortcut* sc=new QShortcut(QKeySequence(tr("Delete","Delete")),this);
    connect(ok,SIGNAL(clicked()),this,SLOT(close()));
    connect(sc,SIGNAL(activated()),removeSession,SIGNAL(clicked()));
    connect(removeSession,SIGNAL(clicked()),this,SLOT(slot_delete()));
    connect(editSession,SIGNAL(clicked()),this,SLOT(slot_edit()));
    connect(newSession,SIGNAL(clicked()),this,SLOT(slotNew()));
    bLay->setSpacing(5);
    bLay->addStretch();
    bLay->addWidget(ok);
    ml->addWidget(fr);
    ml->addLayout(bLay);

    fr->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    fr->setLineWidth(2);

    setSizeGripEnabled ( true );
    setWindowIcon(QIcon(((ONMainWindow*)parent)->iconsPath("/32x32/edit.png")));
    setWindowTitle(tr("Session Management"));
    loadSessions();
    connect(sessions,SIGNAL(clicked(const QModelIndex&)),
            this,SLOT(slot_activated(const QModelIndex&)));
    connect(sessions,SIGNAL(doubleClicked(const QModelIndex&)),
            this,SLOT(slot_dclicked(const QModelIndex&)));
}


SessionManageDialog::~SessionManageDialog()
{}


void SessionManageDialog::loadSessions()
{
    QStringListModel *model=(QStringListModel*)sessions->model();
    if(!model)
        model=new QStringListModel();
    sessions->setModel(model);
    QStringList lst;
    model->setStringList(lst);

    const QList<SessionButton*> *sess=par->getSessionsList();

    for(int i=0;i<sess->size();++i)
        lst<<sess->at(i)->name();

    model->setStringList(lst);
    removeSession->setEnabled(false);
    editSession->setEnabled(false);
    sessions->setEditTriggers(QAbstractItemView::NoEditTriggers);
}


void SessionManageDialog::slot_activated(const QModelIndex&)
{
    removeSession->setEnabled(true);
    editSession->setEnabled(true);
}

void SessionManageDialog::slot_dclicked(const QModelIndex& )
{
    slot_edit();
}


void SessionManageDialog::slotNew()
{
    par->slotNewSession();
    loadSessions();
}


void SessionManageDialog::slot_edit()
{
    int ind=sessions->currentIndex().row();
    if(ind<0)
        return;
    par->slot_edit(par->getSessionsList()->at(ind));
    loadSessions();
}


void SessionManageDialog::slot_delete()
{
    int ind=sessions->currentIndex().row();
    if(ind<0)
        return;
    par->slotDeleteButton(par->getSessionsList()->at(ind));
    loadSessions();
}
