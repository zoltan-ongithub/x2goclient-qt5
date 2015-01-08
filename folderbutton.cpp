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

#include "folderbutton.h"
#include "x2goclientconfig.h"
#include "x2goutils.h"

#include <QFont>
#include <QPixmap>
#include <QLabel>
#include "x2gosettings.h"
#include <QDir>
#include <QLayout>
#include <QPushButton>
#include "onmainwindow.h"
#include "x2gologdebug.h"
#include <QApplication>
#include <QDesktopWidget>
#include "sessionexplorer.h"
#include <QMouseEvent>


FolderButton::FolderButton ( ONMainWindow* mw,QWidget *parent, QString folderPath, QString folderName )
    : SVGFrame ( ":/svg/folder.svg",false,parent )
{
    QPalette pal=palette();
    pal.setColor ( QPalette::Active, QPalette::WindowText, QPalette::Mid );
    pal.setColor ( QPalette::Active, QPalette::ButtonText, QPalette::Mid );
    pal.setColor ( QPalette::Active, QPalette::Text, QPalette::Mid );
    pal.setColor ( QPalette::Inactive, QPalette::WindowText, QPalette::Mid );
    pal.setColor ( QPalette::Inactive, QPalette::ButtonText, QPalette::Mid );
    pal.setColor ( QPalette::Inactive, QPalette::Text, QPalette::Mid );

    setPalette(pal);

    path=folderPath;
    name=folderName;

    QFont fnt=font();
    if ( mw->retMiniMode() )
#ifdef Q_WS_HILDON
        fnt.setPointSize ( 10 );
#else
        fnt.setPointSize ( 9 );
#endif
    setFont ( fnt );
    setFocusPolicy ( Qt::NoFocus );
    bool miniMode=mw->retMiniMode();
    if ( !miniMode )
        setFixedSize ( 340,190 );
    else
        setFixedSize ( 250,145 );

    par= mw;
    connect ( this,SIGNAL ( clicked() ),this,SLOT ( slotClicked() ) );


    nameLabel=new QLabel ( this );
    description=tr("Sessions folder");

    setChildrenList(QStringList() );

    icon=new QLabel ( this );

    nameLabel->setWordWrap(true);
    nameLabel->setTextInteractionFlags(Qt::NoTextInteraction);

    icon->move ( 10,25 );

    if ( !miniMode )
    {
        nameLabel->move ( 80,34 );
        nameLabel->setFixedSize(235,135);
    }
    else
    {
        nameLabel->move ( 64,18 );
        nameLabel->setFixedSize(170,120);
    }
    loadIcon();
}

FolderButton::~FolderButton()
{}

void FolderButton::loadIcon()
{
    X2goSettings *st;

    if (par->getBrokerMode())
        st=new X2goSettings(par->getConfig()->iniFile,QSettings::IniFormat);
    else
        st= new X2goSettings( "sessions" );

    QString sessIcon=":icons/128x128/folder.png";
    QPixmap* pix;

    QString normPath=(path+"/"+name).split("/",QString::SkipEmptyParts).join("::");

    QByteArray picture = QByteArray::fromBase64( st->setting()->value ( "icon_"+normPath,
                       ( QVariant )QString()).toString().toLocal8Bit());
    if(!picture.size())
    {
        pix=new QPixmap( sessIcon );
    }
    else
    {
        pix=new QPixmap();
        pix->loadFromData(picture);
    }
    bool miniMode=par->retMiniMode();

    if ( !miniMode )
    {
        icon->setPixmap ( pix->scaled ( 64,64,Qt::IgnoreAspectRatio,
                                        Qt::SmoothTransformation ) );
    }
    else
    {
        icon->setPixmap ( pix->scaled ( 48,48,Qt::IgnoreAspectRatio,
                                        Qt::SmoothTransformation ) );
    }

    delete pix;
}

void FolderButton::slotClicked()
{
    emit folderSelected ( this );
}

bool FolderButton::lessThen ( const FolderButton* b1,
                              const FolderButton* b2 )
{
    return b1->name.toLower().localeAwareCompare (
               b2->name.toLower() ) <0;
}

void FolderButton::mousePressEvent ( QMouseEvent * event )
{
    SVGFrame::mousePressEvent ( event );
    loadBg ( ":/svg/folder_grey.svg" );
}

void FolderButton::mouseReleaseEvent ( QMouseEvent * event )
{
    SVGFrame::mouseReleaseEvent ( event );
    int x=event->x();
    int y=event->y();
    loadBg ( ":/svg/folder.svg" );
    if ( x>=0 && x< width() && y>=0 && y<height() )
        emit clicked();
}

void FolderButton::setChildrenList(QStringList children)
{
    QString text="<b>"+name+"</b>";
    if(description.length()>0)
    {
        text+="<br>("+description+")";
    }
    if(children.count())
    {
        text+="<p style=\"color:grey\">"+children.join(", ")+"</p>";
    }
    nameLabel->setText(text);
}
