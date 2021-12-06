#ifndef NBG_PROG_COMPUTE_H
#define NBG_PROG_COMPUTE_H

#include "ygl.h"

//Manque Window
//Manque perline

#define QuoteIdent(ident) #ident
#define Stringify(macro) QuoteIdent(macro)

static const char nbg_cram_mode_0_f[] =
"layout(std430, binding = 3) readonly buffer VDP2C { uint cram[]; };\n"
"//CRAM Mode 0\n"
"uint ReadCramValue(uint index, uint line) {\n"
"   uint colorval = 0u; \n"
"   index = ((index<<1)&0xFFFu); \n"
"   colorval = cram[index >> 2 + 0x1000u * line]; \n"
"   if ((index & 0x02u) != 0u) { colorval >>= 16; } \n"
"   return (colorval & 0xFFFFu); \n"
"}\n"
"vec4 ReadCramColor(uint index, uint line, uint colorcalc) {\n"
"   uint colorVal = ReadCramValue(index, line);\n"
"   vec4 color = vec4(0.0);\n"
"   color.r = float((colorVal & 0x1Fu) << 3u)/255.0;\n"
"   color.g = float(((colorVal>>5) & 0x1Fu) << 3u)/255.0;\n"
"   //We are passing the MSB CC as LSB ob blue color\n"
"   color.b = float((((colorVal>>10) & 0x1Fu) << 3u)|colorcalc)/255.0;\n"
"   return color;\n"
"}\n"
"uint get_cram_msb(uint index, uint line) { \n"
"   return (ReadCramValue(index, line) & 0x8000u); \n"
"}\n";

static const char nbg_cram_mode_1_f[] =
"layout(std430, binding = 3) readonly buffer VDP2C { uint cram[]; };\n"
"//CRAM Mode 1\n"
"uint ReadCramValue(uint index, uint line) {\n"
"   uint colorval = 0u; \n"
"   index = ((index<<1)&0xFFFu); \n"
"   colorval = cram[index >> 2 + 0x1000u * line]; \n"
"   if ((index & 0x02u) != 0u) { colorval >>= 16; } \n"
"   return colorval & 0xFFFFu; \n"
"}\n"
"vec4 ReadCramColor(uint index, uint line, uint colorcalc) {\n"
"   uint colorVal = ReadCramValue(index, line);\n"
"   vec4 color = vec4(0.0);\n"
"   color.r = float((colorVal & 0x1Fu) << 3u)/255.0;\n"
"   color.g = float(((colorVal>>5) & 0x1Fu) << 3u)/255.0;\n"
"   //We are passing the MSB CC as LSB ob blue color\n"
"   color.b = float((((colorVal>>10) & 0x1Fu) << 3u)|colorcalc)/255.0;\n"
"   return color;\n"
"}\n"
"uint get_cram_msb(uint index, uint line) { \n"
"   return (ReadCramValue(index, line) & 0x8000u); \n"
"}\n";

static const char nbg_cram_mode_2_f[] =
"layout(std430, binding = 3) readonly buffer VDP2C { uint cram[]; };\n"
"//CRAM Mode 2\n"
"uint ReadCramValue(uint index, uint line) {\n"
"   uint colorval = 0u; \n"
"   colorval = cram[index + 0x1000u * line]; \n"
"   return colorval; \n"
"}\n"
"vec4 ReadCramColor(uint index, uint line, uint colorcalc) {\n"
"   uint colorVal = ReadCramValue(index, line);\n"
"   vec4 color = vec4(0.0);\n"
"   color.r = float(colorVal & 0xFFu)/255.0;\n"
"   color.g = float((colorVal>>8) & 0xFFu)/255.0;\n"
"//We are passing the MSB CC as LSB ob blue color\n"
"   color.b = float(((colorVal>>16) & 0xFEu)|colorcalc)/255.0;\n"
"   return color;\n"
"}\n"
"uint get_cram_msb(uint index, uint line) { \n"
"   return ((ReadCramValue(index, line)>>16) & 0x8000u); \n"
"}\n";

static const char nbg_no_cramf[] =
"//NO CRAM\n";

static const char nbg_8x8_header_f[] =
SHADER_VERSION_COMPUTE
"#ifdef GL_ES\n"
"precision highp float;\n"
"#endif\n"
"layout(local_size_x = 8, local_size_y = 8) in;\n";

