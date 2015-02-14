/**************************************************************************
*   Copyright (C) 2005-2015 by Oleksandr Shneyder                         *
*                              <o.shneyder@phoca-gmbh.de>                 *
*   Copyright (C) 2015 by Mihai Moldovan <ionic@ionic.de>                 *
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

#include <vector>
#include <algorithm>
#include <QString>
#include <QDir>

QString expandHome( QString path )
{
    path = path.trimmed();
    if ( path.startsWith("~/") || path.startsWith("~\\") ) {
        path = path.replace(QString("~"), QDir::homePath());
    }
    return path;
}

QString wrap_legacy_resources (const QString res_path) {
  QString ret (res_path);

  if (!(res_path.isEmpty ())) {
    std::vector<QString> legacy_locations;
    legacy_locations.push_back (QString (":/icons/"));
    legacy_locations.push_back (QString (":/png/"));
    legacy_locations.push_back (QString (":/svg/"));

    bool detected = false;

    /* This would be so much easier with C++ and lambdas... */
    QString::const_iterator it = legacy_locations.begin ();
    while (it != legacy_locations.end ()) {
      if (res_path.startsWith (*(it++))) {
        detected = true;
        break;
      }
    }

    if (detected)
      ret.insert (1, QString ("/img"));
  }

  return (ret);
}
