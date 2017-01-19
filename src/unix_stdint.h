/***************************************************************************
 *  Copyright (C) 2016-2017 by Mihai Moldovan <ionic@ionic.de>             *
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

#ifndef UNIX_STDINT_H
#define UNIX_STDINT_H

#ifdef Q_OS_UNIX

#include <stdint.h>

/*
 * We need this ugly hack because the cstdint header is C++11-only
 * (or available with extensions we can't portably use)
 * and stdint.h won't put these types into the std namespace.
 */
namespace std {
  using ::int8_t;
  using ::int16_t;
  using ::int32_t;
  using ::int64_t;
  using ::uint8_t;
  using ::uint16_t;
  using ::uint32_t;
  using ::uint64_t;
}

/* Same issue with unistd's ssize_t type. */
namespace std {
  using ::ssize_t;
}

#endif /* defined (Q_OS_UNIX) */


#endif /* !defined (UNIX_STDINT_H) */
