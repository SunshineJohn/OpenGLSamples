#include <sb7.h>
#include <vmath.h>

#include <object.h>
#include <shader.h>
#include <sb7ktx.h>

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

  uniform mat4 mv_matrix;
  uniform mat4 proj_matrix;

  layout  (location = 0) in vec4 position;
  layout  (location = 4) in vec2 tc;

  out VS_OUT
  {
    vec2 tc;
  } vs_out;

  void main(void)
  {
    vec4 pos_vs = mv_matrix * position;

    vs_out.tc = tc;

    gl_Position = proj_matrix * pos_vs;
  }
);

static const GLchar* fragment_shader_source = GLSL
(
  450 core,

  layout(binding = 0) uniform sampler2D tex_object;

  in VS_OUT
  {
    vec2 tc;
  } fs_in;

  out vec4 color;

  void main(void)
  {
    color = texture(tex_object, fs_in.tc * vec2(3.0, 1.0));
  }
);

class TextureCoords : public sb7::application
{
public:
  TextureCoords() :
      render_program(0),
      texture_index(0)
  {
  }

protected:
  virtual void startup()
  {
#define B 0x00,0x00,0x00,0x00
#define W 0xFF,0xFF,0xFF,0xFF
    static const GLubyte tex_data[] =
    {
      B, W, B, W, B, W, B, W, B, W, B, W, B, W, B, W,
      W, B, W, B, W, B, W, B, W, B, W, B, W, B, W, B,
      B, W, B, W, B, W, B, W, B, W, B, W, B, W, B, W,
      W, B, W, B, W, B, W, B, W, B, W, B, W, B, W, B,
      B, W, B, W, B, W, B, W, B, W, B, W, B, W, B, W,
      W, B, W, B, W, B, W, B, W, B, W, B, W, B, W, B,
      B, W, B, W, B, W, B, W, B, W, B, W, B, W, B, W,
      W, B, W, B, W, B, W, B, W, B, W, B, W, B, W, B,
      B, W, B, W, B, W, B, W, B, W, B, W, B, W, B, W,
      W, B, W, B, W, B, W, B, W, B, W, B, W, B, W, B,
      B, W, B, W, B, W, B, W, B, W, B, W, B, W, B, W,
      W, B, W, B, W, B, W, B, W, B, W, B, W, B, W, B,
      B, W, B, W, B, W, B, W, B, W, B, W, B, W, B, W,
      W, B, W, B, W, B, W, B, W, B, W, B, W, B, W, B,
      B, W, B, W, B, W, B, W, B, W, B, W, B, W, B, W,
      W, B, W, B, W, B, W, B, W, B, W, B, W, B, W, B,
    };
#undef B
#undef W

    glGenTextures(1, &texture_object[0]);
    glBindTexture(GL_TEXTURE_2D, texture_object[0]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, 16, 16);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 16, 16, GL_RGBA, 
                    GL_UNSIGNED_BYTE, tex_data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    texture_object[1] = sb7::ktx::file::load("../../../media/textures/pattern1.ktx");

    object.load("../../../media/objects/torus_nrms_tc.sbm");

    load_shaders();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
  }

  virtual void shutdown()
  {
    glDeleteProgram(render_program);
    glDeleteTextures(2, texture_object);
  }

  virtual void render(double current_time)
  {
    static const GLfloat gray[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    static const GLfloat ones[] = { 1.0f };

    glClearBufferfv(GL_COLOR, 0, gray);
    glClearBufferfv(GL_DEPTH, 0, ones);

    glViewport(0, 0, info.windowWidth, info.windowHeight);

    glBindTexture(GL_TEXTURE_2D, texture_object[texture_index]);

    glUseProgram(render_program);

    vmath::mat4 proj_matrix = 
        vmath::perspective(60.0f, (float)info.windowWidth / 
                           (float)info.windowHeight, 0.1f, 1000.0f);

    vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -3.0f)*
        vmath::rotate((float)current_time*19.3f, 0.0f, 1.0f, 0.0f)*
        vmath::rotate((float)current_time*21.1f, 0.0f, 0.0f, 1.0f);

    glUniformMatrix4fv(uniforms.mv_matrix, 1, GL_FALSE, mv_matrix);
    glUniformMatrix4fv(uniforms.proj_matrix, 1, GL_FALSE, proj_matrix);

    object.render();
  }

  void load_shaders()
  {
    if (render_program)
    {
      glDeleteProgram(render_program);
    }

    GLuint vertex_shader = 0;
    GLuint fragment_shader = 0;

    vertex_shader = 
        sb7::shader::load("../../../media/shaders/simpletexcoords/render.vs.glsl", 
                          GL_VERTEX_SHADER);
    fragment_shader = 
        sb7::shader::load("../../../media/shaders/simpletexcoords/render.fs.glsl", 
                          GL_FRAGMENT_SHADER);

    /*vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
    glCompileShader(vertex_shader);

    print_shader_log(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
    glCompileShader(fragment_shader);

    print_shader_log(fragment_shader);*/

    render_program = glCreateProgram();

    glAttachShader(render_program, vertex_shader);
    glAttachShader(render_program, fragment_shader);

    glLinkProgram(render_program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    uniforms.mv_matrix = glGetUniformLocation(render_program, "mv_matrix");
    uniforms.proj_matrix = glGetUniformLocation(render_program, "proj_matrix");
  }

  virtual void onKey(int key, int action)
  {
    if (action)
    {
      switch (key)
      {
      case 'R': 
        load_shaders();
        break;
      case 'T':
        if (texture_index == 1)
          texture_index = 0;
        else
          ++texture_index;
        break;
      }
    }
  }
protected:
  GLuint render_program;

  GLuint texture_object[2];
  GLuint texture_index;

  struct
  {
    GLuint mv_matrix;
    GLuint proj_matrix;
  } uniforms;

  sb7::object object;
};

DECLARE_MAIN(TextureCoords)