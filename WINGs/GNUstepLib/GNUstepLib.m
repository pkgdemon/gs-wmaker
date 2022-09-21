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
#import <GNUstepGUI/GSTheme.h>

int GSLaunchApp(const char *pathname, char *const argv[]) {
  CREATE_AUTORELEASE_POOL(pool);
  NSWorkspace* ws = [NSWorkspace sharedWorkspace];
  //[ws openFile:path withApplication:app]
  RELEASE(pool);
  return -1;
}

const char* GSCacheAppIcon(const char* cache_path, const char* path, const char *wm_instance, const char *wm_class) {
  CREATE_AUTORELEASE_POOL(pool);

  const char* val = NULL;
  char* rv = NULL;

  NSString* ap = [NSString stringWithUTF8String:path];
  if (![ap hasSuffix:@".app"]) ap = [ap stringByDeletingLastPathComponent]; //try to remove executable
  if (![ap hasSuffix:@".app"]) return NULL;

  NSWorkspace* ws = [NSWorkspace sharedWorkspace];
  NSImage* img = [ws iconForFile:ap];
  if (!img) return NULL;

  NSString* ip = [[NSString stringWithUTF8String:cache_path]
                 stringByAppendingPathComponent:[NSString stringWithFormat:@"%s.%s.tiff", wm_instance, wm_class]];
  [[img TIFFRepresentation] writeToFile:ip atomically:NO];

  NSLog(@"3 %@", ip);

  val = [ip UTF8String];

  if (val) {
    int sz = strlen(val)+1;
    rv = malloc(sz);
    strncpy(rv, val, sz);
  }

  RELEASE(pool);
  NSLog(@"RET %s", rv);

  return rv;
}

GSAppInfo GSGetDroppedAppInfo() {
  GSAppInfo i = {NULL, NULL};
  CREATE_AUTORELEASE_POOL(pool);

  const char* val = NULL;
  char* rv = NULL;
  NSPasteboard* pb = [NSPasteboard pasteboardWithName:NSDragPboard];
  NSArray *ls = [pb propertyListForType:NSFilenamesPboardType];
  if ([ls count] == 0) return i;

  NSString* path = [ls firstObject];
  if (![path hasSuffix:@".app"]) return i;

  NSString* name = [[path lastPathComponent] stringByDeletingPathExtension];
  path = [path stringByAppendingFormat:@"/%@", name];

  val = [path UTF8String];
  if (val) {
    int sz = strlen(val)+1;
    rv = malloc(sz);
    strncpy(rv, val, sz);
    i.cmd = rv;
  }

  val = [name UTF8String];
  if (val) {
    int sz = strlen(val)+1;
    rv = malloc(sz);
    strncpy(rv, val, sz);
    i.name = rv;
  }

  RELEASE(pool);

  return i;
}


const char* GSGetDroppedFilePath() {
  CREATE_AUTORELEASE_POOL(pool);

  const char* val = NULL;
  char* rv = NULL;
  NSPasteboard* pb = [NSPasteboard pasteboardWithName:NSDragPboard];
  NSArray *ls = [pb propertyListForType:NSFilenamesPboardType];
  if ([ls count] == 0) return NULL;

  NSString* path = [ls firstObject];
  val = [path UTF8String];

  if (val) {
    int sz = strlen(val)+1;
    rv = malloc(sz);
    strncpy(rv, val, sz);
  }

  RELEASE(pool);

  return rv;
}

const char* GSGetFontForName(char* name) {
  CREATE_AUTORELEASE_POOL(pool);

  const char* val = NULL;
  char* rv = NULL;
  NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
  NSMutableDictionary* domain = [[defaults persistentDomainForName:NSGlobalDomain] mutableCopy];

  if (strcmp(name, "SYSTEM_FONT") == 0) {
    val = [[domain valueForKey:@"NSFont"] UTF8String];
  }
  else if (strcmp(name, "SYSTEM_BOLDFONT") == 0) {
    val = [[domain valueForKey:@"NSBoldFont"] UTF8String];
  }

  if (val) {
    int sz = strlen(val)+1;
    rv = malloc(sz);
    strncpy(rv, val, sz);
  }

  RELEASE(pool);

  return val;
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
  CREATE_AUTORELEASE_POOL(pool);
  int sz = 0;

  NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
  NSMutableDictionary* domain = [[defaults persistentDomainForName:NSGlobalDomain] mutableCopy];

  sz = [[domain valueForKey:@"NSFontSize"] integerValue];

  RELEASE(pool);
  return sz;
}

GSColorInfo GSGetColorForName(char* name) {
  GSTheme* theme = [GSTheme theme];
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
