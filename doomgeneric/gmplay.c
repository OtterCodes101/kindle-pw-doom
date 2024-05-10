// The following code is based on geekmaster's kindle video player,
// https://www.mobileread.com/forums/showthread.php?t=177455
// the original copyright header can be seen below:

//====================================================
// gmplay 1.5a - geekmaster's kindle video player
// Copyright (C) 2012 by geekmaster, with MIT license:
// http://www.opensource.org/licenses/mit-license.php
//----------------------------------------------------
// Tested on DX,DXG,K3,K4main,K4diags,K5main,K5diags.
//---------------------------------------------------- 

#include "gmplay.h"

//----- gmlib global vars -----
u8 *fb0 = NULL; // framebuffer pointer
int fdFB = 0;   // fb0 file descriptor
int teu = 0;    // eink update time
u32 fs = 0;     // fb0 stride
u32 MX = 0;     // xres (visible)
u32 MY = 0;     // yres (visible)
u32 VY = 0;     // (VY>MY): mxcfb driver
u8 ppb = 0;     // pixels per byte
u32 fc = 0;     // frame counter

#ifdef X_PC
static Display *s_Display = NULL;
static Window s_Window = 0;
static GC s_Gc = 0;
static XImage *s_Image = NULL;

static unsigned char *colorFb = NULL;
#endif

// The following function is based on geekmaster's kindle video transcoder,
// https://www.mobileread.com/forums/showthread.php?p=2074379#post2074379
// the original copyright header can be seen below:

//====================================================
// raw2gmv 1.0a - raw to geekmaster video transcoder
// Copyright (C) 2012 by geekmaster, with MIT license:
// http://www.opensource.org/licenses/mit-license.php
//----------------------------------------------------
static void preprocessFrame(unsigned char * frame, unsigned char * outFrame) {
  unsigned char o, to, tb;
  unsigned int x, y, xi, yi, c = 250, b = 120; // c=contrast, b=brightness
  for (y = 0; y < KINDLE_RESY; y++) {
      xi = y;
      tb = 0;
      for (x = 0; x < KINDLE_RESX; x++) {
        yi = 599 - x;
        o = x ^ y;
        to = (y >> 2 & 1 | o >> 1 & 2 | y << 1 & 4 | o << 2 & 8 | y << 4 & 16 |
              o << 5 & 32) -
                 (frame[yi * KINDLE_RESY + xi] * 63 + b) / c >>
             8;
        tb = (tb >> 1) | (to & 128);
        if (7 == (x & 7)) {
          *outFrame = tb;
          outFrame++;
          tb = 0;
        }
      }
    }
}

#ifdef X_PC
static void showFrame() {
  int x = 0;
  int y = 0;
  unsigned char *currentColorPixel = colorFb;
  unsigned char *currentMonochromePixel = fb0;
  for (y = 0; y < KINDLE_FB_RES_Y; y++) {
    for (x = 0; x < KINDLE_FB_RES_X; x++) {
      currentColorPixel[0] = *currentMonochromePixel;
      currentColorPixel[1] = *currentMonochromePixel;
      currentColorPixel[2] = *currentMonochromePixel;
      currentMonochromePixel++;
      currentColorPixel += 4;
    }
  }

  if (s_Display) {
    while (XPending(s_Display) > 0) {
      XEvent e;
      XNextEvent(s_Display, &e);
    }

    XPutImage(s_Display, s_Window, s_Gc, s_Image, 0, 0, 0, 0, KINDLE_FB_RES_X, KINDLE_FB_RES_Y);
  }
}

