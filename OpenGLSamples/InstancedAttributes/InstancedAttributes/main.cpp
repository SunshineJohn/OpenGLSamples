#include <sb7.h>

#include <vector>

class InstancedAttributes : public sb7::application
{
public:
  void startup() override
  {
    static const GLfloat square_vertices[] =
    {
      -1.0f, -1.0f, 0.0f, 1.0f,
       1.0f, -1.0f, 0.0f, 1.0f,
       1.0f,  1.0f, 0.0f, 1.0f,
      -1.0f,  1.0f, 0.0f, 1.0f
    };

    static const GLfloat square_colors[] =
    {
      1.0f, 0.0f, 0.0f, 1.0f,
      0.0f, 1.0f, 0.0f, 1.0f,
      0.0f, 0.0f, 1.0f, 1.0f,
      1.0f, 1.0f, 0.0f, 1.0f
    };

    static const GLfloat instance_positions[] =
    {
      -2.0f, -2.0f, 0.0f, 0.0f,
       2.0f, -2.0f, 0.0f, 0.0f,
       2.0f,  2.0f, 0.0f, 0.0f,
      -2.0f,  2.0f, 0.0f, 0.0f
    };

    GLuint offset = 0;

    glGenVertexArrays(1, &square_vao);
    glGenBuffers(1, &square_buffer);
    glBindVertexArray(square_vao);
    glBindBuffer(GL_ARRAY_BUFFER, square_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square_vertices) + 
                                  sizeof(square_colors) + 
                                  sizeof(instance_positions), 
                 nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, offset, 
                    sizeof(square_vertices), square_vertices);
    offset += sizeof(square_vertices);
    glBufferSubData(GL_ARRAY_BUFFER, offset, 
                    sizeof(square_colors), square_colors);
    offset += sizeof(square_colors);
    glBufferSubData(GL_ARRAY_BUFFER, offset, 
                    sizeof(instance_positions), instance_positions);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 
                          0, (GLvoid *)sizeof(square_vertices));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, 
        (GLvoid *)(sizeof(square_vertices) + sizeof(square_colors)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);

    square_program = glCreateProgram();

    //load shader
    char *data = nullptr;
    FILE *fp = nullptr;
    size_t filesize = 0;

    fp = fopen("Instanced.vs.glsl", "rb");
    if (!fp)
    {
      return;
    }

    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    data = new char[filesize +1];

    if (!data)
    {
      return;
    }

    fread(data, 1, filesize, fp);
    data[filesize] = 0;
    fclose(fp);

    GLuint square_vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(square_vs, 1, &data, nullptr);
    delete []data;
    glAttachShader(square_program, square_vs);

    fp = fopen("Instanced.fs.glsl", "rb");
    if (!fp)
    {
      return;
    }

    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    data = new char[filesize +1];

    if (!data)
    {
      return;
    }

    fread(data, 1, filesize, fp);
    data[filesize] = 0;
    fclose(fp);

    GLuint square_fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(square_fs, 1, &data, nullptr);
    delete []data;
    glAttachShader(square_program, square_fs);

    glLinkProgram(square_program);
    glDeleteShader(square_vs);
    glDeleteShader(square_fs);
  }

  void shutdown() override
  {
    glDeleteProgram(square_program);
    glDeleteBuffers(1, &square_buffer);
    glDeleteVertexArrays(1, &square_vao);
  }

  void render(double current_time) override
  {
    static const GLfloat black[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, black);

    glUseProgram(square_program);
    glBindVertexArray(square_vao);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 4);
  }

private:
  GLuint square_buffer;
  GLuint square_vao;

  GLuint square_program;
};

DECLARE_MAIN(InstancedAttributes);