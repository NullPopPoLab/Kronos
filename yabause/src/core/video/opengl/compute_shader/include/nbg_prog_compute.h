#ifndef NBG_PROG_COMPUTE_H
#define NBG_PROG_COMPUTE_H

#include "ygl.h"

//Manque Window
//Manque multiple Format
//Manque perline

#define NBG_CS_LOCAL_SIZE_X 8
#define NBG_CS_LOCAL_SIZE_Y 8

#define QuoteIdent(ident) #ident
#define Stringify(macro) QuoteIdent(macro)

#define NB_PRG_VDP2 (1<<11)

static const char nbg_cell_cram_mode_0_f[] =
"layout(std430, binding = 3) readonly buffer VDP2C { uint cram[]; };\n"
"//CRAM Mode 0\n"
"uint ReadCramValue(uint index, uint line) {\n"
"   uint colorval = 0u; \n"
"   index = ((index<<1)&0xFFFu); \n"
"   colorval = cram[index >> 2 + 0x1000u * line]; \n"
"   if ((index & 0x02u) != 0u) { colorval >>= 16; } \n"
"   return colorval & 0xFFFFu; \n"
"}\n"
"vec4 ReadCramColor(uint index, uint line) {\n"
"   uint colorVal = ReadCramValue(index, line);\n"
"   uint cc = (colorVal >> 15u)&0x1u;\n"
"   vec4 color = vec4(0.0);\n"
"   color.r = float((colorVal & 0x1Fu) << 3u)/255.0;\n"
"   color.g = float(((colorVal>>5) & 0x1Fu) << 3u)/255.0;\n"
"   //We are passing the MSB CC as LSB ob blue color\n"
"   color.b = float((((colorVal>>10) & 0x1Fu) << 3u)|cc)/255.0;\n"
"   return color;\n"
"}\n"
"uint get_cram_msb(uint index, uint line) { \n"
"   return (ReadCramValue(index, line) & 0x8000u); \n"
"}\n";

static const char nbg_cell_cram_mode_1_f[] =
"layout(std430, binding = 3) readonly buffer VDP2C { uint cram[]; };\n"
"//CRAM Mode 1\n"
"uint ReadCramValue(uint index, uint line) {\n"
"   uint colorval = 0u; \n"
"   colorval = cram[(index << 1u) + 0x1000u * line]; \n"
"   if ((index & 0x01u) != 0u) { colorval >>= 16; } \n"
"   return colorval & 0xFFFFu; \n"
"}\n"
"vec4 ReadCramColor(uint index, uint line) {\n"
"   uint colorVal = ReadCramValue(index, line);\n"
"   uint cc = (colorVal >> 15u)&0x1u;\n"
"   vec4 color = vec4(0.0);\n"
"   color.r = float((colorVal & 0x1Fu) << 3u)/255.0;\n"
"   color.g = float(((colorVal>>5) & 0x1Fu) << 3u)/255.0;\n"
"//We are passing the MSB CC as LSB ob blue color\n"
"   color.b = float((((colorVal>>10) & 0x1Fu) << 3u)|cc)/255.0;\n"
"   return color;\n"
"}\n"
"uint get_cram_msb(uint index, uint line) { \n"
"   return (ReadCramValue(index, line) & 0x8000u); \n"
"}\n";

static const char nbg_cell_cram_mode_2_f[] =
"layout(std430, binding = 3) readonly buffer VDP2C { uint cram[]; };\n"
"//CRAM Mode 2\n"
"uint ReadCramValue(uint index, uint line) {\n"
"   uint colorval = 0u; \n"
"   colorval = cram[index + 0x1000u * line]; \n"
"   return colorval; \n"
"}\n"
"vec4 ReadCramColor(uint index, uint line) {\n"
"   uint colorVal = ReadCramValue(index, line);\n"
"   uint cc = (colorVal >> 31u)&0x1u;\n"
"   vec4 color = vec4(0.0);\n"
"   color.r = float(colorVal & 0xFFu)/255.0;\n"
"   color.g = float((colorVal>>8) & 0xFFu)/255.0;\n"
"//We are passing the MSB CC as LSB ob blue color\n"
"   color.b = float(((colorVal>>16) & 0xFEu)|cc)/255.0;\n"
"   return color;\n"
"}\n"
"uint get_cram_msb(uint index, uint line) { \n"
"   return ((ReadCramValue(index, line)>>16) & 0x8000u); \n"
"}\n";

