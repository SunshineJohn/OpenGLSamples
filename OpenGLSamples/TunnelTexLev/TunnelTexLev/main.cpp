#include <sb7.h>
#include <sb7ktx.h>
#include <vmath.h>

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
  out VS_OUT
  {
    vec2 tc;
  } vs_out;

  uniform mat4 mvp;
  uniform float offset;

  void main(void)
  {
    const vec2[4] position = vec2[4](vec2(-0.5, -0.5),
                                     vec2( 0.5, -0.5),
                                     vec2(-0.5,  0.5),
                                     vec2( 0.5,  0.5));

    vs_out.tc = (position[gl_VertexID].xy + vec2(offset, 0.5)) * vec2(30.0, 1.0);

    gl_Position = mvp * vec4(position[gl_VertexID], 0.0, 1.0);
  }
);

static const GLchar* fragment_shader_source = GLSL
(
  450 core,
  layout (location = 0) out vec4 color;

  in VS_OUT
  {
    vec2 tc;
  } fs_in;

  layout (binding = 0) uniform sampler2D tex;

  void main(void)
  {
    color = texture(tex, fs_in.tc);
  }
);

class Tunnel : public sb7::application
{
public:
  virtual void startup() override
  {
    load_shaders();

    uniforms.mvp = glGetUniformLocation(render_prog, "mvp");
    uniforms.offset = glGetUniformLocation(render_prog, "offset");

    glGenVertexArrays(1, &render_vao);
    glBindVertexArray(render_vao);

    tex_wall = sb7::ktx::file::load("../../../media/textures/brick.ktx");
    tex_ceiling = sb7::ktx::file::load("../../../media/textures/ceiling.ktx");
    tex_floor = sb7::ktx::file::load("../../../media/textures/floor.ktx");

    GLuint textures[] = { tex_floor, tex_wall, tex_ceiling };

    for (int i = 0; i < 3; ++i)
    {
      glBindTexture(GL_TEXTURE_2D, textures[i]);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    glBindVertexArray(render_vao);
  }

  virtual void render(double current_time) override
  {
    static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    float t = (float)current_time;

    glViewport(0, 0, info.windowWidth, info.windowHeight);
    glClearBufferfv(GL_COLOR, 0, black);

    glUseProgram(render_prog);

    vmath::mat4 proj_matrix = 
        vmath::perspective(60.0f, (float)info.windowWidth / 
                                  (float)info.windowHeight, 
                           0.1f, 1000.0f);

    glUniform1f(uniforms.offset, t*0.03f);

    GLuint textures[] = { tex_wall, tex_floor, tex_wall, tex_ceiling };

    for (int i = 0; i < 4; ++i)
    {
      vmath::mat4 mv_matrix = vmath::rotate(90.0f*(float)i, 
                                            vmath::vec3(0.0f, 0.0f, 1.0f)) *
                              vmath::translate(-0.5f, 0.0f, -10.0f) *
                              vmath::rotate(90.0f, 0.0f, 1.0f, 0.0f) *
                              vmath::scale(30.0f, 1.0f, 1.0f);

      vmath::mat4 mvp = proj_matrix * mv_matrix;

      glUniformMatrix4fv(uniforms.mvp, 1, GL_FALSE, mvp);
      glBindTexture(GL_TEXTURE_2D, textures[i]);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
  }

  void load_shaders()
  {
    char buffer[1024];

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
    glCompileShader(vertex_shader);

    glGetShaderInfoLog(vertex_shader, 1024, nullptr, buffer);

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
    glCompileShader(fragment_shader);

    glGetShaderInfoLog(fragment_shader, 1024, nullptr, buffer);

    render_prog = glCreateProgram();
    glAttachShader(render_prog, vertex_shader);
    glAttachShader(render_prog, fragment_shader);

    glLinkProgram(render_prog);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    glGetProgramInfoLog(render_prog, 1024, nullptr, buffer);
  }
protected:
  GLuint render_prog;
  GLuint render_vao;

  struct
  {
    GLuint mvp;
    GLuint offset;
  } uniforms;

  GLuint tex_wall;
  GLuint tex_ceiling;
  GLuint tex_floor;
};

DECLARE_MAIN(Tunnel)