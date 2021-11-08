#include "nbg_compute.h"

#include "nbg_prog_compute.h"


static GLuint prg_vdp2[NB_PRG_VDP2] = {0};
static const GLchar * a_prg_vdp2_map[NB_PRG_VDP2][3] = {
  // PG_VDP2_NORMAL
	{
		nbg_map_start_f,
		nbg_normal,
		nbg_map_end_f
	},
  // PG_VDP2_MOSAIC
  {
		nbg_map_start_f,
		nbg_normal_mosaic,
		nbg_map_end_f
	},
  // PG_VDP2_NORMAL_CRAM
  {
		nbg_map_start_f,
		nbg_cram,
		nbg_map_end_f
	},
  // PG_VDP2_MOSAIC_CRAM
  {
		nbg_map_start_f,
		nbg_cram_mosaic,
		nbg_map_end_f
	}
};

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
  if (info->colornumber >= 3) {
    if (info->mosaicxmask != 1 || info->mosaicymask != 1) return 1; //PG_VDP2_MOSAIC
    return 0; //PG_VDP2_NORMAL
  }
  else {
    if (info->mosaicxmask != 1 || info->mosaicymask != 1) return 3; //PG_VDP2_MOSAIC_CRAM
    return 2; // PG_VDP2_NORMAL_CRAM
  }
}

void CSDrawNBGCell(vdp2draw_struct* info, int** cmdList, int nbCmd) {

  int work_groups_x = nbCmd*8 / VDP2_LOCAL_SIZE_X;
  int work_groups_y = nbCmd*8 / VDP2_LOCAL_SIZE_Y;

  int progId = getProgramId(info);
  if (prg_vdp2[progId] == 0)
    prg_vdp2[progId] = createProgram(sizeof(a_prg_vdp2_map[progId]) / sizeof(char*), (const GLchar**)a_prg_vdp2_map[progId]);
  glUseProgram(prg_vdp2[progId]);
//Quid de info?
}
