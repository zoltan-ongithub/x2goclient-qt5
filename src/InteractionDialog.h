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

#ifndef INTERACTIONDIALOG_H
#define INTERACTIONDIALOG_H
#include "x2goclientconfig.h"


#include "SVGFrame.h"

class ONMainWindow;
class QTextEdit;
class QLineEdit;
class QPushButton;
class InteractionDialog: public SVGFrame
{

    Q_OBJECT
public:
    InteractionDialog ( QWidget* parent=0);
    virtual ~InteractionDialog();
    void reset();
    void appendText(QString txt);
    bool isInterrupted() {
        return interrupted;
    }
    void setDisplayMode();
private:
    ONMainWindow* mw;
    QTextEdit* textEdit;
    QPushButton* cancelButton;
    QLineEdit* textEntry;
    bool interrupted;
    bool display;
private slots:
    void slotTextEntered();
    void slotButtonPressed();
signals:
    void textEntered(QString text);
    void interrupt();
    void closeInterractionDialog();
};

#endif