static const char nbg_cell_no_cramf[] =
"{//NO CRAM}\n";

static const char nbg_cell_8x8_header_f[] =
SHADER_VERSION_COMPUTE
"#ifdef GL_ES\n"
"precision highp float;\n"
"#endif\n"
"layout(local_size_x = "Stringify(NBG_CS_LOCAL_SIZE_X)", local_size_y = "Stringify(NBG_CS_LOCAL_SIZE_Y)") in;\n"
"layout(rgba8, binding = 0) writeonly uniform image2D outSurface;\n"
"layout(std430, binding = 1) readonly buffer VDP2 { uint vram[]; };\n"
"layout(std430, binding = 2) readonly buffer CMD { uint cmd[]; };\n"
"uint readVdp2RamWord(uint addr) {\n"
"   uint data = vram[ addr>>2u ];\n"
"   if( (addr & 0x02u) != 0u ) { data >>= 16u; } \n"
"   return ((((data) >> 8u) & 0xFFu) | ((data) & 0xFFu) << 8u);\n"
"}\n";

static const char nbg_cell_8x8_main_f[] =
"void main()\n"
"{\n"
"uint idCmd = gl_WorkGroupID.x * 10;\n"
"ivec2 texel = ivec2(cmd[idCmd]+gl_LocalInvocationID.x,cmd[idCmd+1]+gl_LocalInvocationID.y);\n"
"ivec2 size = imageSize(outSurface);\n"
"vec4 outcolor = vec4(0.0);\n"
"uint cellw = cmd[idCmd+8];\n"
"uint cellh = cmd[idCmd+9];\n"
"if (texel.x >= size.x || texel.y >= size.y ) return;\n" //Texel n'est pas bon. Il doit prendre le x/y d'netree
"uint coloroffset = cmd[idCmd+3];\n"
"uint paladdr = cmd[idCmd+4];\n"
"uint priority = cmd[idCmd+5];\n"
"uint cc = 1u;\n"
"uint specialcode = cmd[idCmd+6];\n"
"uint alpha = cmd[idCmd+7];\n"
"uint charaddr = cmd[idCmd+2]+ gl_LocalInvocationID.y*cellw/2 + gl_LocalInvocationID.x/2;\n";

static const char nbg_4bpp[] =
"uint dot = (readVdp2RamWord(charaddr) >> uint(4*(3-mod(texel.x,4)))) & 0xFu;\n"
"uint cramindex = coloroffset + ((paladdr << 4u) | (dot));\n";

static const char nbg_transparency_4bpp[] =
"if ((dot & 0xFu) == 0x0u) {\n"
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
"if (get_cram_msb(index, 0) == 0u)cc = 0;\n";

static const char nbg_normal[] =
"outcolor.a = float(alpha & 0xF8u | priority)/255.0;\n"
"outcolor.r = float((cc<<16u)|((cramindex>>16)& 0xFE))/255.0;\n"
"outcolor.g = float((cramindex>>8)&0xFF)/255.0;\n"
"outcolor.b = float(cramindex & 0xFFu)/255.0;\n";

static const char nbg_normal_mosaic[] =
"outcolor.a = float(alpha & 0xF8u | priority)/255.0;\n"
"outcolor.r = float((cc<<16u)|((cramindex>>16)& 0xFE))/255.0;\n"
"outcolor.g = float((cramindex>>8)&0xFF)/255.0;\n"
"outcolor.b = float(cramindex & 0xFFu)/255.0;\n";

static const char nbg_cram[] =
//Do not use line yet
//"outcolor = ReadCramColor(cramindex, texel.y);\n"
"outcolor = ReadCramColor(cramindex, 0);\n"
"outcolor.a = float(alpha & 0xF8u | priority)/255.0;\n"
"if (alpha == 0u) outcolor = vec4(0.0);\n";

static const char nbg_cram_mosaic[] =
//Do not use line yet
//"outcolor = ReadCramColor(cramindex, texel.y);\n"
"outcolor = ReadCramColor(cramindex, 0);\n"
"outcolor.a = float(alpha & 0xF8u | priority)/255.0;\n"
"if (alpha == 0u) outcolor = vec4(0.0);\n";

static const char nbg_end_f[] =
//"Color = (Color&0xFEFFFFu) | (cc << 16) | ((0xF8u | priority)<<24);\n";
"imageStore(outSurface,texel,outcolor);\n;"
"}\n";



#endif //NBG_PROG_COMPUTE_H