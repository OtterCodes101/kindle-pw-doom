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

#ifndef __GMPLAY__
#define __GMPLAY__

#include <fcntl.h>     // open, close, write
#include <linux/fb.h>  // screeninfo
#include <stdio.h>     // printf
#include <stdlib.h>    // malloc, free
#include <string.h>    // memset, memcpy
#include <sys/ioctl.h> // ioctl
#include <sys/mman.h>  // mmap, munmap
#include <sys/time.h>  // gettimeofday
#include <time.h>      // time
#include <unistd.h>    // usleep

#define KINDLE_RESX 1072
#define KINDLE_RESY 1448

#ifdef X_PC
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define KINDLE_FB_RES_X (KINDLE_RESX + 8)
#define KINDLE_FB_RES_Y (KINDLE_RESY + 8)
#endif

typedef unsigned long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

u32 __invalid_size_argument_for_IOC; // ioctl.h bug fix for tcc
//----- eink definitions from eink_fb.h and mxcfb.h -----

#define EU3 0x46dd
#define EU50 0x4040462e
#define EU51 0x4048462e

struct update_area_t {
  int x1, y1, x2, y2, which_fx;
  u8 *buffer;
};

struct mxcfb_rect {
  u32 top, left, width, height;
};

struct mxcfb_alt_buffer_data {
  u32 phys_addr, width, height;
  struct mxcfb_rect alt_update_region;
};

struct mxcfb_update_data {
  struct mxcfb_rect update_region;
  u32 waveform_mode, update_mode, update_marker;
  int temp;
  unsigned int flags;
  struct mxcfb_alt_buffer_data alt_buffer_data;
};

struct mxcfb_update_data51 {
  struct mxcfb_rect update_region;
  u32 waveform_mode, update_mode, update_marker;
  u32 hist_bw_waveform_mode, hist_gray_waveform_mode;
  int temp;
  unsigned int flags;
  struct mxcfb_alt_buffer_data alt_buffer_data;
};

enum GMLIB_op { GMLIB_INIT, GMLIB_CLOSE, GMLIB_UPDATE, GMLIB_VSYNC };

//----- function prototypes -----
void gmplay8(void);
int getmsec(void);
int gmlib(int);
void gm_show_fb(unsigned char *fbt);
int main_gmplay(void);

#define FBSIZE (KINDLE_RESX / 8 * KINDLE_RESY)

#endif