static void initWindow() {
  s_Display = XOpenDisplay(NULL);

  int s_Screen = DefaultScreen(s_Display);

  int blackColor = BlackPixel(s_Display, s_Screen);
  int whiteColor = WhitePixel(s_Display, s_Screen);

  XSetWindowAttributes attr;
  memset(&attr, 0, sizeof(XSetWindowAttributes));
  attr.event_mask = ExposureMask | KeyPressMask;
  attr.background_pixel = BlackPixel(s_Display, s_Screen);

  int depth = DefaultDepth(s_Display, s_Screen);

  s_Window = XCreateSimpleWindow(s_Display, DefaultRootWindow(s_Display), 0, 0,
                                 KINDLE_FB_RES_X, KINDLE_FB_RES_Y, 0, blackColor, blackColor);

  const char windowTitle[] = "Kindle Framebuffer Preview";
  XChangeProperty(s_Display, s_Window, XA_WM_NAME, XA_STRING, 8,
                  PropModeReplace, windowTitle, strlen(windowTitle));

  XSelectInput(s_Display, s_Window,
               StructureNotifyMask | KeyPressMask | KeyReleaseMask);

  XMapWindow(s_Display, s_Window);

  s_Gc = XCreateGC(s_Display, s_Window, 0, NULL);

  XSetForeground(s_Display, s_Gc, whiteColor);

  while (1) {
    XEvent e;
    XNextEvent(s_Display, &e);
    if (e.type == MapNotify) {
      break;
    }
  }

  Visual *visual = DefaultVisual(s_Display, s_Screen);

  s_Image = XCreateImage(s_Display, visual, depth, ZPixmap, 0, colorFb, KINDLE_FB_RES_X,
                         KINDLE_FB_RES_Y, 32, 0);
}
#endif

//==================================
// gmplay8 - play video on 8-bit fb0
//----------------------------------
void gmplay8(void) {
  u32 i, x, y, b, fbsize = FBSIZE;
  u8 fbt[FBSIZE];
  while (fread(fbt, fbsize, 1, stdin)) {
    teu += 130; // teu: next update time
    if (getmsec() > teu + 1000) {
      continue;         // drop frame if > 1 sec behind
    }
    gmlib(GMLIB_VSYNC); // wait for fb0 ready
    for (y = 0; y < 1448; y++)
      for (x = 0; x < 1072; x += 8) {
        b = fbt[1072 / 8 * y + x / 8];
        i = y * fs + x;
        fb0[i] = (b & 1) * 255;
        b >>= 1;
        fb0[i + 1] = (b & 1) * 255;
        b >>= 1;
        fb0[i + 2] = (b & 1) * 255;
        b >>= 1;
        fb0[i + 3] = (b & 1) * 255;
        b >>= 1;
        fb0[i + 4] = (b & 1) * 255;
        b >>= 1;
        fb0[i + 5] = (b & 1) * 255;
        b >>= 1;
        fb0[i + 6] = (b & 1) * 255;
        b >>= 1;
        fb0[i + 7] = (b & 1) * 255;
      }
    fc++;
    gmlib(GMLIB_UPDATE);
  }
}

void gm_show_fb(unsigned char *frame) {
  u8 fbt[FBSIZE];
  preprocessFrame(frame, fbt);

  unsigned int i, x, y, b, fbsize = (400 / 8 * 640);

  // teu += 130; // teu: next update time
  // if (getmsec() > teu + 1000) {
  //   printf("Skip frame\n");
  //   return;         // drop frame if > 1 sec behind
  // }
  gmlib(GMLIB_VSYNC); // wait for fb0 ready
  for (y = 0; y < 1448; y++)
    for (x = 0; x < 1072; x += 8) {
      b = fbt[600 / 8 * y + x / 8];
      i = y * fs + x;
      fb0[i] = (b & 1) * 255;
      b >>= 1;
      fb0[i + 1] = (b & 1) * 255;
      b >>= 1;
      fb0[i + 2] = (b & 1) * 255;
      b >>= 1;
      fb0[i + 3] = (b & 1) * 255;
      b >>= 1;
      fb0[i + 4] = (b & 1) * 255;
      b >>= 1;
      fb0[i + 5] = (b & 1) * 255;
      b >>= 1;
      fb0[i + 6] = (b & 1) * 255;
      b >>= 1;
      fb0[i + 7] = (b & 1) * 255;
    }
  fc++;
  gmlib(GMLIB_UPDATE);
}

