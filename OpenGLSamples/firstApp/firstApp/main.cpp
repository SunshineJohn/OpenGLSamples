#include "sb7.h"
#include "vmath.h"

#define GLSL(version, shader) "#version " #version "\n" #shader                


static const GLchar* vertex_shader_source = GLSL
(
  450 core,
  layout (location = 0) in vec4 position;
  layout (location = 1) in vec4 color;

  out vec4 vs_color;

  void main()
  {
    gl_Position = position/*vertices[gl_VertexID] + offset*/;

    vs_color = color;
  }
);

static const GLchar* fragment_shader_source = GLSL
(
  450 core,

  in vec4 vs_color;

  out vec4 color;
  void main()
  {
    color = vs_color;
  }
);

static const GLchar* tess_control_source = GLSL
(
  450 core,
  layout(vertices = 3) out;

  void main(void)
  {
    if (gl_InvocationID == 0)
    {
      gl_TessLevelInner[0] = 5.0;
      gl_TessLevelOuter[0] = 5.0;
      gl_TessLevelOuter[1] = 5.0;
      gl_TessLevelOuter[2] = 5.0;
    }

    gl_out[gl_InvocationID].gl_Position =
      gl_in[gl_InvocationID].gl_Position;
  }

);

static const GLchar* tess_evaluation_source = GLSL
(
  450 core,
  layout (triangles, equal_spacing, cw) in;

  void main(void)
  {
    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position +
                   gl_TessCoord.y * gl_in[1].gl_Position +
                   gl_TessCoord.z * gl_in[2].gl_Position);
  }
);

static const GLchar* geometry_shader_source = GLSL
(
  450 core,

  layout (triangles) in;
  layout(points, max_vertices = 3) out;

  void main(void)
  {
    int i = 0;

    for (i; i < gl_in.length(); ++i)
    {
      gl_Position = gl_in[i].gl_Position;
      EmitVertex();
    }
  }
);

static const GLchar* compute_shader_source = GLSL
(
  450 core,

  layout (local_size_x = 32, local_size_y = 32) in;

  void main(void)
  {
    
  }
);

class my_application : public sb7::application
{
public:
  void startup() override
  {
    rendering_program = compile_shaders();
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    static const float data[] = { 0.25, -0.25, 0.5, 1.0,
                                  -0.25, -0.25, 0.5, 1.0,
                                  0.25, 0.25, 0.5, 1.0 };

    glCreateBuffers(2, &buffers[0]);
    glNamedBufferStorage(buffers[0], sizeof(data), data, 0);

    glVertexArrayVertexBuffer(vao, 0, buffers[0], 0, sizeof(vmath::vec4));
    glVertexArrayAttribFormat(vao, 0, 4, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao, 0, 0);

    glEnableVertexArrayAttrib(vao, 0);

    // glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), data);

    static const float color_data[] = { 1.0f, 0.0f, 0.0f, 1.0f,
                                        0.0f, 1.0f, 0.0f, 1.0f,
                                        0.0f, 0.0f, 1.0f, 1.0f };

    glNamedBufferStorage(buffers[1], sizeof(color_data), color_data, 0);
    glVertexArrayVertexBuffer(vao, 1, buffers[1], 0, sizeof(vmath::vec4));
    glVertexArrayAttribFormat(vao, 1, 4, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao, 1, 1);
    glEnableVertexAttribArray(1);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }

  void shutdown() override
  {
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(rendering_program);
  }

	void render(double current_time) override
	{
    GLfloat color[] = { 0.5f, 0.5f,
                        0.0f, 1.0f };

		glClearBufferfv(GL_COLOR, 0, color);
    
    glUseProgram(rendering_program);

    /*GLfloat attrib[] = { (float)sin(current_time)*0.5f,
                         (float)cos(current_time)*0.5f,
                          0.0f, 0.0f };

    glVertexAttrib4fv(0, attrib);*/

    //glPointSize(5.0f);
    
    glDrawArrays(GL_TRIANGLES, 0, 3);
	}

  GLuint compile_shaders(void)
  {
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint control_shader;
    GLuint evaluation_shader;
    GLuint geometry_shader;
    GLuint program;

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
    glCompileShader(vertex_shader);

    //control_shader = glCreateShader(GL_TESS_CONTROL_SHADER);
    //glShaderSource(control_shader, 1, &tess_control_source, nullptr);
    //glCompileShader(control_shader);

    //evaluation_shader = glCreateShader(GL_TESS_EVALUATION_SHADER);
    //glShaderSource(evaluation_shader, 1, &tess_evaluation_source, nullptr);
    //glCompileShader(evaluation_shader);

    //geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
    //glShaderSource(geometry_shader, 1, &geometry_shader_source, nullptr);
    //glCompileShader(geometry_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
    glCompileShader(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    //glAttachShader(program, control_shader);
    //glAttachShader(program, evaluation_shader);
    //glAttachShader(program, geometry_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glDeleteShader(vertex_shader);
    //glDeleteShader(control_shader);
    //glDeleteShader(evaluation_shader);
    //glDeleteShader(geometry_shader);
    glDeleteShader(fragment_shader);

    return program;
  }

private:
  GLuint rendering_program;
  GLuint vao;
  GLuint buffers[2];
};

DECLARE_MAIN(my_application);