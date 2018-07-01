#include <sb7.h>
#include <object.h>
#include <shader.h>
#include <vmath.h>

class ClippingDistance: public sb7::application
{
public:
  void LoadShaders();

  void startup();
  void render(double current_time);
  void onKey(int key, int action);
protected:
  sb7::object object;
  GLuint render_program;
  bool paused;

  struct
  {
    GLuint proj_matrix;
    GLuint mv_matrix;
    GLuint clip_plane;
    GLuint clip_sphere;
  } uniforms;
};

void ClippingDistance::LoadShaders()
{
  if (render_program)
    glDeleteProgram(render_program);

  GLuint shaders[] =
  {
    sb7::shader::load("render.vs.glsl", GL_VERTEX_SHADER),
    sb7::shader::load("render.fs.glsl", GL_FRAGMENT_SHADER)
  };

  render_program = sb7::program::link_from_shaders(shaders, 2, true);

  uniforms.proj_matrix = glGetUniformLocation(render_program, "proj_matrix");
  uniforms.mv_matrix = glGetUniformLocation(render_program, "mv_matrix");
  uniforms.clip_plane = glGetUniformLocation(render_program, "clip_plane");
  uniforms.clip_sphere = glGetUniformLocation(render_program, "clip_sphere");
}

void ClippingDistance::startup()
{
  object.load("../../../media/objects/dragon.sbm");

  LoadShaders();
}

void ClippingDistance::render(double current_time)
{
  static const GLfloat black[] = {0.0f, 0.0f, 0.0f, 0.0f};
  static const GLfloat one = 1.0f;

  static double last_time = 0.0;
  static double total_time = 0.0;

  if (!paused)
  {
    total_time += (current_time - last_time);
  }

  last_time = current_time;

  float f = (float)total_time;

  glClearBufferfv(GL_COLOR, 0, black);
  glClearBufferfv(GL_DEPTH, 0, &one);

  glUseProgram(render_program);

  vmath::mat4 proj_matrix = vmath::perspective(50.0f, (float)info.windowWidth / 
                                                      (float)info.windowHeight,
                                               0.1f, 1000.0f);

  vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -15.0f) *
                          vmath::rotate(f * 1.3f, 0.0f, 1.0f, 0.0f) * 
                          vmath::translate(0.0f, -4.0f, 0.0f);

  vmath::mat4 plane_matrix = vmath::rotate(f * 6.0f, 1.0f, 0.0f, 0.0f) *
                             vmath::rotate(f * 7.0f, 0.0f, 1.0f, 0.0f);

  vmath::vec4 plane = plane_matrix[0];
  plane[3] = 0.0f;
  plane = vmath::normalize(plane);

  vmath::vec4 clip_sphere = vmath::vec4(sinf(f * 0.7f) * 3.0f, 
                                        cosf(f * 1.9f) * 3.0f, 
                                        sinf(f * 0.1f) * 3.0f, 
                                        cosf(f * 1.7f) + 2.5f);

  glUniformMatrix4fv(uniforms.proj_matrix, 1, GL_FALSE, proj_matrix);
  glUniformMatrix4fv(uniforms.mv_matrix, 1, GL_FALSE, mv_matrix);
  glUniform4fv(uniforms.clip_plane, 1, plane);
  glUniform4fv(uniforms.clip_sphere, 1, clip_sphere);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CLIP_DISTANCE0);
  glEnable(GL_CLIP_DISTANCE1);

  object.render();
}

void ClippingDistance::onKey(int key, int action)
{
  if (action)
  {
    switch (key)
    {
      case 'P':
        paused = !paused;
        break;
      case 'R':
        LoadShaders();
        break;
    }
  }
}

DECLARE_MAIN(ClippingDistance);