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

#import "GNUstepLib.h"
#import	<AppKit/AppKit.h>

const char* GSGetFontForName(char* name) {
  return NULL;
  /*
  CREATE_AUTORELEASE_POOL(pool);

  [NSApplication sharedApplication];
  NSFont* font = nil;
  if (strcmp(name, "SYSTEM_FONT") == 0) {
    font = [NSFont systemFontOfSize:[NSFont systemFontSize]];
  }
  else if (strcmp(name, "SYSTEM_BOLDFONT") == 0) {
    font = [NSFont systemFontOfSize:[NSFont systemFontSize]];
  }

  RELEASE(pool);

  if (font) {
    return [[font fontName] UTF8String];
  }
  else {
    return NULL;
  }
  */
}

int GSGetAntialiasText() {
  CREATE_AUTORELEASE_POOL(pool);
  int v = -1;

  NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
  NSMutableDictionary* domain = [[defaults persistentDomainForName:NSGlobalDomain] mutableCopy];

  if ([domain valueForKey:@"GSFontAntiAlias"]) {
    v = (BOOL)[[domain valueForKey:@"GSFontAntiAlias"] boolValue];
  }

  RELEASE(pool);

  return v;
}

int GSGetFontDefaultFontSize() {
  CGFloat sz = 0;
  /*
  CREATE_AUTORELEASE_POOL(pool);
  [NSApplication sharedApplication];

  sz = [NSFont systemFontSize];

  RELEASE(pool);
  */
  return (int)sz;
}

GSColorInfo GSGetColorForName(char* name) {
  GSColorInfo i = {0, 0, 0, 0};
  CGFloat r, g, b, a;

  CREATE_AUTORELEASE_POOL(pool);

  NSColor* color = nil;
  if (strcmp(name, "SYSTEM_GRAY_COLOR") == 0) {
    color = [NSColor controlBackgroundColor];
  }
  else if (strcmp(name, "SYSTEM_DARKGRAY_COLOR") == 0) {
    color = [NSColor controlShadowColor];
  }
  else if (strcmp(name, "SYSTEM_SCROLLER_COLOR") == 0) {
    color = [NSColor scrollBarColor];
  }
  
  if (color) {
    [[color colorUsingColorSpaceName:NSCalibratedRGBColorSpace] getRed:&r green:&g blue:&b alpha:&a];
    i.red = (unsigned int)round(r*255);
    i.green = (unsigned int)round(g*255);
    i.blue = (unsigned int)round(b*255);
    i.valid = 1;
  }

  RELEASE(pool);

  return i;
}
