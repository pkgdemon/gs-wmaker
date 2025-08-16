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

// Automatic GNUstep runtime initialization
static void __attribute__((constructor)) init_gnustep_runtime(void) {
    static int initialized = 0;
    if (initialized) return;
    
    extern char **environ;
    
    // Initialize GNUstep runtime with dummy argc/argv
    // This is sufficient for the runtime to work properly
    char *dummy_argv[] = {"wmaker", NULL};
    GSInitializeProcess(1, dummy_argv, environ);
    
    // Create a persistent autorelease pool
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    // Don't release - let it persist for the lifetime of the program
    
    initialized = 1;
}

//static __GNUstepLib_exec_queue = NULL;

int GSLaunchApp(const char *xcmd) {
  //if (__GNUstepLib_exec_queue == NULL) __GNUstepLib_exec_queue = dispatch_queue_create("GNUstepLib_exec", DISPATCH_QUEUE_SERIAL);
  //dispatch_queue_t xq = __GNUstepLib_exec_queue;

  CREATE_AUTORELEASE_POOL(pool);
  
  BOOL autolaunch = NO;
  NSString* cmd = [NSString stringWithUTF8String:xcmd];
  NSString* ap = nil;
  NSString* file = nil;

  NSRange i = [cmd rangeOfString:@" -autolaunch "];
  if (i.location != NSNotFound) {
    ap = [cmd substringToIndex:i.location];
    autolaunch = YES;
    RELEASE(pool);
    return -1; // we want wmaker to handle the autostarts for now
  }

  i = [cmd rangeOfString:@" -GSFilePath "];
  if (i.location != NSNotFound) {
    ap = [cmd substringToIndex:i.location];
    file = [cmd substringFromIndex:i.location+13];
  }
  else {
    ap = cmd;
  }

  NSWorkspace* ws = [NSWorkspace sharedWorkspace];

  if (![ap hasSuffix:@".app"]) ap = [ws fullPathForApplication:ap];
  if (![ap hasSuffix:@".app"]) ap = [ap stringByDeletingLastPathComponent]; //try to remove executable
  if (![ap hasSuffix:@".app"]) {
    RELEASE(pool);
    return -1;
  }

  NSString* aname = [ap lastPathComponent];
  NSString* ap2 = [ws fullPathForApplication:aname];

  if ([aname isEqualToString:@"GWorkspace.app"] || [aname isEqualToString:@"GWorkspace"]) {
    if (autolaunch) {
      NSLog(@"GWorkspace");
      RELEASE(pool);
      return -1; // launch it by regular exec
    }
  }

  if ([ap isEqualToString:ap2]) {
    if (file) {
      NSWorkspace* xws = [NSWorkspace sharedWorkspace];
      [xws openFile:file withApplication:aname];
      RELEASE(pool);
      return 1;
    }
    else if (autolaunch) {
      NSWorkspace* xws = [NSWorkspace sharedWorkspace];
      [xws launchApplication:aname showIcon:NO autolaunch:YES];
      RELEASE(pool);
      return 1;
    }
    else {
      NSWorkspace* xws = [NSWorkspace sharedWorkspace];
      [xws launchApplication:aname];
      RELEASE(pool);
      return 1;
    }
  }
  else {
    NSLog(@"%@ doesn't match %@, prevent from launching using GWorkspace", ap, ap2);
  }

  RELEASE(pool);
  return -1;
}

