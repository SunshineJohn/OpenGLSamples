#include <sb7.h>
#include <sb7ktx.h>
#include <vmath.h>

#include <cmath>

#define GLSL(version, shader) "#version " #version "\n" #shader  

static const GLchar* vertex_shader_source = GLSL
(
  450 core,
  layout (location = 0) in vec4 position;
  layout (location = 1) in vec4 color;

  uniform float time;
  uniform mat4 proj_matrix;

  flat out vec4 star_color;

  void main(void)
  {
    vec4 newVertex = position;
    newVertex.z += time;
    newVertex.z = fract(newVertex.z);

    float size = (20.0 * newVertex.z * newVertex.z);

    star_color = smoothstep(1.0, 7.0, size) * color;

    newVertex.z = (999.9 * newVertex.z) - 1000.0;
    gl_Position = proj_matrix * newVertex;
    gl_PointSize = size;
  }
);

static const GLchar* fragment_shader_source = GLSL
(
  450 core,
  layout (location = 0) out vec4 color;

  uniform sampler2D tex_star;
  flat in vec4 star_color;

  void main(void)
  {
    color = star_color * texture(tex_star, gl_PointCoord);
  }
);

static unsigned int seed = 0x133371337;

static inline float random_float()
{
  float res;
  unsigned int tmp;

  seed *= 16807;

  tmp = seed ^ (seed >> 4) ^ (seed << 15);

  *((unsigned int *)&res) = (tmp >> 9) | 0x3F800000;

  return (res - 1.0f);
}

enum
{
  NUM_STARS = 2000
};

class Stars : public sb7::application
{
private:
  void LoadShaders();

  void startup() override;
  void render(double time) override;

protected:
  GLuint program;
  GLuint star_texture;
  GLuint star_vao;
  GLuint star_buffer;

  struct
  {
    GLuint time;
    GLuint proj_matrix;
  } uniforms;
};

void Stars::LoadShaders()
{
  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
  glCompileShader(vertex_shader);

  char *buffer = new char[4096];
  glGetShaderInfoLog(vertex_shader, 4096, nullptr, buffer);
  OutputDebugStringA(buffer);

  delete []buffer;

  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
  glCompileShader(fragment_shader);

  buffer = new char[4096];
  glGetShaderInfoLog(fragment_shader,4096, nullptr, buffer);
  OutputDebugStringA(buffer);

  delete []buffer;

  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);

  glLinkProgram(program);

  buffer = new char[4096];
  glGetProgramInfoLog(program, 4096, nullptr, buffer);
  OutputDebugStringA(buffer);

  delete []buffer;

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
}

void Stars::startup()
{
  LoadShaders();

  uniforms.time = glGetUniformLocation(program, "time");
  uniforms.proj_matrix = glGetUniformLocation(program, "proj_matrix");

  star_texture = sb7::ktx::file::load("../../../media/textures/star.ktx");

  glGenVertexArrays(1, &star_vao);
  glBindVertexArray(star_vao);

  struct star_t
  {
    vmath::vec3 position;
    vmath::vec3 color;
  };

  glGenBuffers(1, &star_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, star_buffer);
  glBufferData(GL_ARRAY_BUFFER, NUM_STARS * sizeof(star_t), 
               nullptr, GL_STATIC_DRAW);

  star_t *star = (star_t*)glMapBufferRange(GL_ARRAY_BUFFER, 0, NUM_STARS * sizeof(star_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

  for (int i = 0; i < 1000; ++i) 
  {
    star[i].position[0] = (random_float() * 2.0f - 1.0f) * 100.0f;
    star[i].position[1] = (random_float() * 2.0f - 1.0f) * 100.0f;
    star[i].position[2] = random_float();
    star[i].color[0] = 0.8f + random_float() * 0.2f;
    star[i].color[1] = 0.8f + random_float() * 0.2f;
    star[i].color[2] = 0.8f + random_float() * 0.2f;
  }

  glUnmapBuffer(GL_ARRAY_BUFFER);

  glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE, sizeof(star_t), NULL);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(star_t), (void*)sizeof(vmath::vec3));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
}

void Stars::render(double current_time)
{
  static const GLfloat black[] = {0.0f, 0.0f, 0.0f, 0.0f};
  static const GLfloat one[] = { 1.0f };

  float t = (float)current_time;

  vmath::mat4 proj_matrix = vmath::perspective(50.0f,
                                               (float)info.windowWidth/
                                               (float)info.windowHeight,
                                               0.1f,
                                               1000.0f);

  t *= 0.1f;
  t -= floor(t);

  glViewport(0,0,info.windowWidth, info.windowHeight);
  glClearBufferfv(GL_COLOR, 0, black);
  glClearBufferfv(GL_DEPTH, 0, one);

  glUseProgram(program);

  glUniform1f(uniforms.time, t);
  glUniformMatrix4fv(uniforms.proj_matrix, 1, GL_FALSE, proj_matrix);

  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);

  glBindVertexArray(star_vao);

  glEnable(GL_PROGRAM_POINT_SIZE);
  glDrawArrays(GL_POINTS, 0, NUM_STARS);
}

DECLARE_MAIN(Stars);