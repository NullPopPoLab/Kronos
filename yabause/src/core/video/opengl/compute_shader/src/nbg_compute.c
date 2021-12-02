#include "nbg_compute.h"

#include "nbg_prog_compute.h"

#define NB_PRG_VDP2 (1<<15)

static GLuint prg_vdp2[NB_PRG_VDP2] = {0};

static GLuint createProgram(int count, const GLchar** prg_strs) {
  GLint status;
	int exactCount = 0;
  GLuint result = glCreateShader(GL_COMPUTE_SHADER);

  for (int id = 0; id < count; id++) {
		if (prg_strs[id] != NULL) exactCount++;
		else break;
	}
  glShaderSource(result, exactCount, prg_strs, NULL);
  glCompileShader(result);
  glGetShaderiv(result, GL_COMPILE_STATUS, &status);

  if (status == GL_FALSE) {
    GLint length;
    glGetShaderiv(result, GL_INFO_LOG_LENGTH, &length);
    GLchar *info = malloc(sizeof(GLchar) *length);
    glGetShaderInfoLog(result, length, NULL, info);
    YuiMsg("[COMPILE] %s\n", info);
    free(info);
    abort();
  }
  GLuint program = glCreateProgram();
  glAttachShader(program, result);
  glLinkProgram(program);
  glDetachShader(program, result);
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) {
    GLint length;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
    GLchar *info = malloc(sizeof(GLchar) *length);
    glGetProgramInfoLog(program, length, NULL, info);
    YuiMsg("[LINK] %s\n", info);
    free(info);
    abort();
  }
  return program;
}

static int getProgramId(vdp2draw_struct *info) {
  int id = 0;
  int rank = 0;
	//Cram accesses
  if (info->patternwh == 2) id |= 1 << rank;
  rank += 1;
	if (info->colornumber >= 3) id |= 0 << rank;
	else {
		if (Vdp2Internal.ColorMode == 0)  id |= 1 << rank;
		else if (Vdp2Internal.ColorMode == 1)  id |= 2 << rank;
		else if (Vdp2Internal.ColorMode == 2)  id |= 3 << rank;
	}
	rank += 2;
  //Start Type => all the same
  rank += 0;
  id |= info->colornumber;
  rank += 3;
  //Transparency ==> 2 values

  if (info->transparencyenable) id |= 0 <<rank;
  rank+=1;
  //SpecialPriority
  if (info->specialprimode == 2) {
    if (info->specialfunction & 1) {
      if (info->specialcode & 0x01) id += 2 << rank;
      else if (info->specialcode & 0x02) id += 3 << rank;
      else if (info->specialcode & 0x04) id += 4 << rank;
      else if (info->specialcode & 0x08) id += 5 << rank;
      else if (info->specialcode & 0x10) id += 6 << rank;
      else if (info->specialcode & 0x20) id += 7 << rank;
      else if (info->specialcode & 0x40) id += 8 << rank;
      else if (info->specialcode & 0x80) id += 9 << rank;
      else id += 1 << rank;
    }
    else id += 1 << rank;
  }
  rank += 4;
  //Color calculation
  switch (info->specialcolormode)
  {
    case 1:
      if (info->specialcolorfunction == 0) id += 1 << rank;
      break;
    case 2:
      if (info->specialcolorfunction == 0) id += 1 << rank;
      else id += 2 << rank;;
      break;
   case 3:
      id += 3 << rank;;
     break;
   default:
     break;
  }
  rank += 2;
  //CRAM/Mosaic support
  if (info->colornumber >= 3) {
    if (info->mosaicxmask != 1 || info->mosaicymask != 1) id += 1 << rank; //PG_VDP2_MOSAIC
  } else {
    if (info->mosaicxmask != 1 || info->mosaicymask != 1) id += 2 << rank; //PG_VDP2_MOSAIC_CRAM
    else id += 3 << rank; // PG_VDP2_NORMAL_CRAM
  }
  rank += 2;
  return id;
}

