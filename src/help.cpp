/***************************************************************************
 *  Copyright (C) 2015 by Mihai Moldovan <ionic@ionic.de> +49 721 14595728 *
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program; if not, write to the                          *
 *  Free Software Foundation, Inc.,                                        *
 *  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.              *
 ***************************************************************************/

#include <QCoreApplication>
#include <QtDebug>
#include <cstddef>
#include "help.h"
#include "version.h"

help::prelude_t help::cleanup_prelude (help::prelude_t prelude) {
  for (help::prelude_t::iterator it = prelude.begin (); it != prelude.end (); ++it) {
    *it = (*it).trimmed ();
  }

  return (prelude);
}

help::params_t help::cleanup_params (help::params_t params) {
  for (help::params_t::const_iterator params_it = params.constBegin (); params_it != params.constEnd (); ++params_it) {
    (*params_it).first = (*params_it).first.trimmed ();
    (*params_it).second = (*params_it).second.trimmed ();
  }

  return (params);
}

help::prelude_t build_prelude () {
  help::prelude_t ret ();

  QStringList args = QCoreApplication::arguments ();

  QString ver ("X2Go Client " << VERSION);

  if (QFile::exists (":/txt/git-info")) {
    QFile file (":/txt/git-info");

    if (file.open (QIODevice::ReadOnly | QIODevice::Text)) {
      QTextStream stream (&file);

      QString git_info (stream.readAll ().trimmed ());

      if (!(git_info.isEmpty ())) {
        ver << " (Git information: " << git_info << ")";
      }
    }
  }

  ret.append (ver);
  ret.append ("Usage: " << args.at (0) << " [OPTION]...");
  ret.append ("Options:");
  ret.append ("");

  return (ret);
}

help::params_t help::build_params () {

}

help::data_t help::build_data () {
  return (help::data_t (help::cleanup_prelude (help::build_prelude ()), help::cleanup_params (help::build_params ())));
}

void help::pretty_print (help::data_t data) {
  help::data_t data = help::build_data ();

  QTextStream out << data.first.join ("\n") << "\n";

  std::size_t max_len = 0;

  /* Iterate over all parameter options and get max width. */
  for (help::params_t::const_iterator it = data.second.constBegin (); it != data.second.constEnd (); ++it) {
    max_len = std::max (max_len, (*it).first.length ());
  }

  std::size_t indent = 0;
}
