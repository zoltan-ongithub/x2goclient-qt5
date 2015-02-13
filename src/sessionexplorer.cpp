/**************************************************************************
*   Copyright (C) 2005-2015 by Oleksandr Shneyder                         *
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
#include <QBuffer>
#include <QDir>

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
    for ( int i=0; i<folders.size(); ++i )
        folders[i]->close();
    folders.clear();
}

void SessionExplorer::exportsEdit ( SessionButton* bt )
{
    EditConnectionDialog dlg (false, bt->id(),parent,3 );
    if ( dlg.exec() ==QDialog::Accepted )
    {
        bt->redraw();
        bool vis=bt->isVisible();
        placeButtons();
        parent->getUsersArea()->ensureVisible ( bt->x(),bt->y(),50,220 );
        bt->setVisible ( vis );
    }
}

void SessionExplorer::getFoldersFromConfig()
{
    X2goSettings *st;

    if (parent->getBrokerMode())
        st=new X2goSettings(parent->getConfig()->iniFile,QSettings::IniFormat);
    else
        st= new X2goSettings( "sessions" );

    QStringList folders=st->setting()->childKeys();
    QString folder;
    foreach(folder,folders)
    {
        if(folder.indexOf("icon_")==0)
        {
            folder=folder.mid(strlen("icon_"));
            folder.replace("::","/");
            if(findFolder(folder)==-1)
                createFolder(folder);
        }
    }
}


void SessionExplorer::slotEdit ( SessionButton* bt )
{
    EditConnectionDialog dlg (false, bt->id(),parent );
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
    getFoldersFromConfig();

    setNavigationVisible(currentPath.length()>0);
    resize();
    int currentVerticalPosition=0;
    qSort ( sessions.begin(),sessions.end(),SessionButton::lessThen );
    qSort ( folders.begin(), folders.end(), FolderButton::lessThen );

    for ( int i=0; i<folders.size(); ++i )
    {
        if(folders[i]->getPath() != currentPath)
        {
            folders[i]->hide();
            continue;
        }

        if ( parent->getMiniMode() )
        {
            folders[i]->move ( ( parent->getUsersArea()->width()-260 ) /2,
                               currentVerticalPosition+5 );
            currentVerticalPosition+=170;
        }
        else
        {
            folders[i]->move ( ( parent->getUsersArea()->width()-360 ) /2,
                               currentVerticalPosition+5 );
            currentVerticalPosition+=230;
        }

        folders[i]->show();
        folders[i]->setChildrenList(getFolderChildren(folders[i]));
    }

    for ( int i=0; i<sessions.size(); ++i )
    {
        if(sessions[i]->getPath() != currentPath)
        {
            sessions[i]->hide();
            continue;
        }

        int horizontalPosition=(parent->getMiniMode())?(parent->getUsersArea()->width()-260 ) /2:(parent->getUsersArea()->width()-360 ) /2;

        sessions[i]->move ( horizontalPosition,
                            currentVerticalPosition+5 );

        if(parent->getBrokerMode())
        {
            currentVerticalPosition+=150;
        }
        else
        {
            if ( parent->getMiniMode() )
            {
                currentVerticalPosition+=170;
            }
            else
            {
                currentVerticalPosition+=230;
            }
        }
        sessions[i]->show();
    }

    if ( currentVerticalPosition )
    {
        parent->getUsersFrame()->setFixedHeight (
            currentVerticalPosition);
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

void SessionExplorer::setFolderIcon(QString path, QString icon)
{
    QPixmap pix(icon);
    if(!pix.isNull())
    {
        pix=pix.scaled(64,64,Qt::KeepAspectRatio, Qt::SmoothTransformation);
        path=path.split("/",QString::SkipEmptyParts).join("::");

        X2goSettings *st;
        if (parent->getBrokerMode())
            st=new X2goSettings(parent->getConfig()->iniFile,QSettings::IniFormat);
        else
            st= new X2goSettings( "sessions" );
        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        pix.save(&buffer,"PNG");
        x2goDebug<<"Save: "<<path;
        st->setting()->setValue("icon_"+path, QString(bytes.toBase64()));
        st->setting()->sync();
        FolderButton* b;
        foreach(b, folders)
        {
            if((b->getPath()+"/"+b->getName()).split("/",QString::SkipEmptyParts).join("::")==path)
            {
                b->loadIcon();
                break;
            }
        }
    }
}

void SessionExplorer::createNewFolder(QString path)
{
    X2goSettings *st;
    if (parent->getBrokerMode())
        st=new X2goSettings(parent->getConfig()->iniFile,QSettings::IniFormat);
    else
        st= new X2goSettings( "sessions" );


    if(findFolder(path)==-1)
    {
        QString name=path;
        name.replace("/","::");
        st->setting()->setValue(name, QByteArray());
        st->setting()->sync();
        createFolder(path);
        placeButtons();
    }
}

void SessionExplorer::renameFolder(QString oldPath, QString currentPath)
{
    FolderButton* b;
    oldPath=oldPath.split("/",QString::SkipEmptyParts).join("/");
    currentPath=currentPath.split("/",QString::SkipEmptyParts).join("/");

    QStringList parts=oldPath.split("/",QString::SkipEmptyParts);
    QString oldName=parts.last();
    parts.pop_back();
    QString pathOfFolder=parts.join("/");


    foreach(b, folders)
    {
        if(b->getPath()==pathOfFolder && b->getName()==oldName)
        {
            b->setName(currentPath.split("/",QString::SkipEmptyParts).last());
        }
        if((b->getPath()+"/").indexOf(oldPath+"/")==0)
        {
            QString newPath=currentPath+b->getPath().mid(oldPath.length());
            b->setPath(newPath);
        }
    }

    SessionButton* s;
    foreach(s, sessions)
    {
        if((s->getPath()+"/").indexOf(oldPath+"/")==0)
        {
            QString newPath=currentPath+s->getPath().mid(oldPath.length());
            s->setPath(newPath);
        }
    }
    if((this->currentPath+"/").indexOf(oldPath+"/")==0)
    {
        this->currentPath=currentPath+this->currentPath.mid(oldPath.length());
    }

    X2goSettings *st;
    if (parent->getBrokerMode())
        st=new X2goSettings(parent->getConfig()->iniFile,QSettings::IniFormat);
    else
        st= new X2goSettings( "sessions" );

    QStringList folders=st->setting()->childKeys();
    QString folder;
    foreach(folder,folders)
    {
        QString fname=folder;
        folder.replace("::","/");
        if((folder+"/").indexOf(oldPath+"/")==0)
        {
            QVariant value=st->setting()->value(fname);
            folder=currentPath+folder.mid(oldPath.length());
            folder.replace("/","::");
            st->setting()->setValue(folder,value);
            st->setting()->remove(fname);
        }
    }

    QStringList sessions=st->setting()->childGroups();
    QString session;
    foreach(session, sessions)
    {
        QString sname=st->setting()->value(session+"/name").toString();
        if((sname+"/").indexOf(oldPath+"/")==0)
        {
            sname=currentPath+sname.mid(oldPath.length());
            st->setting()->setValue(session+"/name",sname);
        }
    }
    st->setting()->sync();
    placeButtons();
}

bool SessionExplorer::isFolderEmpty(QString path)
{
    FolderButton* b;
    path=path.split("/",QString::SkipEmptyParts).join("/");

    foreach(b, folders)
    {
        if(b->getPath()==path)
        {
            return false;
        }
    }

    SessionButton* s;
    foreach(s, sessions)
    {
        if(s->getPath()==path)
        {
            return false;
        }
    }
    return true;
}

void SessionExplorer::deleteFolder(QString path)
{
    path=path.split("/",QString::SkipEmptyParts).join("::");
    X2goSettings *st;
    if (parent->getBrokerMode())
        st=new X2goSettings(parent->getConfig()->iniFile,QSettings::IniFormat);
    else
        st= new X2goSettings( "sessions" );

    st->setting()->remove(path);

    path.replace("::","/");


    for(int i=0; i< folders.count(); ++i)
    {
        FolderButton* b=folders[i];
        if((b->getPath()+"/"+b->getName()).split("/",QString::SkipEmptyParts).join("/")==path)
        {
            b->close();
            folders.removeAt(i);
            break;
        }
    }
    if(currentPath==path)
    {
        currentPath="";
    }
    placeButtons();
}

void SessionExplorer::setEnable(bool enable)
{
    backButton->setEnabled(enable);
}
