#ifndef NBG_PROG_COMPUTE_H
#define NBG_PROG_COMPUTE_H

#include "ygl.h"

#define VDP2_LOCAL_SIZE_X 4
#define VDP2_LOCAL_SIZE_Y 4

#define QuoteIdent(ident) #ident
#define Stringify(macro) QuoteIdent(macro)

enum
{
  VDP2_MAP_PAT_SIZE_0_WH_0 = 0,
  VDP2_MAP_PAT_SIZE_0_WH_1,
  VDP2_MAP_PAT_SIZE_1_WH_0,
  VDP2_MAP_PAT_SIZE_1_WH_1,
  NB_PRG_VDP2
};

static const char nbg_map_start_f[] =
SHADER_VERSION_COMPUTE
"#ifdef GL_ES\n"
"precision highp float;\n"
"#endif\n"
"layout(local_size_x = "Stringify(VDP2_LOCAL_SIZE_X)", local_size_y = "Stringify(VDP2_LOCAL_SIZE_Y)") in;\n"
"layout(rgba8, binding = 0) writeonly uniform image2D outSurface;\n"
"layout(std430, binding = 1) readonly buffer VDP2 { uint vram[]; };\n"
"layout(location = 3) uniform ivec4 param;\n"
"int readVdp2RamWord(int addr) {\n"
"	 int ddata = vram[ addr>>2 ]; \n"
"  if( (addr & 0x02u) != 0u ) { data >>= 16; } \n"
"  return (((data) >> 8 & 0xFFu) | ((data) & 0xFFu) << 8);\n"
"\n}"
"void main()\n"
"{\n"
"ivec2 texel = ivec2(gl_GlobalInvocationID.xy);\n"
"ivec2 size = imageSize(outSurface);\n"
"int idx = 0;\n"
"int cellw = 8;\n"
"int cellh = 8;\n"
"if (texel.x >= size.x || texel.y >= size.y ) return;\n";
"int coloroffset = param.y;\n"
"int paladdr = param.z;\n"
"int priority = parm.w;\n"
"int charaddr = param.x + texel.y*cellw/2 + texel.x/2;\n"
//getPixel4bpp
"int dot = (readVdp2RamWord(charaddr) >> (4*(3-mod(texel.x,4)))) & 0xFu;\n"
//Do Not take care of transparency yet
"int cramindex = coloroffset + ((paladdr << 4u) | (dot));\n"
//No specialpriority yet
//cc considered 1 for now
// No alpha yet
"int Color = ReadCramLong(cramindex);\n" //Mettre la colorRam (pas la texture du coup)
"Color = (Color&0xFEFFFFu) | (cc << 16) | ((0xF8 | priority)<<24);\n"
//Ecrire la valeur dans screen_tex[NBG0]

static const char nbg_normal[] =
"addr = ((offset[idx] & 0x7F) >> deca) * (multi * 0x1000);\n";

static const char nbg_normal_mosaic[] =
"addr = ((offset[idx] & 0x1F) >> deca) * (multi * 0x4000);\n";

static const char nbg_cram[] =
"addr = (offset[idx] >> deca) * (multi * 0x800);\n";

static const char nbg_cram_mosaic[] =
"addr = ((offset[idx] & 0x3F) >> deca) * (multi * 0x2000);\n";


static const char nbg_map_end_f[] =
"}\n";



#endif //NBG_PROG_COMPUTE_H