int GSOpenDocument(const char *cmd) {
  CREATE_AUTORELEASE_POOL(pool);
  
  int rv = -1;
  BOOL isdir = NO;
  NSString* path = [NSString stringWithUTF8String:cmd];
  NSString* ext = [path pathExtension];
  NSFileManager* fm = [NSFileManager defaultManager] ;
 
  NSWorkspace* ws = [NSWorkspace sharedWorkspace];
  if ([fm fileExistsAtPath:path isDirectory:&isdir]) {
    if (isdir && [ws isFilePackageAtPath:path] == NO) {
      [ws selectFile:@"." inFileViewerRootedAtPath:path];
      rv = 1;
    }
    else if ([ext isEqualToString: @"app"]
              || [ext isEqualToString: @"debug"]
              || [ext isEqualToString: @"profile"]) {
      [ws launchApplication:path];
      rv = 2;
    }
    else {
      [ws openFile:path];
      rv = 3;
    }
  }
  RELEASE(pool);
  return rv;
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
  if (!img) {
    RELEASE(pool);
    return NULL;
  }

  NSString* ip = [[NSString stringWithUTF8String:cache_path]
                 stringByAppendingPathComponent:[NSString stringWithFormat:@"%s.%s.tiff", wm_instance, wm_class]];

  [[img TIFFRepresentation] writeToFile:ip atomically:NO];

  val = [ip UTF8String];

  if (val) {
    int sz = strlen(val)+1;
    rv = malloc(sz);
    strncpy(rv, val, sz);
  }

  RELEASE(pool);
  return rv;
}

const char* GSCachePathIcon(const char* cache_path, const char* path, const char *wm_instance, const char *wm_class) {
  CREATE_AUTORELEASE_POOL(pool);

  const char* val = NULL;
  char* rv = NULL;

  NSString* p = [NSString stringWithUTF8String:path];

  NSWorkspace* ws = [NSWorkspace sharedWorkspace];
  NSImage* img = [ws iconForFile:p];

NSLog(@"ICON for %@ -> %@", p, img);

  if (!img) {
    RELEASE(pool);
    return NULL;
  }

  NSString* ip = [[NSString stringWithUTF8String:cache_path]
                 stringByAppendingPathComponent:[NSString stringWithFormat:@"%s.%s.tiff", wm_instance, wm_class]];

  [[img TIFFRepresentation] writeToFile:ip atomically:NO];

  val = [ip UTF8String];

  if (val) {
    int sz = strlen(val)+1;
    rv = malloc(sz);
    strncpy(rv, val, sz);
  }

  RELEASE(pool);
  return rv;
}


GSDropInfo GSGetDropInfo() {
  GSDropInfo i = {NULL, NULL, 0};
  CREATE_AUTORELEASE_POOL(pool);

  const char* val = NULL;
  char* rv = NULL;
  NSPasteboard* pb = [NSPasteboard pasteboardWithName:NSDragPboard];
  NSArray *ls = [pb propertyListForType:NSFilenamesPboardType];
  if ([ls count] == 0) {
    RELEASE(pool);
    return i;
  }

  NSString* path = [ls firstObject];
  NSString* name = [path lastPathComponent];

  if ([path hasSuffix:@".app"]) {
    name = [[path lastPathComponent] stringByDeletingPathExtension];
    path = [path stringByAppendingFormat:@"/%@", name];
    i.is_app = 1;
  }

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
  if ([ls count] == 0) {
    RELEASE(pool);
    return NULL;
  }

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
  int sz = 0;
  NSString* str = nil;
  NSMutableString* buff = [NSMutableString string];
  NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
  NSMutableDictionary* domain = [[defaults persistentDomainForName:NSGlobalDomain] mutableCopy];

  sz = [[domain valueForKey:@"NSFontSize"] integerValue];

  if (strcmp(name, "SYSTEM_FONT") == 0) {
    str = [domain valueForKey:@"NSFont"];
  }
  else if (strcmp(name, "SYSTEM_BOLDFONT") == 0) {
    str = [domain valueForKey:@"NSBoldFont"];
  }

  if (str) {
    NSInteger i = [str rangeOfString:@"-"].location;
    
    if (i != NSNotFound) [buff appendString:[str substringToIndex:i]];
    else [buff appendString:str];

    if ([str hasSuffix:@"Bold"]) [buff appendString:@":weight=200"];
    else [buff appendString:@":weight=80"];

    if (sz > 0) [buff appendFormat:@":pixelsize=%d", sz];
    //:antialias=False

    val = [buff UTF8String];
    int sz = strlen(val)+1;
    rv = malloc(sz);
    strncpy(rv, val, sz);
  }

  RELEASE(pool);
  return rv;
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
  CREATE_AUTORELEASE_POOL(pool);
  GSTheme* theme = [GSTheme theme];
  GSColorInfo i = {0, 0, 0, 0};
  CGFloat r, g, b, a;

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