//====================================
// gmlib - geekmaster function library
// op (init, update, vsync, close)
//------------------------------------
int gmlib(int op) {
  static struct update_area_t ua = {0, 0, 1072, 1448, 21, NULL};
  static struct mxcfb_update_data ur = {
      {0, 0, 1072, 1448}, 257, 0, 1, 0x1001, 0, {0, 0, 0, {0, 0, 0, 0}}};
  static struct mxcfb_update_data51 ur51 = {
      {0, 0, 1072, 1448}, 257, 0, 1, 0, 0, 0x1001, 0, {0, 0, 0, {0, 0, 0, 0}}};
  static int eupcode;
  static void *eupdata = NULL;
  struct fb_var_screeninfo screeninfo;
  if (GMLIB_INIT == op) {
    #ifdef X_PC
    ppb = 1;
    fs = 608;
    fb0 = malloc(KINDLE_FB_RES_X * KINDLE_FB_RES_Y);
    colorFb = malloc(KINDLE_FB_RES_X * KINDLE_FB_RES_Y * 4);
    initWindow();
    #else
    teu = getmsec();
    fdFB = open("/dev/fb0", O_RDWR);
    ioctl(fdFB, FBIOGET_VSCREENINFO, &screeninfo);
    ppb = 8 / screeninfo.bits_per_pixel;
    fs = screeninfo.xres_virtual / ppb;
    VY = screeninfo.yres_virtual;
    MX = screeninfo.xres;
    MY = screeninfo.yres;
    ua.x2 = MX;
    ua.y2 = MY;
    ur.update_region.width = MX;
    ur.update_region.height = MY;
    fb0 = (u8 *)mmap(0, MY * fs, PROT_READ | PROT_WRITE, MAP_SHARED, fdFB,
                     0); // map fb0
    if (VY > MY) {
      eupcode = EU50;
      eupdata = &ur;
      ur.update_mode = 0;
      if (ioctl(fdFB, eupcode, eupdata) < 0) {
        eupcode = EU51;
        eupdata = &ur51;
      }
    } else {
      eupcode = EU3;
      eupdata = &ua;
    }
    system("eips -f -c;eips -c");
    sleep(1);
    #endif
  } else if (GMLIB_UPDATE == op) {
    #ifdef X_PC
    showFrame();
    #else
    if (ioctl(fdFB, eupcode, eupdata) < 0) {
      system("eips ''"); // 5.1.0 fallback
    }
    #endif
  } else if (GMLIB_VSYNC == op) {
    #ifdef X_PC
    #else
    while (teu > getmsec()) {
      usleep(1000); // fb0 busy
    }
    #endif
  } else if (GMLIB_CLOSE == op) {
    #ifdef X_PC
    free(fb0);
    free(colorFb);
    XCloseDisplay(s_Display);
    #else
    gmlib(GMLIB_UPDATE);
    sleep(1); // last screen
    system("eips -f -c;eips -c");
    munmap(fb0, MY * fs);
    close(fdFB);
    #endif
  } else {
    return -1;
  }
  return 0;
}
//====================================
// getmsec - get msec since first call
// (tick counter wraps every 12 days)
//------------------------------------
int getmsec(void) {
  int tc;
  static int ts = 0;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  tc = tv.tv_usec / 1000 + 1000 * (0xFFFFF & tv.tv_sec);
  if (0 == ts)
    ts = tc;
  return tc - ts;
}
//==================
// main - start here
//------------------
int main_gmplay(void) {
  int i;
  gmlib(GMLIB_INIT);
  if (ppb - 1) {
    printf("Screen type supported!\n");
  } else {
    gmplay8();
  }
  i = getmsec() / 100;
  printf("%d frames in %0.1f secs = %2.1f FPS\n", fc, (double)i / 10.0,
         (double)fc * 10.0 / i);
  gmlib(GMLIB_CLOSE);
  return 0;
}
