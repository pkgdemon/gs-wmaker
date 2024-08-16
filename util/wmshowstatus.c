#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

Pixmap root_pixmap(Display* display)
{
  Window root = DefaultRootWindow(display);
  Atom prop = 0;
  Atom prop2 = 0;
  Atom type;
  int format;
  unsigned long length, after;
  unsigned char *data;
  int mode;
  Pixmap pixmap = 0;

  if (!prop) {
    prop = XInternAtom(display, "_XROOTPMAP_ID", False);
  }
  if (!prop2) {
    prop2 = XInternAtom(display, "ESETROOT_PMAP_ID", False);
  }

  XGrabServer(display);

  XGetWindowProperty(display, root, prop, 0L, 1L, False, AnyPropertyType,
			   &type, &format, &length, &after, &data);

  if ((type == XA_PIXMAP) && (format == 32) && (length == 1)) {
    pixmap = *((Pixmap *) data);
  }

  XUngrabServer(display);

  return pixmap;
}


Window root_window(Display* display)
{
  int screen_num = DefaultScreen(display);
  Window root = DefaultRootWindow(display);

  Pixmap pixmap = root_pixmap(display);
  return root;
}

GC create_gc(Display* display, Window win, int reverse_video)
{
  GC gc;				/* handle of newly created GC.  */
  unsigned long valuemask = 0;		/* which values in 'values' to  */
					/* check when creating the GC.  */
  XGCValues values;			/* initial values for the GC.   */
  unsigned int line_width = 2;		/* line width for the GC.       */
  int line_style = LineSolid;		/* style for lines drawing and  */
  int cap_style = CapButt;		/* style of the line's edje and */
  int join_style = JoinBevel;		/*  joined lines.		*/
  int screen_num = DefaultScreen(display);

  gc = XCreateGC(display, win, valuemask, &values);
  if (gc < 0) {
	fprintf(stderr, "XCreateGC: \n");
  }

  /* allocate foreground and background colors for this GC. */
  if (reverse_video) {
    XSetForeground(display, gc, WhitePixel(display, screen_num));
    XSetBackground(display, gc, BlackPixel(display, screen_num));
  }
  else {
    XSetForeground(display, gc, BlackPixel(display, screen_num));
    XSetBackground(display, gc, WhitePixel(display, screen_num));
  }

  /* define the style of lines that will be drawn using this GC. */
  XSetLineAttributes(display, gc,
                     line_width, line_style, cap_style, join_style);

  /* define the fill style for the GC. to be 'solid filling'. */
  XSetFillStyle(display, gc, FillSolid);

  return gc;
}

void
main(int argc, char* argv[])
{
  char text_string[256];
  char bg_color[256];

  Display* display;
  int screen_num;
  Window win;
  unsigned int display_width,
               display_height;
  unsigned int win_width,
	       win_height;
  char *display_name = getenv("DISPLAY");
  GC gc;

  XFontStruct* font_info;
//  char* font_name = "*-helvetica-*-12-*"; /* font to use for drawing text.   */
  char *font_name = "fixed";

  display = XOpenDisplay(display_name);
  if (display == NULL) {
    fprintf(stderr, "%s: cannot connect to X server '%s'\n",
            argv[0], display_name);
    exit(1);
  }

  /* get the geometry of the default screen for our display. */
  screen_num = DefaultScreen(display);
  display_width = DisplayWidth(display, screen_num);
  display_height = DisplayHeight(display, screen_num);

  win = root_window(display);

  if (argc > 2) {
    strcpy(bg_color, argv[2]);
    XColor color, colorx;
    XAllocNamedColor(display,
                     DefaultColormapOfScreen(DefaultScreenOfDisplay(display)),
                     bg_color,
                     &color, &colorx);

    XSetWindowBackground(display, win, color.pixel);
    XClearWindow(display, win);
  }
  if (argc >= 2) {
    gc = create_gc(display, win, 1);
    XSync(display, False);

    /* try to load the given font. */
    font_info = XLoadQueryFont(display, font_name);
    if (!font_info) {
        fprintf(stderr, "XLoadQueryFont: failed loading font '%s'\n", font_name);
        exit(-1);
    }

    /* assign the given font to our GC. */
    XSetFont(display, gc, font_info->fid);

    /* variables used for drawing the text strings. */
    int x, y;
    int string_width;
    int font_height;

    /* find the height of the characters drawn using this font.        */
    font_height = font_info->ascent + font_info->descent;


    x = 10;
    y = font_height;

    strcpy(text_string, argv[1]);
    XClearWindow(display, win);
    XDrawString(display, win, gc, x, y, text_string, strlen(text_string));
  }
  else {
    XClearWindow(display, win);
  }

  /* flush all pending requests to the X server. */
  XFlush(display);

  /* close the connection to the X server. */
  XCloseDisplay(display);

  exit(0);
}
