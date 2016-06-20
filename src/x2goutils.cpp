/**************************************************************************
*   Copyright (C) 2005-2016 by Oleksandr Shneyder                         *
*                              <o.shneyder@phoca-gmbh.de>                 *
*   Copyright (C) 2016 by Mihai Moldovan <ionic@ionic.de>                 *
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
#include <QFont>
#include <QFontInfo>
#include <QObject>
#include <QStringList>

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

  /*
   * Skip empty or non-resource URIs.
   * Assume all paths not starting with a colon are
   * absolute, non-resource URIs.
   */
  if ((!(res_path.isEmpty ())) && (res_path.startsWith (':'))) {
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

void show_RichText_Generic_MsgBox (QMessageBox::Icon icon, const QString &main_text, const QString &informative_text) {
  QString fixup_main_text (convert_to_rich_text (main_text));
  QString fixup_informative_text (convert_to_rich_text (informative_text, true));

  QMessageBox msg_box (icon, QString ("X2Go Client"), fixup_main_text, QMessageBox::Ok);

  msg_box.setTextFormat (Qt::RichText);
  msg_box.setInformativeText (fixup_informative_text);
  msg_box.setWindowModality (Qt::WindowModal);
  msg_box.exec ();
}

void show_RichText_WarningMsgBox (const QString &main_text, const QString &informative_text) {
  show_RichText_Generic_MsgBox (QMessageBox::Warning, main_text, informative_text);
}

void show_RichText_ErrorMsgBox (const QString &main_text, const QString &informative_text) {
  show_RichText_Generic_MsgBox (QMessageBox::Critical, main_text, informative_text);
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

bool font_is_monospaced (const QFont &font) {
  const QFontInfo font_info (font);
  return (font_info.fixedPitch ());
}

#ifdef Q_OS_DARWIN
void show_XQuartz_not_found_error () {
  show_XQuartz_generic_error (QObject::tr ("X2Go Client could not find any suitable X11 server."),
                              QString ());
}

void show_XQuartz_start_error () {
  show_XQuartz_generic_error (QObject::tr ("X2Go Client could not start X11 server."),
                              QObject::tr ("X2Go Client requires XQuartz to be installed.\n\n"
                                           "If XQuartz is already installed on your system,\n"
                                           "please select the correct path in the now upcoming dialog.\n"
                                           "Refer to the end of this message for path examples,\n"
                                           "in case you do not know the exact location yourself.\n\n"
                                           "Should you have <b>not</b> installed XQuartz yet, please\n"
                                           "follow the outlined steps:\n\n"));
}

void show_XQuartz_generic_error (const QString &main_error, const QString &additional_info) {
  show_RichText_WarningMsgBox (main_error,
                               additional_info +
                               QObject::tr ("MacPorts users, please install either the port <b>xorg-server</b>\n"
                                            "or the port <b>xorg-server-devel</b>.\n"
                                            "Upon successful installation, please follow the instructions printed\n"
                                            "by the port utility to autostart/load the server.\n\n"

                                            "All other users, please obtain and install XQuartz from:\n"

                                            "<center><a href=\"https://xquartz.macosforge.org/\">"
                                                "https://xquartz.macosforge.org/"
                                            "</a></center>\n\n"

                                            "Afterwards, restart X2Go Client and select the correct path\n"
                                            "to the X11 application in the general X2Go Client settings.\n"
                                            "This will most likely be\n"
                                            "<center><b>/Applications/MacPorts/X11.app</b></center>\n"
                                            "or\n"
                                            "<center><b>/Applications/Utilities/XQuartz.app</b></center>"));
}

QString add_to_path (const QString &orig_path, const QStringList &add, const bool back) {
  QString ret = orig_path;
  std::vector<bool> found;

  QStringList orig_path_list = orig_path.split (":");

  /*
   * Clean up add list. We want to make sure no entry ends in a slash
   * and skip empty entries.
   */
  QStringList tmp_clean_add;
  for (int i = 0; i < add.size (); ++i) {
    if (!(add[i].isEmpty ())) {
      if (add[i].right (1) == "/") {
        QString tmp_elem = add[i].right (1);

        if (!(tmp_elem.isEmpty ())) {
          tmp_clean_add.append (tmp_elem);
        }
      }
      else {
        tmp_clean_add.append (add[i]);
      }
    }
  }

  /* Nothing to add, really... */
  if (tmp_clean_add.isEmpty ()) {
    return (ret);
  }

  /* Create unique array. */
  QStringList clean_add;
  {
    QStringList::const_iterator begin = tmp_clean_add.constBegin (),
                                end = tmp_clean_add.constEnd ();
    for (QStringList::const_iterator cit = begin; cit != end; ++cit) {
      bool tmp_found = false;

      for (QStringList::const_iterator cit2 = cit + 1; cit2 != end; ++cit2) {
        if (*cit == *cit) {
          tmp_found = true;
          break;
        }
      }

      if (!tmp_found) {
        clean_add.append (*cit);
      }
    }
  }

  /* Nothing to add. */
  if (clean_add.isEmpty ()) {
    return (ret);
  }

  found.resize (clean_add.size (), false);

  for (int i = 0; i < orig_path_list.length (); ++i) {
    for (int y = 0; y < clean_add.size (); ++y) {
      if (!found[y]) {
        if ((orig_path_list[i] == QString (clean_add[y])) || (orig_path_list[i] == QString (clean_add[y] + '/'))) {
          found[y] = true;
          break;
        }
      }
    }
  }

  if (back) {
    for (int i = 0; i < clean_add.size (); ++i) {
      if (!found[i]) {
        ret.append (QString (":" + clean_add[i]));
      }
    }
  }
  else {
    for (int i = (clean_add.size () - 1); i > 0; --i) {
      if (!found[i]) {
        ret.prepend (QString (clean_add[i] + ":"));
      }
    }
  }

  return (ret);
}

QString find_binary (const QString &path, const QString &binary_name) {
  QString ret = "";

  if (!(binary_name.isEmpty ())) {
    QString cur_path = "";
    QString tmp_path = path;

    if (!(path.isEmpty ())) {
      tmp_path = "./";
    }

    QStringList path_list = tmp_path.split (":");

    for (QStringList const_it = path_list.constBegin (); const_it != path_list.constEnd (); ++const_it) {
      cur_path = *const_it;

      if (cur_path.isEmpty ()) {
        cur_path = "./";
      }

      cur_path = QDir (cur_path).absolutePath ();

      cur_path += "/" + binary_name;

      QFileInfo tmp_file_info = QFileInfo (cur_path);

      if ((tmp_file_info.exists ()) && (tmp_file_info.isExecutable ())) {
        ret = tmp_file_info.canonicalFilePath ();
        break;
      }
    }
  }

  return (ret);
}
#endif /* defined (Q_OS_DARWIN) */
