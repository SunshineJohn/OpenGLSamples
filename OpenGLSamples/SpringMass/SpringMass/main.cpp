#include "sb7.h"
#include <vmath.h>
#include <shader.h>

enum BUFFER_TYPE_t
{
  POSITION_A,
  POSITION_B,
  VELOCITY_A,
  VELOCITY_B,
  CONNECTION
};

enum
{
  POINTS_X = 50,
  POINTS_Y = 50,
  POINTS_TOTAL = (POINTS_X * POINTS_Y),
  CONNECTIONS_TOTAL = (POINTS_X - 1) * POINTS_Y  + (POINTS_Y - 1) * POINTS_X
};

class SpringMass : public sb7::application
{
  void startup() override;
  void shutdown() override;
  void render(double current_time) override;
  void onKey(int key, int action) override;

  void LoadShaders();

protected:
  GLuint m_vao[2];
  GLuint m_vbo[2];
  GLuint m_index_buffer;
  GLuint m_pos_tbo[2];
  GLuint m_update_program;
  GLuint m_render_program;

  GLuint m_C_loc;
  GLuint m_interation_index;

  bool draw_points;
  bool draw_lines;
  int iterations_per_frame;
};

void SpringMass::LoadShaders()
{
  GLuint vs;
  char buffer[1024];

  vs = sb7::shader::load("update.vs.glsl", GL_VERTEX_SHADER);

  if (m_update_program)
    glDeleteProgram(m_update_program);
  m_update_program = glCreateProgram();

  glAttachShader(m_update_program, vs);

  static const char *tf_varyings[] =
  {
    "tf_position_mass",
    "tf_velocity"
  };

  glTransformFeedbackVaryings(m_update_program, 2, 
                              tf_varyings, GL_SEPARATE_ATTRIBS);

  glLinkProgram(m_update_program);

  glGetShaderInfoLog(vs, 1024, nullptr, buffer);
  glGetProgramInfoLog(m_update_program, 1024, nullptr, buffer);

  glDeleteShader(vs);

  vs = sb7::shader::load("render.vs.glsl", GL_VERTEX_SHADER);
  GLuint fs = sb7::shader::load("render.fs.glsl", GL_FRAGMENT_SHADER);

  if (m_render_program) 
    glDeleteProgram(m_render_program);
  m_render_program = glCreateProgram();

  glAttachShader(m_render_program, vs);
  glAttachShader(m_render_program, fs);

  glLinkProgram(m_render_program);

  glGetShaderInfoLog(vs, 1024, nullptr, buffer);
  glGetShaderInfoLog(fs, 1024, nullptr, buffer);
  glGetProgramInfoLog(m_update_program, 1024, nullptr, buffer);
}

void SpringMass::startup()
{
  LoadShaders();

  vmath::vec4 *initial_positions = new vmath::vec4[POINTS_TOTAL];
  vmath::vec4 *initial_velocities = new vmath::vec4[POINTS_TOTAL];
  vmath::ivec4 *connection_vectors = new vmath::ivec4[POINTS_TOTAL];

  int n = 0;

  for (size_t j = 0; j < POINTS_Y; ++j) 
  {
    float fj = (float)j / (float)POINTS_Y;

    for (size_t i = 0; i < POINTS_X; ++i)
    {
      float fi = (float)i / (float)POINTS_X;

      initial_positions[n] = vmath::vec4((fi - 0.5f)*float(POINTS_X),
                                         (fj - 0.5f)*float(POINTS_Y),
                                         0.6f * sinf(fi) * cosf(fj),
                                         1.0f);
      initial_velocities[n] = vmath::vec4(0.0f);
      connection_vectors[n] = vmath::ivec4(-1);

      if (j != (POINTS_Y - 1))
      {
        if (i != 0)
          connection_vectors[n][0] = n - 1;

        if (j != 0)
          connection_vectors[n][1] = n - POINTS_X;

        if (i != (POINTS_X - 1))
          connection_vectors[n][2] = n + 1;

        if (j != (POINTS_Y - 1))
          connection_vectors[n][3] = n + POINTS_X;
      }

      n++;
    }
  }

  glGenVertexArrays(2, m_vao);
  glGenBuffers(2, m_vbo);

  for (size_t i = 0; i < 2; ++i)
  {
    glBindVertexArray(m_vao[i]);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo[POSITION_A + i]);
    glBufferData(GL_ARRAY_BUFFER, POINTS_TOTAL * sizeof(vmath::vec4), 
                 initial_positions, GL_DYNAMIC_COPY);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
  }
}

DECLARE_MAIN(SpringMass);