static const char nbg_16x16_header_f[] =
SHADER_VERSION_COMPUTE
"#ifdef GL_ES\n"
"precision highp float;\n"
"#endif\n"
"layout(local_size_x = 16, local_size_y = 16) in;\n";

static const char nbg_header_f[] =
"layout(rgba8, binding = 0) writeonly uniform image2D outSurface;\n"
"layout(std430, binding = 1) readonly buffer VDP2 { uint vram[]; };\n"
"layout(std430, binding = 2) readonly buffer CMD { uint cmd[]; };\n"
"uint readVdp2RamLong(uint addr) {\n"
"   uint data = vram[ (addr&0x7FFFFu)>>2u ];\n"
"   return data;\n"
"}\n"
"uint readVdp2RamWord(uint addr) {\n"
"   uint data = vram[ (addr&0x7FFFFu)>>2u ];\n"
"   if( (addr & 0x02u) != 0u ) { data >>= 16u; } \n"
"   return ((((data) >> 8u) & 0xFFu) | ((data) & 0xFFu) << 8u);\n"
"}\n";

static const char nbg_cell_main_f[] =
"void main()\n"
"{\n"
"uint idCmd = gl_WorkGroupID.x * 10;\n"
"ivec2 size = imageSize(outSurface);\n"
"ivec2 texel = ivec2(cmd[idCmd]+gl_LocalInvocationID.x,size.y-(cmd[idCmd+1]+gl_LocalInvocationID.y));\n"
"vec4 outcolor = vec4(0.0);\n"
"uint cellw = cmd[idCmd+8];\n"
"uint cellh = cmd[idCmd+9];\n"
"if (texel.x >= size.x || texel.y >= size.y ) return;\n"
"if (texel.x < 0 || texel.y < 0 ) return;\n"
"uint coloroffset = cmd[idCmd+3];\n"
"uint paladdr = cmd[idCmd+4];\n"
"uint priority = cmd[idCmd+5];\n"
"uint cc = 1u;\n"
"uint specialcode = cmd[idCmd+6];\n"
"uint alpha = cmd[idCmd+7];\n"
"ivec2 cellCoord = ivec2(mod(gl_LocalInvocationID.xy, 8));\n"
"uint idCellOffset  = ((uint(gl_LocalInvocationID.y)/8u)*2u + (uint(gl_LocalInvocationID.x)/8u)) * 64u;\n";


static const char nbg_4bpp[] =
"//4bpp\n"
"uint charaddr = cmd[idCmd+2]+ ((idCellOffset + cellCoord.y*8u + cellCoord.x)>>1);\n"
"uint dot = (readVdp2RamWord(charaddr) >> uint(4*(3-(cellCoord.x&0x3u)))) & 0xFu;\n"
"uint cramindex = coloroffset + ((paladdr << 4u) | (dot));\n";

static const char nbg_8bpp[] =
"//8bpp\n"
"uint charaddr = cmd[idCmd+2]+ (idCellOffset + cellCoord.y*8u + cellCoord.x);\n"
"uint dot = (readVdp2RamWord(charaddr) >> uint(8*(1-(cellCoord.x&0x1u)))) & 0xFFu;\n"
"uint cramindex = coloroffset + ((paladdr << 4u) | (dot));\n";

static const char nbg_16bpp[] =
"//16bpp\n"
"uint charaddr = cmd[idCmd+2]+ ((idCellOffset + cellCoord.y*8u + cellCoord.x)<<1);\n"
"uint dot = (readVdp2RamWord(charaddr) & 0xFFFFu;\n"
"uint cramindex = coloroffset + dot;\n";

static const char nbg_16bpp_rgb[] =
"//16bpp_rgb\n"
//Pas bon la
"uint charaddr = cmd[idCmd+2]+ ((idCellOffset + cellCoord.y*8u + cellCoord.x)<<1);\n"
"uint dot = (readVdp2RamWord(charaddr) & 0xFFFFu;\n"
"uint cramindex = coloroffset + dot;\n";

static const char nbg_32bpp[] =
"//32bpp\n"
//Pas bon la
"uint charaddr = cmd[idCmd+2]+ (idCellOffset + cellCoord.y*8u + cellCoord.x)<<2;\n"
"uint dot1 = readVdp2RamWord(charaddr);\n"
"uint dot2 = readVdp2RamWord(charaddr+2);\n"
"uint cramindex = ((dot1 & 0xFFu)<< 16) | (dot2 & 0xFFFFu);\n"
"uint dot = dot1 & 0x8000;\n";

