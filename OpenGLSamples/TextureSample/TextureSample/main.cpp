#include "sb7.h"
#include "vmath.h"
#include "sb7ktx.h"

#include <string>

static void print_shader_log(GLuint shader)
{
  std::string str;
  GLint len;

  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
  if (len != 0)
  {
    str.resize(len);
    glGetShaderInfoLog(shader, len, nullptr, &str[0]);
  }

#ifdef WIN32
  OutputDebugStringA(str.c_str());
#endif

}

#define GLSL(version, shader) "#version " #version "\n" #shader  

static const GLchar* vertex_shader_source = GLSL
(
  450 core,
  void main(void)
  {
    const vec4 vertices[] = vec4[](vec4(0.75, -0.75, 0.5, 1.0),
                                   vec4(-0.75, -0.75, 0.5, 1.0),
                                   vec4(0.75, 0.75, 0.5, 1.0));
    gl_Position = vertices[gl_VertexID];
  }
);

static const GLchar* fragment_shader_source = GLSL
(
  450 core,
  uniform sampler2D s;
  
  out vec4 color;
  void main(void)
  {
    color = texture(s, gl_FragCoord.xy / textureSize(s, 0));
  }
);

class SampleTexture : public sb7::application
{
public:
  virtual void startup(void) override
  {
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);

    glTextureStorage2D(texture, 8, GL_RGBA32F, 256, 256);

    float *data = new float[256 * 256 * 4];

    generate_texture(data, 256, 256);

    glTextureSubImage2D(texture, 0, 0, 0, 256, 256, 
                        GL_RGBA, GL_FLOAT, data);

    delete[] data;

    CompileShaders();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    sb7::ktx::file::load();
  }

  virtual void shutdown() override
  {
    glDeleteProgram(program);
    glDeleteVertexArrays(1, &vao);
    glDeleteTextures(1, &texture);
  }

  virtual void render(double current_time) override
  {
    static const GLfloat green[] = { 0.0f, 0.25f,0.0f,1.0f };

    glClearBufferfv(GL_COLOR, 0, green);

    glUseProgram(program);
    glDrawArrays(GL_TRIANGLES, 0, 3);
  }

  void CompileShaders()
  {
    program = glCreateProgram();
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
    glCompileShader(vertex_shader);

    print_shader_log(vertex_shader);

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
    glCompileShader(fragment_shader);

    print_shader_log(fragment_shader);

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);

    glLinkProgram(program);
  }

private:
  void generate_texture(float * data, int width, int height)
  {
    int x, y;

    for (y = 0; y < height; ++y)
    {
      for (x = 0; x < width; ++x)
      {
        data[(y * width + x) * 4 + 0] = (float)((x & y) & 0xFF) / 255.0f;
        data[(y * width + x) * 4 + 1] = (float)((x | y) & 0xFF) / 255.0f;
        data[(y * width + x) * 4 + 2] = (float)((x ^ y) & 0xFF) / 255.0f;
        data[(y * width + x) * 4 + 3] = 1.0f;
      }
    }
  }

public:
  GLuint vao;
  GLuint texture;
  GLuint program;

};

DECLARE_MAIN(SampleTexture);