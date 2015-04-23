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
#include <QMessageBox>

#include "x2goutils.h"
#include "onmainwindow.h"
#include "x2gologdebug.h"

QString expandHome (QString path) {
  path = path.trimmed ();
  if (path.startsWith ("~/") || path.startsWith ("~\\")) {
      path = path.replace (QString ("~"), QDir::homePath ());
  }
  return path;
}

QString fixup_resource_URIs (const QString &res_path) {
  QString ret (res_path);

  if (!(res_path.isEmpty ())) {
    if (ret.at (1) != '/') {
      ret.insert (1, '/');
    }
  }

  return (ret);
}

QString wrap_legacy_resource_URIs (const QString &res_path) {
  QString ret (res_path);

  if (!(res_path.isEmpty ())) {
    std::vector<QString> legacy_locations;
    legacy_locations.push_back (QString (":/icons/"));
    legacy_locations.push_back (QString (":/png/"));
    legacy_locations.push_back (QString (":/svg/"));

    ret = fixup_resource_URIs (ret);

    bool detected = false;

    /* This would be so much easier with C++ and lambdas... */
    std::vector<QString>::const_iterator it = legacy_locations.begin ();
    while (it != legacy_locations.end ()) {
      if (ret.startsWith (*(it++))) {
        detected = true;
        break;
      }
    }

    if (detected)
      ret.insert (1, QString ("/img"));
  }

  return (ret);
}

QString convert_to_rich_text (const QString &text, bool force) {
  QString fixup_text (text);
  fixup_text.replace ("\n", "\n<br />\n");

  if (force) {
    // This is a workaround for a bug in Qt. Even though we set Qt::RichText as the text format
    // later on, the informative text is not recognized as rich text, UNLESS a HTML tag
    // is used ON THE VERY FIRST LINE.
    // Make sure, that there always is one...
    fixup_text.prepend ("<b></b>");
  }

  return (fixup_text);
}

void show_RichText_WarningMsgBox (const QString &main_text, const QString &informative_text) {
  QString fixup_main_text (convert_to_rich_text (main_text));
  QString fixup_informative_text (convert_to_rich_text (informative_text, true));

  QMessageBox msg_box (QMessageBox::Warning, QString ("X2Go Client"),
                       fixup_main_text, NULL);

  msg_box.setTextFormat (Qt::RichText);
  msg_box.setInformativeText (fixup_informative_text);
  msg_box.setWindowModality (Qt::WindowModal);
  msg_box.exec ();
}

QString git_changelog_extract_commit_sha (const QString &gitlog) {
  QString ret = "";

  /*
   * Do a poor man's split.
   * We know that a newline character should be somewhere at the beginning of the string.
   * We don't need to have Qt split the string up completely as we only care about
   * a substring: from start to the first newline character.
   */
  std::ptrdiff_t pos = gitlog.indexOf ("\n");

  if (0 < pos) {
    ret = gitlog.left (pos + 1);

    x2goDebug << "First line of git changelog: " << ret;

    pos = ret.lastIndexOf (")");

    if (0 < pos) {
      std::ptrdiff_t pos_paren_start = ret.lastIndexOf ("(");

      if ((0 < pos_paren_start) && (pos_paren_start < pos)) {
        ret = ret.mid (pos_paren_start + 1, pos - pos_paren_start - 1);
      }
      else {
        // Either starting parenthesis not found or starting parenthesis comes first.
        ret = "";
      }
    }
    else {
      // End parenthesis not found.
      ret = "";
    }
  }

  return (ret);
}
