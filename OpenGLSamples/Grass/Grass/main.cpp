#include <sb7.h>
#include <vmath.h>
#include <sb7ktx.h>

#include <string>

#define GLSL(version, shader) "#version " #version "\n" #shader  

static void PrintShaderLog(GLuint shader)
{
    std::string str;
    GLint len;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    str.resize(len);
    glGetShaderInfoLog(shader, len, NULL, &str[0]);

#ifdef _WIN32
    OutputDebugStringA(str.c_str());
#endif
}

static const GLchar* vertex_shader_source = GLSL
(
  450 core,

);

static const GLchar* fragment_shader_source = GLSL
(
  450 core,

);

class Grass : public sb7::application
{
public:
  void startup(void) override
  {
    static const GLfloat grass_blade[] =
    {
       -0.3f, 0.0f,
        0.3f, 0.0f,
       -0.2f, 1.0f,
        0.1f, 1.3f,
      -0.05f, 2.3f,
        0.0f, 3.3f
    };

    glGenBuffers(1, &grass_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, grass_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(grass_blade), 
                 grass_blade, GL_STATIC_DRAW);

    glGenVertexArrays(1, &grass_vao);
    glBindVertexArray(grass_vao);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    grass_program = glCreateProgram();
    GLuint grass_vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint grass_fs = glCreateShader(GL_FRAGMENT_SHADER);

    FILE *fp = fopen("grass.vs.glsl", "rb");
    if (!fp)
    {
      return;
    }

    fseek(fp, 0, SEEK_END);
    size_t filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *data = new char[filesize + 1];

    fread(data, 1, filesize, fp);
    data[filesize] = 0;
    fclose(fp);

    glShaderSource(grass_vs, 1, &data, nullptr);
    delete []data;

    fp = fopen("grass.fs.glsl", "rb");
    if (!fp)
    {
      return;
    }

    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    data = new char[filesize + 1];

    fread(data, 1, filesize, fp);
    data[filesize] = 0;
    fclose(fp);
    glShaderSource(grass_fs, 1, &data, nullptr);

    glCompileShader(grass_vs);
    PrintShaderLog(grass_vs);
    glCompileShader(grass_fs);
    PrintShaderLog(grass_fs);

    glAttachShader(grass_program, grass_vs);
    glAttachShader(grass_program, grass_fs);

    glLinkProgram(grass_program);
    glDeleteShader(grass_fs);
    glDeleteShader(grass_vs);

    uniforms.mvp_matrix = glGetUniformLocation(grass_program, "mvpMatrix");

    glActiveTexture(GL_TEXTURE1);
    tex_grass_length = 
        sb7::ktx::file::load("../../../media/textures/grass_length.ktx");
    glActiveTexture(GL_TEXTURE2);
    tex_grass_orientation = 
        sb7::ktx::file::load("../../../media/textures/grass_orientation.ktx");
    glActiveTexture(GL_TEXTURE3);
    tex_grass_color = 
        sb7::ktx::file::load("../../../media/textures/grass_color.ktx");
    glActiveTexture(GL_TEXTURE4);
    tex_grass_bend = 
        sb7::ktx::file::load("../../../media/textures/grass_bend.ktx");
  }

  void shutdown(void) override
  {
    glDeleteProgram(grass_program);
  }

  void render(double current_time) override
  {
    float t = (float)current_time * 0.02f;
    float r = 550.0f;

    static const GLfloat black[] = {0.0f, 0.0f, 0.0f, 1.0f};
    static const GLfloat one = 1.0f;
    glClearBufferfv(GL_COLOR, 0, black);
    glClearBufferfv(GL_DEPTH, 0, &one);

    vmath::mat4 mv_matrix = 
        vmath::lookat(vmath::vec3(sinf(t) * r, 25.0f, cosf(t) * r),
                      vmath::vec3(0.0f, -50.0f, 0.0f),
                      vmath::vec3(0.0f, 1.0f, 0.0f));

    vmath::mat4 prj_matrix = vmath::perspective(45.0f, 
                                                (float)info.windowWidth / 
                                                (float)info.windowHeight,
                                                0.1f, 1000.0f); 

    glUseProgram(grass_program);

    glUniformMatrix4fv(uniforms.mvp_matrix, 1, 
                       GL_FALSE, (prj_matrix * mv_matrix));

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glViewport(0,0,info.windowWidth, info.windowHeight);

    glBindVertexArray(grass_vao);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0,6, 1024 * 1024);
  }

private:
  GLuint grass_buffer;
  GLuint grass_vao;

  GLuint grass_program;

  GLuint tex_grass_color;
  GLuint tex_grass_length;
  GLuint tex_grass_orientation;
  GLuint tex_grass_bend;

  struct
  {
    GLuint mvp_matrix;
  } uniforms;
};

DECLARE_MAIN(Grass);