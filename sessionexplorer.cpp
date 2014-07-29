/**************************************************************************
*   Copyright (C) 2005-2014 by Oleksandr Shneyder                         *
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

#include "sessionexplorer.h"
#include "sessionbutton.h"
#include "folderbutton.h"
#include "editconnectiondialog.h"
#include "onmainwindow.h"
#include <QMessageBox>
#include <QCheckBox>
#include "x2gosettings.h"
#include <QGridLayout>
#include <QScrollArea>
#include <QFile>
#include <QDesktopServices>
#include "x2goutils.h"
#include "imgframe.h"
#include <QToolButton>
#include "x2gologdebug.h"

SessionExplorer::SessionExplorer(ONMainWindow* p):QObject(p)
{
    parent=p;
    lastSession=0;
    backButton=new QToolButton(parent->getCentralFrame());
    backButton->setIcon(QIcon( parent->iconsPath ( "/32x32/tbhide.png" )));
    backButton->setAutoRaise(true);
    pathLabel=new QLabel(" ",parent->getCentralFrame());
    backButton->setFixedWidth(36);
    navigationLayout=new QHBoxLayout();
    navigationLayout->addWidget(backButton);
    navigationLayout->addWidget(pathLabel);
    backButton->setToolTip(tr("Back"));
    QPalette pal=backButton->palette();
    pal.setBrush ( QPalette::Window, QColor ( 110,112,127,255 ) );
    pal.setBrush ( QPalette::Base, QColor ( 110,112,127,255 ) );
    pal.setBrush ( QPalette::Button, QColor ( 110,112,127,255 ) );

    backButton->setPalette(pal);
    backButton->setAutoFillBackground(true);
    pal=pathLabel->palette();
    pal.setBrush ( QPalette::Window, QColor ( 110,112,127,255 ) );
    pal.setBrush ( QPalette::WindowText, QColor ( 200,200,200,255 ) );
    pathLabel->setPalette(pal);
    pathLabel->setAutoFillBackground(true);
    setNavigationVisible(false);
    connect(backButton,SIGNAL(clicked(bool)), this, SLOT(slotLevelUp()));
}

SessionExplorer::~SessionExplorer()
{
}

void SessionExplorer::resize()
{
    pathLabel->setMaximumWidth(parent->getUsersArea()->width()-backButton->width());
    QFontMetrics metrics(pathLabel->font());
    QString displayText = metrics.elidedText(" "+currentPath, Qt::ElideLeft, pathLabel->width()-6);
    pathLabel->setText(displayText);
}

void SessionExplorer::cleanSessions()
{
    for ( int i=0; i<sessions.size(); ++i )
        sessions[i]->close();
    sessions.clear();
}

void SessionExplorer::exportsEdit ( SessionButton* bt )
{
    EditConnectionDialog dlg ( bt->id(),parent,3 );
    if ( dlg.exec() ==QDialog::Accepted )
    {
        bt->redraw();
        bool vis=bt->isVisible();
        placeButtons();
        parent->getUsersArea()->ensureVisible ( bt->x(),bt->y(),50,220 );
        bt->setVisible ( vis );
    }
}


void SessionExplorer::slotEdit ( SessionButton* bt )
{
    EditConnectionDialog dlg ( bt->id(),parent );
    if ( dlg.exec() ==QDialog::Accepted )
    {
        bt->redraw();
        placeButtons();
        parent->getUsersArea()->ensureVisible ( bt->x(),bt->y(),50,220 );
    }
}

void SessionExplorer::slotCreateDesktopIcon ( SessionButton* bt )
{
    QMessageBox messageBox(QMessageBox::Question,
                           tr ( "Create session icon on desktop" ),
                           tr ( "Desktop icons can be configured "
                                "not to show x2goclient (hidden mode). "
                                "If you like to use this feature you'll "
                                "need to configure login by a gpg key "
                                "or gpg smart card.\n\n"
                                "Use x2goclient hidden mode?" ),
                           QMessageBox::Yes|QMessageBox::No,
                           parent);

    //adding a chekbox to know if user want to enable trayicon in hide sessions
    QCheckBox cbShowTrayIcon(tr("Show session tray icon when running"));
    messageBox.layout()->addWidget(&cbShowTrayIcon);
    QGridLayout* gridLayout = (QGridLayout*) messageBox.layout();
    gridLayout->addWidget(&cbShowTrayIcon, gridLayout->rowCount(), 0, 1, gridLayout->columnCount());
    cbShowTrayIcon.blockSignals(true);

    //getting the result
    bool crHidden = (messageBox.exec() == QMessageBox::Yes);
    bool bShowTrayicon = (cbShowTrayIcon.checkState() == Qt::Checked);


    X2goSettings st ( "sessions" );

    QString name=st.setting()->value ( bt->id() +"/name",
                                       ( QVariant ) tr ( "New Session" ) ).toString() ;

    // PyHoca-GUI uses the slash as separator for cascaded menus, so let's handle these on the file system
    name.replace("/","::");

    QString sessIcon=st.setting()->value (
                         bt->id() +"/icon",
                         ( QVariant )
                         ":icons/128x128/x2gosession.png"
                     ).toString();
    sessIcon = expandHome(sessIcon);
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

    if (bShowTrayicon)
        cmd += " --tray-icon";

    QTextStream out ( &file );
    out << "[Desktop Entry]\n"<<
        "Exec="<<cmd<<" --sessionid="<<bt->id() <<"\n"<<
        "Icon="<<sessIcon<<"\n"<<
        "Name="<<name<<"\n"<<
        "StartupNotify=true\n"<<
        "Terminal=false\n"<<
        "Type=Application\n"<<
        "X-KDE-SubstituteUID=false\n";
    file.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner);
    file.close();
#else
    QString scrname=QDir::tempPath() +"\\mklnk.vbs";
    QFile file ( scrname );
    if ( !file.open ( QIODevice::WriteOnly | QIODevice::Text ) )
        return;

    QSettings xst ( "HKEY_LOCAL_MACHINE\\SOFTWARE\\x2goclient",
                    QSettings::NativeFormat );
    QString workDir=xst.value ( "Default" ).toString();
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


SessionButton* SessionExplorer::createBut ( const QString& id )
{
    SessionButton* l;
    l=new SessionButton ( parent,parent->getUsersFrame(),id );
    sessions.append ( l );
    connect ( l,SIGNAL ( signal_edit ( SessionButton* ) ),
              this,SLOT ( slotEdit ( SessionButton* ) ) );

    connect ( l,SIGNAL ( signal_remove ( SessionButton* ) ),
              this,SLOT ( slotDeleteButton ( SessionButton* ) ) );

    connect ( l,SIGNAL ( sessionSelected ( SessionButton* ) ),parent,
              SLOT ( slotSelectedFromList ( SessionButton* ) ) );

    if(l->getPath()!="")
    {
        if(findFolder(l->getPath())==-1)
        {
            createFolder(l->getPath());
        }
    }

    return l;
}


void SessionExplorer::placeButtons()
{
    setNavigationVisible(currentPath.length()>0);
    resize();
    int currentIndex=0;
    qSort ( sessions.begin(),sessions.end(),SessionButton::lessThen );
    qSort ( folders.begin(), folders.end(), FolderButton::lessThen );

    for ( int i=0; i<folders.size(); ++i )
    {
        if(folders[i]->getPath() != currentPath)
        {
            folders[i]->hide();
            continue;
        }
        if ( !parent->getMiniMode() )
            folders[i]->move ( ( parent->getUsersArea()->width()-360 ) /2,
                               currentIndex*220+currentIndex*25+5 );
        else
            folders[i]->move ( ( parent->getUsersArea()->width()-260 ) /2,
                               currentIndex*155+currentIndex*20+5 );
        if (parent->getBrokerMode())
            folders[i]->move ( ( parent->getUsersArea()->width()-360 ) /2,
                               currentIndex*150+currentIndex*25+5 );
        folders[i]->show();
        folders[i]->setChildrenList(getFolderChildren(folders[i]));
        ++currentIndex;
    }

    for ( int i=0; i<sessions.size(); ++i )
    {
        if(sessions[i]->getPath() != currentPath)
        {
            sessions[i]->hide();
            continue;
        }
        if ( !parent->getMiniMode() )
            sessions[i]->move ( ( parent->getUsersArea()->width()-360 ) /2,
                                currentIndex*220+currentIndex*25+5 );
        else
            sessions[i]->move ( ( parent->getUsersArea()->width()-260 ) /2,
                                currentIndex*155+currentIndex*20+5 );
        if (parent->getBrokerMode())
            sessions[i]->move ( ( parent->getUsersArea()->width()-360 ) /2,
                                currentIndex*150+currentIndex*25+5 );
        sessions[i]->show();
        ++currentIndex;
    }

    if ( currentIndex )
    {
        if ( !parent->getMiniMode() )
            parent->getUsersFrame()->setFixedHeight (
                currentIndex *220+ ( currentIndex -1 ) *25 );
        else
            parent->getUsersFrame()->setFixedHeight (
                currentIndex *155+ ( currentIndex-1 ) *20 );
        if (parent->getBrokerMode())
            parent->getUsersFrame()->setFixedHeight (
                currentIndex *150+ ( currentIndex-1 ) *25 );
    }

}

QStringList SessionExplorer::getFolderChildren(FolderButton* folder)
{
    QStringList children;
    QString normPath=(folder->getPath()+"/"+folder->getName()).split("/",QString::SkipEmptyParts).join("/");

    for(int i=0; i<folders.count(); ++i)
    {
        if(folders[i]->getPath()==normPath)
            children<<folders[i]->getName();
    }
    for(int i=0; i<sessions.count(); ++i)
    {
        if(sessions[i]->getPath()==normPath)
            children<<sessions[i]->name();
    }
    return children;
}


void SessionExplorer::slotDeleteButton ( SessionButton * bt )
{
    if ( QMessageBox::warning (
                parent,bt->name(),
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
    parent->getUsersArea()->ensureVisible ( 0,0,50,220 );
}

void SessionExplorer::setNavigationVisible(bool value)
{
    backButton->setVisible(value);
    pathLabel->setVisible(value);
}

void SessionExplorer::createFolder(QString path)
{
    QStringList tails=path.split("/");
    QStringList currentPath;
    for(int i=0; i<tails.count()-1; ++i)
    {
        currentPath<<tails[i];
        if(findFolder(currentPath.join("/"))==-1)
        {
            createFolder(currentPath.join("/"));
        }
    }
    FolderButton* fb=new FolderButton(parent,parent->getUsersFrame(),currentPath.join("/"), tails.last());
    connect(fb, SIGNAL(folderSelected(FolderButton*)), this, SLOT(slotFolderSelected(FolderButton*)));
    folders<<fb;
}

int SessionExplorer::findFolder(QString path)
{
    for(int i=0; i<folders.count(); ++i)
    {
        QString normPath=(folders[i]->getPath()+"/"+folders[i]->getName()).split("/",QString::SkipEmptyParts).join("/");
        if(normPath==path)
            return i;
    }
    return -1;
}

void SessionExplorer::slotFolderSelected(FolderButton* bt)
{
    currentPath=(bt->getPath()+"/"+bt->getName()).split("/",QString::SkipEmptyParts).join("/");
    placeButtons();
}

void SessionExplorer::slotLevelUp()
{
    QStringList levels=currentPath.split("/",QString::SkipEmptyParts);
    if(levels.count())
    {
        levels.pop_back();
        currentPath=levels.join("/");
    }
    placeButtons();
}
