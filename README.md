Notable changes:

# fixing GNUstep menus

Making sure only one menu or popup is visible at a time.

# fixing "Hide" application command for GNUstep applications

The Hide command did not work consistently across all applications because WMaker got confused about what windows belong to which application. 
Use wm_class and wm_instance to gather all relevant windows for an app.

# launch on startup

Use the same approach as the NextSpace is using by appending `-autolaunch YES` to the command line'

# align UI look and feel with GNUstep settings

Use color and font configuration from the GNUstep libraries

