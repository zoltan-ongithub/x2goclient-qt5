/**************************************************************************
*   Copyright (C) 2005-2017 by Oleksandr Shneyder                         *
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

#include "InteractionDialog.h"
#include "x2goclientconfig.h"
#include "onmainwindow.h"
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QScrollBar>
#include <QTimer>

#ifndef Q_OS_LINUX
#if QT_VERSION < 0x050000
#include <QPlastiqueStyle>
#endif
#endif

InteractionDialog::InteractionDialog(QWidget* parent): SVGFrame(":/img/svg/passform.svg",
            false,parent )
{
    mw=(ONMainWindow*)parent;

    if ( !mw->retMiniMode() )
        setFixedSize ( this->sizeHint().width(),this->sizeHint().height()*1.5 );
    else
        setFixedSize ( 310,280 );


    QPalette pal=this->palette();
    pal.setBrush ( QPalette::Window, QColor ( 255,255,255,0 ) );
    pal.setColor ( QPalette::Active, QPalette::WindowText, QPalette::Mid );
    pal.setColor ( QPalette::Active, QPalette::ButtonText, QPalette::Mid );
    pal.setColor ( QPalette::Active, QPalette::Text, QPalette::Mid );
    pal.setColor ( QPalette::Inactive, QPalette::WindowText, QPalette::Mid );
    pal.setColor ( QPalette::Inactive, QPalette::ButtonText, QPalette::Mid );
    pal.setColor ( QPalette::Inactive, QPalette::Text, QPalette::Mid );

    this->setPalette ( pal );

    pal.setColor ( QPalette::Button, QColor ( 255,255,255,0 ) );
    pal.setColor ( QPalette::Window, QColor ( 255,255,255,255 ) );
    pal.setColor ( QPalette::Base, QColor ( 255,255,255,255 ) );

    QFont fnt=this->font();
    if ( mw->retMiniMode() )
#ifdef Q_WS_HILDON
        fnt.setPointSize ( 10 );
#else
        fnt.setPointSize ( 9 );
#endif
    this->setFont ( fnt );
    this->hide();

    textEdit=new QTextEdit(this);
    QVBoxLayout* lay=new QVBoxLayout(this);
    lay->addWidget(new QLabel(tr("Terminal output:")));
    lay->addWidget(textEdit);

    textEntry=new QLineEdit(this);
    textEntry->setEchoMode(QLineEdit::Password);
    lay->addWidget(textEntry);

    cancelButton=new QPushButton(tr("Cancel"),this);
    lay->addWidget(cancelButton);
    textEdit->setReadOnly(true);
    connect(textEntry,SIGNAL(returnPressed()),this,SLOT(slotTextEntered()));
    connect(cancelButton, SIGNAL(clicked(bool)),this,SLOT(slotButtonPressed()));
    textEdit->setFrameStyle ( QFrame::StyledPanel|QFrame::Plain );
    cancelButton->setFlat(true);

#ifndef Q_OS_LINUX
    QStyle* widgetExtraStyle;
#if QT_VERSION < 0x050000
    widgetExtraStyle = new QPlastiqueStyle ();
#else
    widgetExtraStyle = QStyleFactory::create ("fusion");
#endif

    this->setStyle(widgetExtraStyle);
    textEntry->setStyle(widgetExtraStyle);
    textEdit->setStyle(widgetExtraStyle);
    textEdit->viewport()->setStyle(widgetExtraStyle);
    cancelButton->setStyle(widgetExtraStyle);

#endif
}

InteractionDialog::~InteractionDialog()
{
//     qDebug()<<"Iter dlg destruct\n";
}

void InteractionDialog::appendText(QString txt)
{
    textEntry->setEnabled(true);
    textEdit->append(txt);
    textEntry->setFocus();
    interrupted=false;
    display=false;
    cancelButton->setText(tr("Cancel"));
    QTimer::singleShot(0, textEntry, SLOT(setFocus()));
}

void InteractionDialog::reset()
{
    textEdit->clear();
}

void InteractionDialog::slotTextEntered()
{
    QString text=textEntry->text()+"\n";
    textEntry->clear();
    emit textEntered(text);
}

void InteractionDialog::slotButtonPressed()
{
    if(!display)
    {
        emit interrupt();
        interrupted=true;
    }
    else
    {
        qDebug()<<"reconnect";
        emit closeInterractionDialog();
    }
}

void InteractionDialog::setDisplayMode()
{
    cancelButton->setText(tr("Reconnect"));
    textEntry->setEnabled(false);
    display=true;
}

void InteractionDialog::setInteractionMode(IMode value)
{
    interactionMode=value;
}

