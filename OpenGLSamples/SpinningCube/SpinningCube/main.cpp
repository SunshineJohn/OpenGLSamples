#include "sb7.h"
#include "vmath.h"

#define GLSL(version, shader) "#version " #version "\n" #shader  

static const GLchar* vertex_shader_source = GLSL
(
  450 core,
  in vec4 position;

  out VS_OUT
  {
    vec4 color;
  } vs_out;

  uniform mat4 mv_matrix;
  uniform mat4 proj_matrix;

  void main(void)
  {
    gl_Position = proj_matrix * mv_matrix * position;
    vs_out.color = position*2.0 + vec4(0.5, 0.5, 0.5, 0.0);
  }
);

static const GLchar* fragment_shader_source = GLSL
(
  450 core,
  out vec4 color;

  in VS_OUT
  {
    vec4 color;
  } fs_in;

  void main(void)
  {
    color = fs_in.color;
  }
);

class SpinningCube : public sb7::application
{
public:
  void LoadShaders()
  {
    GLuint vertex_shader;
    GLuint fragment_shader;

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
    glCompileShader(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
    glCompileShader(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
  }
  
  virtual void startup() override
  {
    LoadShaders();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    static const GLfloat vertex_position[] =
    {
      -0.25f,  0.25f, -0.25f,
      -0.25f, -0.25f, -0.25f,
      0.25f, -0.25f, -0.25f,

      0.25f, -0.25f, -0.25f,
      0.25f,  0.25f, -0.25f,
      -0.25f,  0.25f, -0.25f,

      0.25f, -0.25f, -0.25f,
      0.25f, -0.25f,  0.25f,
      0.25f,  0.25f, -0.25f,

      0.25f, -0.25f,  0.25f,
      0.25f,  0.25f,  0.25f,
      0.25f,  0.25f, -0.25f,

      0.25f, -0.25f,  0.25f,
      -0.25f, -0.25f,  0.25f,
      0.25f,  0.25f,  0.25f,

      -0.25f, -0.25f,  0.25f,
      -0.25f,  0.25f,  0.25f,
      0.25f,  0.25f,  0.25f,

      -0.25f, -0.25f,  0.25f,
      -0.25f, -0.25f, -0.25f,
      -0.25f,  0.25f,  0.25f,

      -0.25f, -0.25f, -0.25f,
      -0.25f,  0.25f, -0.25f,
      -0.25f,  0.25f,  0.25f,

      -0.25f, -0.25f,  0.25f,
      0.25f, -0.25f,  0.25f,
      0.25f, -0.25f, -0.25f,

      0.25f, -0.25f, -0.25f,
      -0.25f, -0.25f, -0.25f,
      -0.25f, -0.25f,  0.25f,

      -0.25f,  0.25f, -0.25f,
      0.25f,  0.25f, -0.25f,
      0.25f,  0.25f,  0.25f,

      0.25f,  0.25f,  0.25f,
      -0.25f,  0.25f,  0.25f,
      -0.25f,  0.25f, -0.25f
    };

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(vertex_position),
                 vertex_position,
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
  }

  virtual void render(double current_time) override
  {
    static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };

    float f = (float)current_time*0.314f;
    vmath::mat4 mv_matrix =
      vmath::translate(0.0f, 0.0f, -4.0f)*
      vmath::translate(sinf(2.1f*f)*0.5f,
                       cosf(1.7f*f)*0.5f,
                       sinf(1.3f*f)*cosf(1.5f*f)*2.0f)*
      vmath::rotate((float)current_time*45.0f, 0.0f, 1.0f, 0.0f)*
      vmath::rotate((float)current_time*81.0f, 1.0f, 0.0f, 0.0f);

    glClearBufferfv(GL_COLOR, 0, green);

    glUseProgram(program);

    glUniformMatrix4fv(mv_location, 1, GL_FALSE, mv_matrix);
    glUniformMatrix4fv(proj_location, 1, GL_FALSE, proj_matrix);

    glDrawArrays(GL_TRIANGLES, 0, 36);
  }

  virtual void shutdown() override
  {
    glDisableVertexAttribArray(0);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);
    glDeleteBuffers(1, &buffer);
  }

  virtual void onResize(int w, int h) override
  {
    sb7::application::onResize(w, h);
    aspect_ratio = (float)info.windowWidth / (float)info.windowHeight;
    proj_matrix = vmath::perspective(50.0f, aspect_ratio, 0.1f, 1000.0f);
  }

private:
  GLuint vao;
  GLuint buffer;
  GLuint program;
  GLuint mv_location;
  GLuint proj_location;

  float aspect_ratio;

  vmath::mat4 proj_matrix;
};

DECLARE_MAIN(SpinningCube)