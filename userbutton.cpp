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

#include "x2goclientconfig.h"
#include "userbutton.h"
#include <QFont>
#include <QPixmap>
#include <QLabel>
#include "onmainwindow.h"
UserButton::UserButton ( ONMainWindow* wnd, QWidget *par, QString name,
                         QString fullName, QPixmap& foto, QPalette& bgpal,
                         int width,int height )
        : QPushButton ( par )
{
    user=name;
    fname=fullName;
    image=foto;
    setFocusPolicy ( Qt::NoFocus );
    setAutoFillBackground ( true );
    setFlat ( true );
    bgpal.setColor ( QPalette::Active, QPalette::WindowText, QPalette::Mid );
    bgpal.setColor ( QPalette::Active, QPalette::ButtonText, QPalette::Mid );
    bgpal.setColor ( QPalette::Inactive, QPalette::WindowText, QPalette::Mid );
    bgpal.setColor ( QPalette::Inactive, QPalette::ButtonText, QPalette::Mid );

    setPalette ( bgpal );

    bool miniMode=wnd->retMiniMode();
    if ( width==0 || height==0 )
    {
        if ( !miniMode )
        {
            setFixedSize ( 340,100 );
        }
        else
            setFixedSize ( 250,100 );
    }
    else
    {
        setFixedSize ( width,height );
    }
    QLabel* f=new QLabel ( this );
    QString text=name+"\n("+fullName+")";
    QLabel* n=new QLabel ( text,this );
    if ( !miniMode )
        n->move ( 110,25 );
    else
        n->move ( 90,25 );
    f->setPixmap ( foto );
    f->setMaximumSize ( 80,80 );
    if ( !miniMode )
        f->move ( 10,10 );
    else
        f->move ( 5,10 );
    connect ( this,SIGNAL ( clicked() ),this,SLOT ( slotClicked() ) );
}

UserButton::~UserButton()
{}

void UserButton::slotClicked()
{
    emit userSelected ( this );
}