static const char nbg_transparency[] =
"//Transparency On\n"
"if (dot == 0x0u) {\n"
"  imageStore(outSurface,texel,vec4(0.0));\n"
"  return;\n"
"}\n";

static const char nbg_no_transparency[] = "//No Transparency\n";

static const char nbg_no_special_priority[] = "//No Special Priority\n";
static const char nbg_special_priority[] =
"//Special priority\n"
"priority = priority & 0xEu\n;";

static const char nbg_special_priority_full_1[] =
"//Special priority full\n"
"priority = priority & 0xEu\n;"
" if ( (dot == 0x0u || dot == 0x1u) ){ priority |= 1u;} \n";

static const char nbg_special_priority_full_2[] =
"//Special priority full\n"
"priority = priority & 0xEu\n;"
" if ( (dot == 0x2u || dot == 0x3u) ){ priority |= 1u;} \n";

static const char nbg_special_priority_full_4[] =
"//Special priority full\n"
"priority = priority & 0xEu\n;"
" if ( (dot == 0x4u || dot == 0x5u) ){ priority |= 1u;} \n";

static const char nbg_special_priority_full_8[] =
"//Special priority full\n"
"priority = priority & 0xEu\n;"
" if ( (dot == 0x6u || dot == 0x7u) ){ priority |= 1u;} \n";

static const char nbg_special_priority_full_10[] =
"//Special priority full\n"
"priority = priority & 0xEu\n;"
" if ( (dot == 0x8u || dot == 0x9u) ){ priority |= 1u;} \n";

static const char nbg_special_priority_full_20[] =
"//Special priority full\n"
"priority = priority & 0xEu\n;"
" if ( (dot == 0xAu || dot == 0xBu) ){ priority |= 1u;} \n";

static const char nbg_special_priority_full_40[] =
"//Special priority full\n"
"priority = priority & 0xEu\n;"
" if ( (dot == 0xCu || dot == 0xDu) ){ priority |= 1u;} \n";

static const char nbg_special_priority_full_80[] =
"//Special priority full\n"
"priority = priority & 0xEu\n;"
" if ( (dot == 0xEu || dot == 0xFu) ){ priority |= 1u;} \n";

static const char do_color_calculation[] =
"//color calculation ON\n";

static const char no_color_calculation[] =
"//color calculation OFF\n"
"cc = 0;\n";

static const char color_calculation_per_dot[] =
"//color calculation PER DOT\n"
"if ((info->specialcode & (1 << (dot >> 1))) == 0u) cc = 0;\n";

static const char color_calculation_per_cram[] =
"//color calculation PER CRAM\n"
//No line support yet
//"if (get_cram_msb(index, texel.y) == 0u)cc = 0;\n";
"if (get_cram_msb(cramindex, 0) == 0u)cc = 0;\n";

static const char nbg_normal[] =
"outcolor.a = float((alpha & 0xF8u) | priority)/255.0;\n"
"outcolor.b = float(cc|((cramindex>>16)& 0xFEu))/255.0;\n"
"outcolor.g = float((cramindex>>8)&0xFF)/255.0;\n"
"outcolor.r = float(cramindex & 0xFFu)/255.0;\n";

static const char nbg_normal_mosaic[] =
"outcolor.a = float((alpha & 0xF8u) | priority)/255.0;\n"
"outcolor.b = float(cc|((cramindex>>16)& 0xFEu))/255.0;\n"
"outcolor.g = float((cramindex>>8)&0xFF)/255.0;\n"
"outcolor.r = float(cramindex & 0xFFu)/255.0;\n";

static const char nbg_cram[] =
//Do not use line yet
//"outcolor = ReadCramColor(cramindex, texel.y);\n"
"outcolor = ReadCramColor(cramindex & 0xFFFFu, 0, cc);\n"
"outcolor.a = float((alpha & 0xF8u) | priority)/255.0;\n"
"if (outcolor.a == 0u) outcolor = vec4(0.0);\n";

static const char nbg_cram_mosaic[] =
//Do not use line yet
//"outcolor = ReadCramColor(cramindex, texel.y);\n"
"outcolor = ReadCramColor(cramindex  & 0xFFFFu, 0, cc);\n"
"outcolor.a = float((alpha & 0xF8u) | priority)/255.0;\n"
"if (outcolor.a == 0u) outcolor = vec4(0.0);\n";

