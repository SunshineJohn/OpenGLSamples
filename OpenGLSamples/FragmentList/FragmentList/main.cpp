#include <sb7.h>
#include <vmath.h>

#include <object.h>
#include <sb7ktx.h>
#include <shader.h>

#include <string>

class FragmentList : public sb7::application
{
public:
  FragmentList():
      clear_program(0),
      append_program(0),
      resolve_program(0) {}

  virtual void startup() override;
  virtual void render(double current_time) override;
  virtual void onKey(int key, int action) override;

  void load_shaders();

protected:
  GLuint clear_program;
  GLuint append_program;
  GLuint resolve_program;

  struct
  {
    GLuint color;
    GLuint normals;
  } textures;

  struct uniforms_block
  {
    vmath::mat4 mv_matrix;
    vmath::mat4 view_matrix;
    vmath::mat4 proj_matrix;
  };

  GLuint uniforms_buffer;

  struct
  {
    GLuint mvp;
  } uniforms;

  sb7::object object;

  GLuint fragment_buffer;
  GLuint head_pointer_image;
  GLuint atomic_counter_buffer;
  GLuint dummy_vao;
};

void FragmentList::startup()
{
  load_shaders();

  uniforms.mvp = glGetUniformLocation(append_program, "mvp");

  glGenBuffers(1, &uniforms_buffer);
  glBindBuffer(GL_UNIFORM_BUFFER, uniforms_buffer);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(uniforms_block), 
               nullptr, GL_DYNAMIC_DRAW);

  object.load("../../../media/objects/dragon.sbm");

  glGenBuffers(1, &fragment_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, fragment_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, 1024 * 1024 * 16, nullptr, GL_DYNAMIC_COPY);

  glGenBuffers(1, &atomic_counter_buffer);
  glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomic_counter_buffer);
  glBufferData(GL_ATOMIC_COUNTER_BUFFER, 4, nullptr, GL_DYNAMIC_COPY);

  glGenTextures(1, &head_pointer_image);
  glBindTexture(GL_TEXTURE_2D, head_pointer_image);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, 1024, 1024);

  glGenVertexArrays(1, &dummy_vao);
  glBindVertexArray(dummy_vao);
}

void FragmentList::render(double current_time)
{
  static const GLfloat zeros[] = { 0.0f, 0.0f, 0.0f, 0.0f };
  static const GLfloat gray[] = { 0.1f, 0.1f, 0.1f, 0.0f };
  static const GLfloat ones[] = { 1.0f };

  const float f = (float)current_time;

  glViewport(0, 0, info.windowWidth, info.windowHeight);

  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | 
                  GL_ATOMIC_COUNTER_BARRIER_BIT | 
                  GL_SHADER_STORAGE_BARRIER_BIT);

  glUseProgram(clear_program);
  glBindVertexArray(dummy_vao);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glUseProgram(append_program);

  vmath::mat4 model_matrix = vmath::scale(7.0f);
  vmath::vec3 view_position = vmath::vec3(cosf(f * 0.35f) * 120.0f, 
                                          cosf(f*0.4f) * 30.0f, 
                                          sinf(f * 0.35f) * 120.0f);
  vmath::mat4 view_matrix = vmath::lookat(view_position, 
                                          vmath::vec3(0.0f, 30.0f, 0.0f),
                                          vmath::vec3(0.0f, 1.0f, 0.0f));

  vmath::mat4 mv_matrix = view_matrix * model_matrix;
  vmath::mat4 proj_matrix = vmath::perspective(50.0f, 
                                               (float)info.windowWidth / 
                                               (float)info.windowHeight,
                                               0.1f, 1000.0f);

  glUniformMatrix4fv(uniforms.mvp, 1, GL_FALSE, proj_matrix * mv_matrix);

  static const unsigned int zero = 0;
  glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomic_counter_buffer);
  glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(zero), &zero);

  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, fragment_buffer);

  glBindImageTexture(0, head_pointer_image, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
  
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | 
                  GL_ATOMIC_COUNTER_BARRIER_BIT | 
                  GL_SHADER_STORAGE_BARRIER_BIT);

  object.render();
  
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | 
                  GL_ATOMIC_COUNTER_BARRIER_BIT | 
                  GL_SHADER_STORAGE_BARRIER_BIT);

  glUseProgram(resolve_program);
  glBindVertexArray(dummy_vao);

  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | 
                  GL_ATOMIC_COUNTER_BARRIER_BIT | 
                  GL_SHADER_STORAGE_BARRIER_BIT);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

GLuint LoadShader(std::string const &filename, GLuint shader_type)
{
  GLuint result_shader;

  FILE* fp;
  size_t filesize;
  char* data;

  fp = fopen(filename.data(), "rb");

  if (!fp)
  {
    return 0;
  }

  fseek(fp, 0, SEEK_END);
  filesize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  data = new char[filesize + 1];

  if (!data)
  {
    return 0;
  }

  fread(data, 1, filesize, fp);
  data[filesize] = 0;
  fclose(fp);

  result_shader = glCreateShader(shader_type);

  glShaderSource(result_shader, 1, &data, nullptr);
  delete[] data;
  glCompileShader(result_shader);

  char buffer[4096];
  glGetShaderInfoLog(result_shader, 4096, nullptr, buffer);

  OutputDebugStringA(filename.data());
  OutputDebugStringA(":");
  OutputDebugStringA(buffer);
  OutputDebugStringA("\n");

  return result_shader;
}

void FragmentList::load_shaders()
{
  char buffer[4096];
  GLuint shaders[2];

  shaders[0] = LoadShader("clear.vs.glsl", GL_VERTEX_SHADER);
  shaders[1] = LoadShader("clear.fs.glsl", GL_FRAGMENT_SHADER);

  if (clear_program)
  {
    glDeleteProgram(clear_program);
  }

  clear_program = glCreateProgram();

  for (int i = 0; i < 2; ++i)
  {
    glAttachShader(clear_program, shaders[i]);
  }

  glLinkProgram(clear_program);

  glGetProgramInfoLog(clear_program, 4096, nullptr, buffer);

  OutputDebugStringA(buffer);
  OutputDebugStringA("\n");


  shaders[0] = LoadShader("append.vs.glsl", GL_VERTEX_SHADER);
  shaders[1] = LoadShader("append.fs.glsl", GL_FRAGMENT_SHADER);

  if (append_program)
  {
    glDeleteProgram(append_program);
  }

  append_program = glCreateProgram();

  for (int i = 0; i < 2; ++i)
  {
    glAttachShader(append_program, shaders[i]);
  }

  glLinkProgram(append_program);

  glGetProgramInfoLog(append_program, 4096, nullptr, buffer);

  OutputDebugStringA(buffer);
  OutputDebugStringA("\n");


  shaders[0] = LoadShader("resolve.vs.glsl", GL_VERTEX_SHADER);
  shaders[1] = LoadShader("resolve.fs.glsl", GL_FRAGMENT_SHADER);

  if (resolve_program)
  {
    glDeleteProgram(resolve_program);
  }

  resolve_program = glCreateProgram();

  for (int i = 0; i < 2; ++i)
  {
    glAttachShader(resolve_program, shaders[i]);
  }

  glLinkProgram(resolve_program);

  glGetProgramInfoLog(resolve_program, 4096, nullptr, buffer);

  OutputDebugStringA(buffer);
  OutputDebugStringA("\n");
}

void FragmentList::onKey(int key, int action)
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

DECLARE_MAIN(FragmentList)