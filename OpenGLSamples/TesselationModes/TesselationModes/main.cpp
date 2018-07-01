#include <sb7.h>
#include <sb7color.h>

#define GLSL(version, shader) "#version " #version "\n" #shader  

static const GLchar* vertex_shader_source = GLSL
(
  450 core,

  void main(void)
  {
    const vec4 vertices[] = vec4[](vec4( 0.4, -0.4, 0.5, 1.0),
                                   vec4(-0.4, -0.4, 0.5, 1.0),
                                   vec4( 0.4,  0.4, 0.5, 1.0),
                                   vec4(-0.4,  0.4, 0.5, 1.0));

    gl_Position = vertices[gl_VertexID];
  }
);

static const GLchar* tcs_triangles_source = GLSL
(
  450 core,
  layout(vertices = 3) out;

  void main(void)
  {
    if (gl_InvocationID == 0)
    {
      gl_TessLevelInner[0] = 5.0;
      gl_TessLevelOuter[0] = 8.0;
      gl_TessLevelOuter[1] = 7.0;
      gl_TessLevelOuter[2] = 6.0;
    }

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
  }
);

static const GLchar* tes_triangles_source = GLSL
(
  450 core,
  layout(triangles) in;

  void main(void)
  {
    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position) +
                  (gl_TessCoord.y * gl_in[1].gl_Position) +
                  (gl_TessCoord.z * gl_in[2].gl_Position);
  }
);

static const GLchar* tes_triangles_as_points_source = GLSL
(
  450 core,

  layout(triangles, point_mode) in;

  void main(void)
  {
    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position) +
                  (gl_TessCoord.y * gl_in[1].gl_Position) +
                  (gl_TessCoord.z * gl_in[2].gl_Position);
  }
);

static const GLchar* tcs_quads_source = GLSL
(
  450 core,
  layout(vertices = 4) out;

  void main(void)
  {
    if (gl_InvocationID == 0)
    {
      gl_TessLevelInner[0] = 9.0;
      gl_TessLevelInner[1] = 5.0;
      gl_TessLevelOuter[0] = 4.0;
      gl_TessLevelOuter[1] = 5.0;
      gl_TessLevelOuter[2] = 3.0;
      gl_TessLevelOuter[3] = 6.0;
    }

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
  }
);

static const GLchar* tes_quads_source = GLSL
(
  450 core,

  layout(quads) in;

  void main(void)
  {
    vec4 p1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
    vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);
    gl_Position = mix(p1, p2, gl_TessCoord.y);
  }
);

static const GLchar* tcs_isolines_source = GLSL
(
  450 core,
  layout(vertices = 4) out;

  void main(void)
  {
    if (gl_InvocationID == 0)
    {
      gl_TessLevelOuter[0] = 5.0;
      gl_TessLevelOuter[1] = 8.0;
    }

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
  }
);

static const GLchar* tes_isolines_source = GLSL
(
  450 core,
  layout(isolines) in;

  void main(void)
  {
    float r = (gl_TessCoord.y + gl_TessCoord.x / gl_TessLevelOuter[0]);
    float t = gl_TessCoord.x * 2.0 * 3.14159;
    gl_Position = vec4(sin(t) * r, cos(t) *r, 0.5, 1.0);
  }
);

static const GLchar* fragment_shader_source = GLSL
(
  450 core,

  out vec4 color;

  void main(void)
  {
    color = vec4(1.0);
  }
);

class TessellationModes: public sb7::application
{
public:
  TessellationModes():
      program_index(0)
  {

  }

  virtual void startup() override;
  virtual void render(double current_time) override;
  virtual void onKey(int key, int action) override;
  virtual void shutdown() override;

  void LoadShaders();
private:
  GLuint program[4];
  int program_index;
  GLuint vao;
};

void TessellationModes::startup()
{
  LoadShaders();

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glPatchParameteri(GL_PATCH_VERTICES, 4);

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void TessellationModes::LoadShaders()
{
  static const GLchar *const *tcs_sources[] = 
  {
    &tcs_quads_source,
    &tcs_triangles_source,
    &tcs_triangles_source,
    &tcs_isolines_source
  };

  static const GLchar *const *tes_sources[] =
  {
    &tes_quads_source,
    &tes_triangles_source,
    &tes_triangles_as_points_source,
    &tes_isolines_source
  };
  
  for (int i = 0; i < 4; ++i)
  {
    program[i] = glCreateProgram();
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader_source, nullptr);
    glCompileShader(vs);

    GLuint tcs = glCreateShader(GL_TESS_CONTROL_SHADER);
    glShaderSource(tcs, 1, tcs_sources[i], nullptr);
    glCompileShader(tcs);

    GLuint tes = glCreateShader(GL_TESS_EVALUATION_SHADER);
    glShaderSource(tes, 1, tes_sources[i], nullptr);
    glCompileShader(tes);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader_source, nullptr);
    glCompileShader(fs);

    glAttachShader(program[i], vs);
    glAttachShader(program[i], tcs);
    glAttachShader(program[i], tes);
    glAttachShader(program[i], fs);
    glLinkProgram(program[i]);

    glDeleteShader(vs);
    glDeleteShader(tcs);
    glDeleteShader(tes);
    glDeleteShader(fs);
  }
}

void TessellationModes::render(double current_time)
{
  glClearBufferfv(GL_COLOR, 0, sb7::color::Black);

  glUseProgram(program[program_index]);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDrawArrays(GL_PATCHES, 0, 4);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void TessellationModes::shutdown()
{
  glDeleteVertexArrays(1, &vao);

  for (int i = 0; i < 4; ++i)
  {
    glDeleteProgram(program[i]);
  }
}

void TessellationModes::onKey(int key, int action)
{
  if (!action)
  {
    return;
  }

  switch (key)
  {
    case 'M':
      program_index = (program_index + 1) % 4;
      break;
  }
}

DECLARE_MAIN(TessellationModes);