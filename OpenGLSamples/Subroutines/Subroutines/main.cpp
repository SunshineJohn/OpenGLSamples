#include <sb7.h>
#include <shader.h>
#include <string>

class Subroutines : public sb7::application
{
public:
  Subroutines() : render_program(0) {}

  void render(double current_time);
  void startup();
protected:
  void load_shaders();
  void onKey(int key, int action);

  GLuint render_program;
  GLuint vao;

  GLuint subroutines[2];

  struct
  {
    GLuint subroutine;
  } uniforms;
};

void Subroutines::startup()
{
  load_shaders();

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
}

void Subroutines::render(double current_time)
{
  int i = (int)current_time;

  glUseProgram(render_program);

  glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subroutines[i & 1]);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Subroutines::load_shaders()
{
  FILE* fp;

  // Load vertex shader
  fp = fopen("subroutines.vs.glsl", "rb");

  fseek(fp, 0, SEEK_END);
  size_t file_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char* data = new char[file_size + 1];

  fread(data, 1, file_size, fp);
  data[file_size] = 0;
  fclose(fp);

  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &data, nullptr);

  delete[] data;

  glCompileShader(vertex_shader);

  GLint status = 0;
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status);
  if (!status) 
  { 
    char status_vertex[4096];
    glGetShaderInfoLog(vertex_shader, 4096, nullptr, status_vertex);

    OutputDebugStringA(status_vertex);
  }

  // Load fragment shader
  fp = fopen("subroutines.fs.glsl", "rb");

  fseek(fp, 0, SEEK_END);
  file_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  data = new char[file_size + 1];

  fread(data, 1, file_size, fp);
  data[file_size] = 0;
  fclose(fp);

  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &data, nullptr);

  delete[] data;

  glCompileShader(fragment_shader);

  status = 0;
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &status);
  if (!status) 
  { 
    char status_fragment[4096];
    glGetShaderInfoLog(fragment_shader, 4096, nullptr, status_fragment);

    OutputDebugStringA(status_fragment);
  }

  // Link to program
  if (render_program)
  {
    glDeleteProgram(render_program);
  }

  render_program = glCreateProgram();
  glAttachShader(render_program, vertex_shader);
  glAttachShader(render_program, fragment_shader);
  
  glLinkProgram(render_program);

  status = 0;
  glGetProgramiv(render_program, GL_LINK_STATUS, &status);
  if (!status)
  {
    char status_program[4096];
    glGetProgramInfoLog(render_program, 4096, nullptr, status_program);

    OutputDebugStringA(status_program);
  }

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  // Get subroutines;

  subroutines[0] = glGetSubroutineIndex(render_program, 
                                        GL_FRAGMENT_SHADER, 
                                        "myFunction1");

  subroutines[1] = glGetSubroutineIndex(render_program, 
                                        GL_FRAGMENT_SHADER, 
                                        "myFunction2");

  uniforms.subroutine = glGetSubroutineUniformLocation(render_program, GL_FRAGMENT_SHADER, "mySubroutineUniform");
}

void Subroutines::onKey(int key, int action)
{
  if (action)
  {
    switch (key)
    {
    case 'R':
      load_shaders();
      break;
    }
  }
}

DECLARE_MAIN(Subroutines);