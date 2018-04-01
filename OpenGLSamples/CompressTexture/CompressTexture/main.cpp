#include <sb7.h>
#include <shader.h>
#include <sb7ktx.h>
#include <sb7color.h>

#define TEXTURE_WIDTH 512
#define TEXTURE_HEIGHT 512

class RGTCCompressor : public sb7::application
{
public:
  RGTCCompressor();

  void startup();
  void render(double current_time);
  void onKey(int key, int action);

private:
  GLuint compress_program;
  GLuint render_program;
  GLuint input_texture;
  GLuint output_texture;
  GLuint output_buffer;
  GLuint output_buffer_texture;
  GLuint dummy_vao;

  enum
  {
    SHOW_INPUT,
    SHOW_OUTPUT,
    MAX_MODE
  };

  int display_mode;

  void load_shaders();
};

RGTCCompressor::RGTCCompressor():
  compress_program(0),
  render_program(0),
  display_mode(SHOW_INPUT) 
{}

void RGTCCompressor::startup()
{
  input_texture = sb7::ktx::file::load("../../../media/textures/gllogodistsm.ktx");

  glGenTextures(1, &output_texture);
  glBindTexture(GL_TEXTURE_2D, output_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glGenBuffers(1, &output_buffer);
  glBindBuffer(GL_TEXTURE_BUFFER, output_buffer);
  glBufferStorage(GL_TEXTURE_BUFFER, TEXTURE_WIDTH * TEXTURE_HEIGHT / 2, NULL, GL_MAP_READ_BIT);

  glGenTextures(1, &output_buffer_texture);
  glBindTexture(GL_TEXTURE_BUFFER, output_buffer_texture);
  glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32UI, output_buffer);

  load_shaders();

  glGenVertexArrays(1, &dummy_vao);
  display_mode = SHOW_OUTPUT;
}

void RGTCCompressor::onKey(int key, int action)
{
  if (action)
  {
    switch (key)
    {
    case 'M':
      display_mode = display_mode+1;
      if (display_mode == MAX_MODE) 
      {
        display_mode = SHOW_INPUT;
      }
      break;

    case 'R':
      load_shaders();
      break;
    }
  }
}

void RGTCCompressor::load_shaders()
{
  glDeleteProgram(compress_program);
  glDeleteProgram(render_program);

  GLuint cs;
  
  cs = sb7::shader::load("rgtccompress.cs.glsl", GL_COMPUTE_SHADER);

  compress_program = sb7::program::link_from_shaders(&cs, 1, true);

  GLuint shaders[2];

  shaders[1] = sb7::shader::load("drawquad.fs.glsl", GL_FRAGMENT_SHADER);
  shaders[2] = sb7::shader::load("drawquad.vs.glsl", GL_VERTEX_SHADER);

  render_program = sb7::program::link_from_shaders(shaders, 2, true);
}

DECLARE_MAIN(RGTCCompressor)