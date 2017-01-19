/***************************************************************************
 *  Copyright (C) 2012-2017 by Mihai Moldovan <ionic@ionic.de>             *
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

#ifndef WINDOWS_STDINT_H
#define WINDOWS_STDINT_H

#ifdef Q_OS_WIN

/* We need this ugly hack because Windows doesn't know about std::(u)int*_t types. */
namespace std {
  typedef signed char            int8_t;
  typedef signed short           int16_t;
  typedef signed int             int32_t;
  typedef signed long long int   int64_t;
  typedef unsigned char          uint8_t;
  typedef unsigned short         uint16_t;
  typedef unsigned int           uint32_t;
  typedef unsigned long long int uint64_t;
}

#endif /* defined (Q_OS_WIN) */


#endif /* !defined (WINDOWS_STDINT_H) */
