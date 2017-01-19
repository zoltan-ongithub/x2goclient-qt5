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

#ifndef FOLDERBUTTON_H
#define FOLDERBUTTON_H

#include "SVGFrame.h"
#include <QPushButton>
#include <QLabel>
class ONMainWindow;
class QComboBox;
class QPushButton;

/**
	@author Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>
*/
class FolderButton : public SVGFrame
{
    Q_OBJECT
public:
    FolderButton ( ONMainWindow* mw, QWidget* parent, QString folderPath, QString folderName );
    ~FolderButton();

    const QPixmap* folderIcon() {
        return icon->pixmap();
    }
    static bool lessThen ( const FolderButton* b1, const FolderButton* b2 );
    QString getName()
    {
        return name;
    }
    QString getPath()
    {
        return path;
    }
    void setPath(QString path)
    {
        this->path=path;
    }
    void setName(QString name)
    {
        this->name=name;
    }
    void loadIcon();

    void setChildrenList(QStringList children);
private:
    QString path;
    QString name;
    QString description;
    QLabel* nameLabel;
    QLabel* icon;
    ONMainWindow* par;

private slots:
    void slotClicked();
signals:
    void folderSelected ( FolderButton* );
    void clicked();
protected:
    virtual void mousePressEvent ( QMouseEvent * event );
    virtual void mouseReleaseEvent ( QMouseEvent * event );
};

#endif
