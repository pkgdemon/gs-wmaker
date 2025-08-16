# WindowMaker (GNUstep Desktop Fork)

This is fork of the legendary [WindowMaker](https://github.com/window-maker/wmaker) specifically created to provide window manager for [GNUstep Desktop](https://github.com/onflapp/gs-desktop). 

It contains many enhancements and tweaks to create consistent user experience for GNUstep applications. Although it could be possible to run it on its own, that's not its purpose.  

## Instructions for FreeBSD

```
. /usr/local/GNUstep/System/Library/Makefiles.sh
DBINC="`pkg-config --cflags dbus-1`"
export CPPFLAGS="$DBINC -I/usr/local/include -I/usr/include"
export LDFLAGS="-L/usr/local/lib -I/usr/lib -linotify"
./autogen.sh
./configure --prefix=/usr/local --disable-randr --disable-dbus
gmake
sudo -E gmake install
```

## Building Window Maker

This fork is inteded to be built and installed as part of [GNUstep Desktop](https://github.com/onflapp/gs-desktop).

## Notable changes:

#### fix GNUstep menus

Only one menu or popup is visible at a time.

#### fix "Hide" application command for GNUstep applications

The Hide command did not work consistently across all applications because WindowMaker got confused about what windows belong to which application. 
New implementation uses `wm_class` and `wm_instance` to gather all relevant windows for an app.

#### launch apps on startup

It uses the same approach as the NextSpace, it appends `-autolaunch YES` to the command line.

#### align the UI look and feel to look the same as a GNUstep app

Use color and font configuration from the GNUstep libraries. Note that WindowMaker links directly to the GNUstep libraries now so having GNUstep installed is hard requirement.

#### launch GNUstep apps with a file dropped on top of its dock icon

The drag & drop works as expected. For example, you can drag a file from GWorkspace onto another GNUstep app (e.g. TextEditor.app) which sits in your dock. This did not work before because the WindowMaker did not known about NSPasteboard, now it does.

#### add apps, files and folders to your dock using drag & drop 

It is now possible to drag an app from GWorkspace and add it to your dock without need to launch it first. The same will works with any file or folder.  

#### D-BUS support (experimental!)

You can control the window manager externally using [D-BUS API](https://github.com/onflapp/gs-wmaker/wiki/DBUS%E2%80%90support) now. This functionality is very basic at the moment but is foundation for making WindowMaker fully scriptable using StepTalk and for closer integration with GNUstep in general.

## UI enhancements:

- middle-click active window will lower the window
- right-click middle of the title bar will show window switcher for that app (useful alternative to tabs)
- shortcuts can now be set using a key combination, no need to go through the window menu
