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
  GLuint m_iteration_index;

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
  vmath::vec3 *initial_velocities = new vmath::vec3[POINTS_TOTAL];
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
      initial_velocities[n] = vmath::vec3(0.0f);
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
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo[VELOCITY_A + i]);
    glBufferData(GL_ARRAY_BUFFER, POINTS_TOTAL * sizeof(vmath::vec3), 
                 initial_velocities, GL_DYNAMIC_COPY);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo[CONNECTION]);
    glBufferData(GL_ARRAY_BUFFER, POINTS_TOTAL * sizeof(vmath::ivec4),
                 connection_vectors, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 4, GL_INT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(2);
  }

  delete []connection_vectors;
  delete []initial_velocities;
  delete []initial_positions;

  glGenTextures(2, m_pos_tbo);
  glBindTexture(GL_TEXTURE_BUFFER, m_pos_tbo[0]);
  glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, m_vbo[POSITION_A]);
  glBindTexture(GL_TEXTURE_BUFFER, m_pos_tbo[1]);
  glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, m_vbo[POSITION_B]);

  int lines = (POINTS_X - 1) * POINTS_Y + (POINTS_Y - 1) * POINTS_X;

  glGenBuffers(1, &m_index_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, lines * 2 * sizeof(int), 
               nullptr, GL_STATIC_DRAW);

  int *e = (int *)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, lines * 2 * sizeof(int), 
                                   GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

  for (size_t j = 0; j < POINTS_Y; ++j)
  {
    for (size_t i = 0; i < POINTS_X - 1; ++i)
    {
      *e++ = i + j * POINTS_X;
      *e++ = 1 + i + j * POINTS_X;
    }
  }

  for (size_t i = 0; i < POINTS_X; ++i)
  {
    for (size_t j = 0; j < POINTS_Y - 1; ++j)
    {
      *e++ = i + j * POINTS_X;
      *e++ = POINTS_X + i + j * POINTS_X;
    }
  }

  glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
}

void SpringMass::shutdown()
{
  glDeleteProgram(m_update_program);
  glDeleteBuffers(5, m_vbo);
  glDeleteVertexArrays(2, m_vao);
}

void SpringMass::render(double current_time)
{
  glUseProgram(m_update_program);

  glEnable(GL_RASTERIZER_DISCARD);

  for (size_t i = iterations_per_frame; i != 0; --i)
  {
    glBindVertexArray(m_vao[m_iteration_index & 1]);
    glBindTexture(GL_TEXTURE_BUFFER, m_pos_tbo[m_iteration_index & 1]);
    m_iteration_index++;
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 
                     m_vbo[POSITION_A + (m_iteration_index & 1)]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, 
                     m_vbo[VELOCITY_A + (m_iteration_index & 1)]);
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, POINTS_TOTAL);
    glEndTransformFeedback();
  }

  glDisable(GL_RASTERIZER_DISCARD);

  static const GLfloat black[] = {0.0f, 0.0f, 0.0f, 0.0f};

  glViewport(0,0, info.windowWidth, info.windowHeight);
  glClearBufferfv(GL_COLOR, 0, black);
  
  glUseProgram(m_render_program);

  if (draw_points)
  {
    glPointSize(4.0f);
    glDrawArrays(GL_POINTS, 0, POINTS_TOTAL);
  }

  if (draw_lines)
  {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
    glDrawElements(GL_LINES, CONNECTIONS_TOTAL * 2, 
                   GL_UNSIGNED_INT, nullptr);
  }
}

void SpringMass::onKey(int key, int action)
{
  if (action)
  {
    switch (key)
    {
    case 'R': LoadShaders();
      break;
    case 'L': draw_lines = !draw_lines;
      break;
    case 'P': draw_points = !draw_points;
      break;
    case GLFW_KEY_KP_ADD: ++iterations_per_frame;
      break;
    case GLFW_KEY_KP_SUBTRACT: --iterations_per_frame;
      break;
    }
  }
}

DECLARE_MAIN(SpringMass);