Notable changes:

### fix GNUstep menus

Making sure only one menu or popup is visible at a time.

### fix "Hide" application command for GNUstep applications

The Hide command did not work consistently across all applications because WMaker got confused about what windows belong to which application. 
Use wm_class and wm_instance to gather all relevant windows for an app.

### launch on startup

Use the same approach as the NextSpace is using by appending `-autolaunch YES` to the command line'

### align the UI look and feel with GNUstep settings

Use color and font configuration from the GNUstep libraries

### launch GNUstep apps with a file dropped on top of its dock icon

When you drop a file from one GNUstep app (e.g. GWorkspace) onto another GNUstep app (e.g. TextEditor.app) which is sitting in your dock, launch it with the file. This did not work before because the WindowMaker didn't known about NSPasteboard, now it does.


