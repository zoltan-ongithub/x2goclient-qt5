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

#ifndef HELP_H
#define HELP_H

#include <QPair>
#include <QStringList>
#include <vector>
#include <algorithm>

namespace help {
  typedef QStringList prelude_t;
  typedef QPair<QString, QString> params_elem_t;
  typedef QList<params_elem_t> params_t;
  typedef QPair<prelude_t, params_t> data_t;

  /* Builds a prelude_t object. Values are hardcoded here. */
  prelude_t build_prelude ();

  /* Builds a params_t object. Values are hardcoded here. */
  params_t build_params ();

  /* Merges prelude_t and params_t into a data_t object. */
  data_t build_data ();

  /* Cleanup functions for string trimming. */
  prelude_t cleanup_prelude (prelude_t prelude);
  params_t cleanup_params (params_t params);

  /* Prints a help_data_t structure. */
  QTextStream pretty_print (data_t data);
}

#endif /* !defined (HELP_H) */
