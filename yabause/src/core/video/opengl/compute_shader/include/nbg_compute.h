#ifndef __NBG_COMPUTE_H__
#define __NBG_COMPUTE_H__

#include "vidshared.h"
#include "ygl.h"
#include "vdp2.h"

extern void CSDrawNBGCell(vdp2draw_struct* info);
extern void CSDrawNBGBitmapScroll(vdp2draw_struct* info, int width, int height);
extern void DrawCellOrderCS(vdp2draw_struct * info, int x, int y);
extern int NBGCmdList[0x8][0x4000][10];

#endif // __NBG_COMPUTE_H__