static const char nbg_end_f[] =
//"Color = (Color&0xFEFFFFu) | (cc << 16) | ((0xF8u | priority)<<24);\n";
"imageStore(outSurface,texel,outcolor);\n"
"}\n";
static const char nbg_bmp_main_f[] =
"layout(std430, binding = 4) readonly buffer LINE { uint line[]; };\n"
"void main()\n"
"{\n"
"ivec2 size = imageSize(outSurface);\n"
"ivec2 texel = ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y);\n"
"vec4 outcolor = vec4(0.0);\n"
"uint cellw = cmd[8];\n"
"uint cellh = cmd[9];\n"
"if (texel.x >= size.x || texel.y >= size.y ) return;\n"
"if (texel.x < 0 || texel.y < 0 ) return;\n"
"uint coloroffset = cmd[3];\n"
"uint paladdr = cmd[4];\n"
"uint priority = cmd[5];\n"
"uint cc = 1u;\n"
"uint specialcode = cmd[6];\n"
"uint alpha = cmd[7];\n"
"ivec2 s = ivec2(int(cmd[0]), int(cmd[1]));\n";

static const char nbg_linescroll[] =
"s.x = s.x + int(line[(size.y-texel.y)*cmd[10]*3+0]);\n"
"s.y = s.y + int(line[(size.y-texel.y)*cmd[10]*3+1]);\n"
"s &= (ivec2(cellw, cellh) - ivec2(1));\n"
"if ((line[(size.y-texel.y)*cmd[10]*3+0] >= 0) && (line[(size.y-texel.y)*cmd[10]*3+1] < s.y)) s.x -= 1;\n";

static const char nbg_bmp_4bpp[] =
"//4bpp\n"
"uint offsety = ((s.x +s.y*cellw)>>2)<<1;\n"
"uint offsetx = (2*(texel.x<<2));\n"
"uint charaddr = cmd[2]+ offsety + offsetx;\n"
"uint dot = (readVdp2RamWord(charaddr) >> uint(4*(3-(cellCoord.x&0x3u)))) & 0xFu;\n"
"uint cramindex = coloroffset + ((paladdr << 4u) | (dot));\n";

static const char nbg_bmp_8bpp[] =
"//8bpp\n"
"uint offsety = ((s.x +s.y*cellw));\n"
"uint offsetx = (2*(texel.x<<1));\n"
"uint charaddr = cmd[2]+ offsety + offsetx;\n"
"uint dot = (readVdp2RamWord(charaddr) >> uint(8*(1-(cellCoord.x&0x1u)))) & 0xFFu;\n"
"uint cramindex = coloroffset + ((paladdr << 4u) | (dot));\n";

static const char nbg_bmp_16bpp[] =
"//16bpp\n"
"uint offsety = ((s.x +s.y*cellw)<<1);\n"
"uint offsetx = (2*(texel.x));\n"
"uint charaddr = cmd[2]+ offsety + offsetx;\n"
"uint dot = (readVdp2RamWord(charaddr) & 0xFFFFu;)\n"
"uint cramindex = coloroffset + dot;\n";

static const char nbg_bmp_16bpp_rgb[] =
"//16bpp_rgb\n"
"uint offsety = ((s.x +s.y*cellw)<<1);\n"
"uint offsetx = (2*(texel.x));\n"
"uint charaddr = cmd[2]+ offsety + offsetx;\n"
"uint cramindex = (readVdp2RamWord(charaddr) & 0xFFFFu);\n"
"uint dot = cramindex & 0x8000;\n";

static const char nbg_bmp_32bpp[] =
"//32bpp\n"
"uint offsety = ((s.x +s.y*cellw)<<2);\n"
"uint offsetx = (4*(texel.x));\n"
"uint charaddr = cmd[2]+ offsety + offsetx;\n"
"uint dot1 = readVdp2RamWord(charaddr);\n"
"uint dot2 = readVdp2RamWord(charaddr+2);\n"
"uint cramindex = ((dot1 & 0xFFu)<< 16) | (dot2 & 0xFFFFu);\n"
"uint dot = dot1 & 0x8000;\n";


#endif //NBG_PROG_COMPUTE_H