#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


static void OnExpose(XExposeEvent* evt, GC gc, XFontStruct* font)
{
    char buff[256];
    XWindowAttributes wa;
    XGetWindowAttributes(evt->display, evt->window, &wa);

    sprintf(buff, "Expose width = %d, height = %d", evt->width, evt->height);
    int length = XTextWidth(font, buff, strlen(buff));
    int msg_x = (wa.width - length) / 2;

    int font_height = font->ascent + font->descent;
    int msg_y  = (wa.height + font_height) / 2;
    XDrawString(evt->display, evt->window, gc, msg_x, msg_y, buff, strlen(buff));

    msg_y += font_height;
    XDrawLine(evt->display, evt->window, gc, msg_x - 10, msg_y, msg_x + length + 10, msg_y);

    msg_y += font_height;
    sprintf(buff, "Window width = %d, height = %d", wa.width, wa.height);
    length = XTextWidth(font, buff, strlen(buff));
    msg_x = (wa.width - length) / 2;
    XDrawString(evt->display, evt->window, gc, msg_x, msg_y, buff, strlen(buff));

    msg_y += font_height;
    XDrawLine(evt->display, evt->window, gc, msg_x - 10, msg_y, msg_x + length + 10, msg_y);
}

static void OnKeyPress(XKeyEvent* evt)
{
    printf("OnKeyPress. code=%d\n", evt->keycode);
}

static void OnButtonPress(XButtonEvent* evt)
{
    printf("OnButtonPress. x=%d, y=%d\n", evt->x, evt->y);
}

static void OnMotionNotify(XMotionEvent* evt)
{
    //printf("OnMotionNotifyEvent. x=%d, y=%d\n", evt->x, evt->y);
}

/*
   static void Resize(struct MAIN_WND* wnd, int width, int height)
   {
//XGetGeometry()
wnd->width = width;
wnd->height = height;
}*/

static void MsgLoop(Display* dpy, Window win, GC gc, XFontStruct* font)
{
    int running = 1;
    XEvent evt;
    Atom wm_quit = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSelectInput(dpy, win, ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask | PointerMotionMask);
    XSetWMProtocols(dpy, win, &wm_quit, 1);
    while(running)
    {
        XNextEvent(dpy, &evt);
        switch(evt.type)
        {
            case Expose:
                if(evt.xexpose.count)
                    break;
                OnExpose(&evt.xexpose, gc, font);
                break;
                //    case ConfigureNotify:
                //      Resize(wnd, evt.xconfigure.width, evt.xconfigure.height);
                //      break;
            case KeyPress:
                OnKeyPress(&evt.xkey);
                break;
            case ButtonPress:
                OnButtonPress(&evt.xbutton);
                break;
            case MotionNotify:
                OnMotionNotify(&evt.xmotion);
                break;
            case ClientMessage:
                if(evt.xclient.data.l[0] != wm_quit)
                    continue;
                running = 0;
                break;
        }
    }
    //Stop receiving events
    XSelectInput(dpy, win, NoEventMask);
}

static void ListFontNames(Display* dpy)
{
    char **cp, **cp0;
    int i, count;
    cp = cp0 = XListFonts (dpy, "*", 256, &count);
    printf("%d fonts found\n", count);
    for(i = 0; i < count; i ++, cp++)
        printf("%s\n", *cp);
    if(count) XFreeFontNames(cp0);
}

static Display* InitDisplay()
{
    Display* dpy = XOpenDisplay(0);
    if(!dpy)
    {
        perror("Couldn't connect to X server\n");
        return 0;
    }
    XSynchronize(dpy, 1);//this slows down but makes synch calls. REMOVE in release
    return dpy;
}

static Window InitWindow(Display* dpy, int argc, char* argv[])
{
    static char* window_name = "Test Window Name";
    static char* icon_name = "XTest";
    XTextProperty windowName, iconName;
    int sn = DefaultScreen(dpy);
    unsigned dw = DisplayWidth(dpy, sn);
    unsigned dh = DisplayHeight(dpy, sn);
    XSizeHints size_hints = {0};
    XWMHints wm_hints = {0};
    XClassHint class_hints = {0};
    Window win = XCreateSimpleWindow(dpy, RootWindow(dpy, sn)
            , 0, 0, dw / 2, dh / 2, 0
            //    , BlackPixel(dpy, screen_num)
            //    , WhitePixel(dpy, screen_num));
        , 0x000000, 0x88FF88);
    //To avoid sending Expose message the a part of window is covered
    //set the mode to backup the covered part of the window
    if(DoesBackingStore(DefaultScreenOfDisplay(dpy)) == Always)
    {
        XSetWindowAttributes attr;
        //CWBackingStore - Backup covered window image (not repaint)
        //CWBitGravity - repaint on size change
        attr.backing_store = Always;
        //only for top-right
        //    attr.bit_gravity = NorthWestGravity;
        //XChangeWindowAttributes keeps all other attr. unchanged
        XChangeWindowAttributes(dpy, win, CWBackingStore /*| CWBitGravity*/, &attr);
    }
    if(XStringListToTextProperty(&window_name, 1, &windowName) == 0)
    {
        perror("Structure allocation for windowName failed");
        return 0;
    }
    if(XStringListToTextProperty(&icon_name, 1, &iconName) == 0)
    {
        perror("Structure allocation for iconName failed");
        return 0;
    }
    size_hints.flags = PMinSize;
    size_hints.min_width = 200;
    size_hints.min_height = 100;

    wm_hints.flags = StateHint | InputHint;
    wm_hints.initial_state = NormalState;
    wm_hints.input = True;

    class_hints.res_name = argv[0];
    class_hints.res_class = "xtest";

    XSetWMProperties(dpy, win, &windowName, &iconName, argv, argc, &size_hints, &wm_hints, &class_hints);

    XFree(windowName.value);
    XFree(iconName.value);
    return win;
}

int main(int argc, char * argv[])
{
    GC gc;
    XFontStruct* font;
    XGCValues values;
    Display* dpy = InitDisplay();
    if(!dpy)
        return EXIT_FAILURE;

    Window win = InitWindow(dpy, argc, argv);
    if(!win)
        return EXIT_FAILURE;


    //  ListFontNames(dpy);
    if(!(font = XLoadQueryFont(dpy, "-misc-fixed-bold-r-normal--18-120-100-100-c-90-iso10646-1")))
    {
        fprintf(stderr, "%s: cannot open 9x15 font.\n", argv[0]);
        return EXIT_FAILURE;
    }
    gc = XCreateGC(dpy, win, 0, &values);

    XSetFont(dpy, gc, font->fid);
    XSetForeground(dpy, gc, BlackPixel(dpy, DefaultScreen(dpy)));
    XMapWindow(dpy, win);
    //XFlush(dpy);//force message sending
    MsgLoop(dpy, win, gc, font);
    XFreeFont(dpy, font);
    XFreeGC(dpy, gc);
    XCloseDisplay(dpy);
    return EXIT_SUCCESS;
}