static GLuint createNBGCellProgram(vdp2draw_struct *info) {
	const GLchar * a_prg_vdp2_map[11];
	int nbProg = 0;
	int progId = getProgramId(info);
	if (prg_vdp2[progId] == 0) {
		  //CRAM access
      if (info->patternwh == 2) a_prg_vdp2_map[nbProg++]= nbg_cell_16x16_header_f;
			else a_prg_vdp2_map[nbProg++]= nbg_cell_8x8_header_f;
      a_prg_vdp2_map[nbProg++]= nbg_cell_header_f;
			if (info->colornumber >= 3) a_prg_vdp2_map[nbProg++]= nbg_cell_no_cramf;
			else {
				if (Vdp2Internal.ColorMode == 0) a_prg_vdp2_map[nbProg++]= nbg_cell_cram_mode_0_f;
				else if (Vdp2Internal.ColorMode == 1) a_prg_vdp2_map[nbProg++]= nbg_cell_cram_mode_1_f;
				else if (Vdp2Internal.ColorMode == 2) a_prg_vdp2_map[nbProg++]= nbg_cell_cram_mode_2_f;
			}
			a_prg_vdp2_map[nbProg++]= nbg_cell_main_f;
      switch(info->colornumber) {
        case 0: // 4 BPP
          a_prg_vdp2_map[nbProg++]= nbg_4bpp;
          break;
        case 1: // 8 BPP
          a_prg_vdp2_map[nbProg++]= nbg_8bpp;
          break;
        case 2: // 16 BPP(palette)
          a_prg_vdp2_map[nbProg++]= nbg_16bpp;
          break;
        case 3: // 16 BPP(RGB)
          a_prg_vdp2_map[nbProg++]= nbg_16bpp_rgb;
          break;
        case 4: // 32 BPP
          a_prg_vdp2_map[nbProg++]= nbg_32bpp;
          break;
      }

			//Transparency
			if (info->transparencyenable) a_prg_vdp2_map[nbProg++]= nbg_transparency;
			else a_prg_vdp2_map[nbProg++] = nbg_no_transparency;
      //SpecialPriority
			if (info->specialprimode == 2) {
				if (info->specialfunction & 1) {
					if (info->specialcode & 0x01) a_prg_vdp2_map[nbProg++]= nbg_special_priority_full_1;
					else if (info->specialcode & 0x02) a_prg_vdp2_map[nbProg++]= nbg_special_priority_full_2;
					else if (info->specialcode & 0x04) a_prg_vdp2_map[nbProg++]= nbg_special_priority_full_4;
					else if (info->specialcode & 0x08) a_prg_vdp2_map[nbProg++]= nbg_special_priority_full_8;
					else if (info->specialcode & 0x10) a_prg_vdp2_map[nbProg++]= nbg_special_priority_full_10;
					else if (info->specialcode & 0x20) a_prg_vdp2_map[nbProg++]= nbg_special_priority_full_20;
					else if (info->specialcode & 0x40) a_prg_vdp2_map[nbProg++]= nbg_special_priority_full_40;
					else if (info->specialcode & 0x80) a_prg_vdp2_map[nbProg++]= nbg_special_priority_full_80;
					else a_prg_vdp2_map[nbProg++]= nbg_special_priority;
				}
				else a_prg_vdp2_map[nbProg++]= nbg_special_priority;
			} else a_prg_vdp2_map[nbProg++]= nbg_no_special_priority;

			//Color calculation
			switch (info->specialcolormode)
		  {
			  case 1:
			    if (info->specialcolorfunction == 0) a_prg_vdp2_map[nbProg++]= no_color_calculation;
					else a_prg_vdp2_map[nbProg++]= do_color_calculation;
			    break;
			  case 2:
			    if (info->specialcolorfunction == 0) a_prg_vdp2_map[nbProg++]= no_color_calculation;
			    else a_prg_vdp2_map[nbProg++]= color_calculation_per_dot;
			    break;
			 case 3:
			   a_prg_vdp2_map[nbProg++]= color_calculation_per_cram;
			   break;
			 default:
				 a_prg_vdp2_map[nbProg++]= do_color_calculation;
		  }
			//CRAM/Mosaic support
			if (info->colornumber >= 3) {
		    if (info->mosaicxmask != 1 || info->mosaicymask != 1) a_prg_vdp2_map[nbProg++] = nbg_normal_mosaic; //PG_VDP2_MOSAIC
				else a_prg_vdp2_map[nbProg++] = nbg_normal; //PG_VDP2_NORMAL
			} else {
				if (info->mosaicxmask != 1 || info->mosaicymask != 1) a_prg_vdp2_map[nbProg++] = nbg_cram_mosaic; //PG_VDP2_MOSAIC_CRAM
		    else a_prg_vdp2_map[nbProg++] = nbg_cram; // PG_VDP2_NORMAL_CRAM
			}
			a_prg_vdp2_map[nbProg++]= nbg_end_f;
			prg_vdp2[progId] = createProgram(nbProg, a_prg_vdp2_map);
	}
  return prg_vdp2[progId];
}

GLuint ssbo_vram_ = 0;
GLuint ssbo_cram_ = 0;
GLuint ssbo_cmd_ = 0;

static void initNBGCompute() {
  if (ssbo_vram_ != 0) return;
  glGenBuffers(1, &ssbo_vram_);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_vram_);
  glBufferData(GL_SHADER_STORAGE_BUFFER, 0x100000,(void*)Vdp2Ram,GL_DYNAMIC_DRAW);

  glGenBuffers(1, &ssbo_cram_);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_cram_);
  glBufferData(GL_SHADER_STORAGE_BUFFER, 0x1000,(void*)Vdp2ColorRam,GL_DYNAMIC_DRAW);

  glGenBuffers(1, &ssbo_cmd_);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_cmd_);
  glBufferData(GL_SHADER_STORAGE_BUFFER, 10*0x4000*sizeof(int),NULL,GL_DYNAMIC_DRAW);

}

void CSDrawNBGCell(vdp2draw_struct* info, int** cmdList) {
  if (ssbo_vram_ == 0) initNBGCompute();

  // YuiMsg("Draw id %x colnum %d\n", info->idScreen, info->colornumber);

  int work_groups_x = info->NbCell;

  int id = createNBGCellProgram(info);
  glUseProgram(id);
  // YuiMsg("Use Program %d for nbCmd = %d\n", id, info->NbCell);

  //Vdp2Ram
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_vram_);
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 0x100000, (void*)Vdp2Ram);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_vram_);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_cmd_);
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, info->NbCell*10*sizeof(int), (void*)cmdList);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo_cmd_);

  if (info->colornumber < 3) {
    //Color Ram
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_cram_);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 0x1000, (void*)Vdp2ColorRam);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo_cram_);
  }

  // glClearTexImage(_Ygl->screen_fbotex[info->idScreen], 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  int w, h;
  glBindTexture(GL_TEXTURE_2D,_Ygl->screen_fbotex[info->idScreen]);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
  if ((w != _Ygl->rwidth/info->coordincx) || (h != _Ygl->rheight/info->coordincy))
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _Ygl->rwidth/info->coordincx, _Ygl->rheight/info->coordincy, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);

  glBindImageTexture(0, _Ygl->screen_fbotex[info->idScreen], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

  glDispatchCompute(work_groups_x, 1, 1);

	//Faire le rendu complet avec Texture de sortie, VRAM, CRAM, et param (x:charaddr, y coloroffset, z: paladdr, w: priority)
//Quid de info?
}
