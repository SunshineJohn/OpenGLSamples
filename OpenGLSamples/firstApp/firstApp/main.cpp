#include "sb7.h"

#define GLSL(version, shader) "#version " #version "\n" #shader                


static const GLchar* vertex_shader_source = GLSL
(
  450 core,
  //layout (location = 0) in vec4 offset;

  void main()
  {
    const vec4 vertices[3] = vec4[3](vec4(0.25, -0.25, 0.5, 1.0),
                                     vec4(-0.25, -0.25, 0.5, 1.0),
                                     vec4(0.25, 0.25, 0.5, 1.0));

    gl_Position = vertices[gl_VertexID]; //+ offset;
  }
);

static const GLchar* fragment_shader_source = GLSL
(
  450 core,

  out vec4 color;
  void main()
  {
    color = vec4(0.0f, 0.8f, 1.0f, 1.0f);
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

class my_application : public sb7::application
{
public:
  void startup() override
  {
    rendering_program = compile_shaders();
    glGenVertexArrays(1, &vertex_array_object);
    glBindVertexArray(vertex_array_object);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }

  void shutdown() override
  {
    glDeleteVertexArrays(1, &vertex_array_object);
    glDeleteProgram(rendering_program);
  }

	void render(double current_time) override
	{
    GLfloat color[] = { 0.5f, 0.5f,
                        0.0f, 1.0f };

		glClearBufferfv(GL_COLOR, 0, color);
    
    glUseProgram(rendering_program);

   /* GLfloat attrib[] = { (float)sin(current_time)*0.5f,
                         (float)cos(current_time)*0.5f,
                          0.0f, 0.0f };

    glVertexAttrib4fv(0, attrib);*/

    glPointSize(5.0f);
    
    glDrawArrays(GL_PATCHES, 0, 3);
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

    control_shader = glCreateShader(GL_TESS_CONTROL_SHADER);
    glShaderSource(control_shader, 1, &tess_control_source, nullptr);
    glCompileShader(control_shader);

    evaluation_shader = glCreateShader(GL_TESS_EVALUATION_SHADER);
    glShaderSource(evaluation_shader, 1, &tess_evaluation_source, nullptr);
    glCompileShader(evaluation_shader);

    geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometry_shader, 1, &geometry_shader_source, nullptr);
    glCompileShader(geometry_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
    glCompileShader(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, control_shader);
    glAttachShader(program, evaluation_shader);
    glAttachShader(program, geometry_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glDeleteShader(vertex_shader);
    glDeleteShader(control_shader);
    glDeleteShader(evaluation_shader);
    glDeleteShader(geometry_shader);
    glDeleteShader(fragment_shader);

    return program;
  }

private:
  GLuint rendering_program;
  GLuint vertex_array_object;
};

DECLARE_MAIN(my_application);