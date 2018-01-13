#include "sb7.h"

#define GLSL(version, shader) "#version " #version "\n" #shader                


static const GLchar* vertex_shader_source = GLSL
(
  450 core,
  layout (location = 0) in vec4 offset;
  layout(location = 1) in vec4 color;

  out VS_OUT
  {
    vec4 color;
  } vs_out;

  void main()
  {
    const vec4 vertices[3] = vec4[3](vec4(0.25, -0.25, 0.5, 1.0),
                                     vec4(-0.25, -0.25, 0.5, 1.0),
                                     vec4(0.25, 0.25, 0.5, 1.0));

    gl_Position = vertices[gl_VertexID] + offset;

    vs_out.color = color;
  }
);

static const GLchar* fragment_shader_source = GLSL
(
  450 core,
  in VS_OUT
  {
    vec4 color;
  } fs_in;

  out vec4 color;
  void main()
  {
    color = fs_in.color;
  }
);

class my_application : public sb7::application
{
public:
  void startup() override
  {
    rendering_program = compile_shaders();
    glCreateVertexArrays(1, &vertex_array_object);
    glBindVertexArray(vertex_array_object);
  }

  void shutdown() override
  {
    glDeleteVertexArrays(1, &vertex_array_object);
    glDeleteProgram(rendering_program);
    glDeleteVertexArrays(1, &vertex_array_object);
  }

	void render(double current_time) override
	{
    GLfloat color[] = { 0.5f, 0.5f,
                        0.0f, 1.0f };

		glClearBufferfv(GL_COLOR, 0, color);
    
    glUseProgram(rendering_program);

    GLfloat attrib[] = { (float)sin(current_time)*0.5f,
                         (float)cos(current_time)*0.5f,
                          0.0f, 0.0f };

    glVertexAttrib4fv(0, attrib);

    GLfloat in_color[] = { (float)sin(current_time)*0.5f + 0.5f,
                           (float)cos(current_time)*0.5f + 0.5f,
                            0.0f, 1.0f };

    glVertexAttrib4fv(1, in_color);

    glDrawArrays(GL_TRIANGLES, 0, 3);
	}

  GLuint compile_shaders(void)
  {
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint program;

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
    glCompileShader(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
    glCompileShader(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
  }

private:
  GLuint rendering_program;
  GLuint vertex_array_object;
};

DECLARE_MAIN(my_application);