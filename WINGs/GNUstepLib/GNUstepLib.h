/*
   Project: GNUstepLib

   Copyright (C) 2022 Free Software Foundation

   Author: ,,,

   Created: 2022-04-29 16:56:37 +0200 by pi

   This application is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This application is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111 USA.
*/

#ifndef _GNUSTEPLIB_H_
#define _GNUSTEPLIB_H_

typedef struct {
  unsigned int red;
  unsigned int green;
  unsigned blue;
  int valid;
} GSColorInfo;

extern int GSGetAntialiasText(void);
extern int GSGetFontDefaultFontSize(void);
extern int GSGetAntialiasText(void);
extern const char* GSGetDroppedFilePath(void);
extern const char* GSGetFontForName(char* name);
extern GSColorInfo GSGetColorForName(char* name);

#endif // _GNUSTEPLIB_H_

