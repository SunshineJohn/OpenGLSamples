#include <sb7.h>
#include <sb7ktx.h>
#include <vmath.h>

#define GLSL(version, shader) "#version " #version "\n" #shader  

static const GLchar* vertex_shader_source = GLSL
(
  450 core,
  layout (location = 0) in int alien_index;

  out VS_OUT
  {
    flat int alien;
    vec2 tc;
  } vs_out;

  struct droplet_t
  {
    float x_offset;
    float y_offset;
    float orientation;
    float unused;
  };

  layout(std140) uniform droplets
  {
    droplet_t droplet[256];
  };

  void main(void)
  {
    const vec2[4] position = vec2[4](vec2(-0.5, -0.5),
                                     vec2( 0.5, -0.5), 
                                     vec2(-0.5,  0.5), 
                                     vec2( 0.5,  0.5));

    vs_out.tc = position[gl_VertexID].xy + vec2(0.5);
    float co = cos(droplet[alien_index].orientation);
    float si = sin(droplet[alien_index].orientation);

    mat2 rot = mat2(vec2(co, si),
                    vec2(-si, co));
    vec2 pos = 0.25*rot*position[gl_VertexID];
    gl_Position = vec4(pos.x + droplet[alien_index].x_offset,
                       pos.y + droplet[alien_index].y_offset, 
                       0.5, 1.0);
    vs_out.alien = alien_index % 64;
  }
);

static const GLchar* fragment_shader_source = GLSL
(
  450 core,
  layout(location = 0) out vec4 color;
  in VS_OUT
  {
    flat int alien;
    vec2 tc;
  } fs_in;

  layout (binding = 0) uniform sampler2DArray tex_aliens;

  void main(void)
  {
    color = texture(tex_aliens, vec3(fs_in.tc, float(fs_in.alien)));
  }
);

// Random number generator
static unsigned int seed = 0x13371337;

static inline float random_float()
{
  float res;
  unsigned int tmp;

  seed *= 16807;

  tmp = seed ^ (seed >> 4) ^ (seed << 15);

  *((unsigned int *)&res) = (tmp >> 9) | 0x3F800000;

  return (res - 1.0f);
}


class AlienRain : public sb7::application
{
public:
  virtual void startup() override
  {
    load_shaders();

    glGenVertexArrays(1, &render_vao);
    glBindVertexArray(render_vao);

    tex_alien_array = 
        sb7::ktx::file::load("../../../media/textures/aliens.ktx");
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex_alien_array);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, 
                    GL_LINEAR_MIPMAP_LINEAR);

    glGenBuffers(1, &rain_buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, rain_buffer);
    glBufferData(GL_UNIFORM_BUFFER, 256 * sizeof(vmath::vec4), 
                 NULL, GL_DYNAMIC_DRAW);

    for (int i = 0; i < 256; ++i) 
    {
      droplet_x_offset[i] = random_float() * 2.0f - 1.0f;
      droplet_rot_speed[i] = (random_float() + 0.5f)*((i & 1) ? -3.0f : 3.0f);
      droplet_fall_speed[i] = random_float() + 0.2f;
    }

    glBindVertexArray(render_vao);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  virtual void render(double current_time) override
  {
    static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float t = (float)current_time;

    glViewport(0, 0, info.windowWidth, info.windowHeight);
    glClearBufferfv(GL_COLOR, 0, black);

    glUseProgram(render_prog);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, rain_buffer);
    vmath::vec4 *droplet = 
        (vmath::vec4*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, 
                                       256 * sizeof(vmath::vec4), 
                                       GL_MAP_WRITE_BIT | 
                                       GL_MAP_INVALIDATE_BUFFER_BIT);

    for (int i = 0; i < 256; ++i)
    {
      droplet[i][0] = droplet_x_offset[i];
      droplet[i][1] = 2.0f - fmodf((t + float(i))*droplet_fall_speed[i], 4.31f);
      droplet[i][2] = t*droplet_rot_speed[i];
      droplet[i][3] = 0.0f;
    }
    glUnmapBuffer(GL_UNIFORM_BUFFER);

    for (int alien_index = 0; alien_index < 256; ++alien_index) 
    {
      glVertexAttribI1i(0, alien_index);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
  }

  void load_shaders()
  {
    char buffer[1024];

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
    glCompileShader(vertex_shader);

    glGetShaderInfoLog(vertex_shader, 1024, nullptr, buffer);

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
    glCompileShader(fragment_shader);

    glGetShaderInfoLog(fragment_shader, 1024, nullptr, buffer);

    render_prog = glCreateProgram();
    glAttachShader(render_prog, vertex_shader);
    glAttachShader(render_prog, fragment_shader);

    glGetProgramInfoLog(render_prog, 1024, nullptr, buffer);

    glLinkProgram(render_prog);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
  }
protected:
  GLuint render_prog;
  GLuint render_vao;

  GLuint tex_alien_array;
  GLuint rain_buffer;

  float droplet_x_offset[256];
  float droplet_rot_speed[256];
  float droplet_fall_speed[256];
};

DECLARE_MAIN(AlienRain)