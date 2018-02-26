#include <sb7.h>
#include <shader.h>
#include <vmath.h>
#include <sb7color.h>
#include <object.h>

class TextureLevels : public sb7::application
{
public:
  virtual void startup() override;
  virtual void render(double currentTime) override;
  virtual void shutdown() override;
  virtual void onKey(int key, int action) override;

protected:
  void load_shaders();

  enum 
  {
    NUM_TEXTURES = 384,
    TEXTURE_LEVELS = 5,
    TEXTURE_SIZE = (1 << (TEXTURE_LEVELS - 1))
  };

  GLuint program;

  struct 
  {
    GLuint mv_matrix;
    GLuint vp_matrix;
  } uniforms;

  struct
  {
    GLuint name;
    GLuint64 handle;
  } textures[1024];

  struct 
  {
    GLuint transformBuffer;
    GLuint textureHandleBuffer;
  } buffers;

  struct MATRICES
  {
    vmath::mat4 view;
    vmath::mat4 projection;
    vmath::mat4 model[NUM_TEXTURES];
  };

  sb7::object object;
};

static unsigned int seed = 0x13371337;

static inline unsigned int random_uint()
{
  float res;
  unsigned int tmp;

  seed *= 16807;

  tmp = seed ^ (seed >> 4) ^ (seed << 15);

  return tmp;
}

void TextureLevels::startup()
{
  int i; 
  int j;

  unsigned char tex_data[32 * 32 * 4];
  unsigned int mutated_data[32 * 32];
  memset(tex_data, 0, sizeof(tex_data));

  for (i = 0; i < 32; ++i)
  {
    for (j = 0; j < 32; ++j)
    {
      tex_data[i * 4 * 32 + j * 4] = (i^j) << 3;
      tex_data[i * 4 * 32 + j * 4 + 1] = (i^j) << 3;
      tex_data[i * 4 * 32 + j * 4 + 2] = (i^j) << 3;
    }
  }

  glGenBuffers(1, &buffers.transformBuffer);
  glBindBuffer(GL_UNIFORM_BUFFER, buffers.transformBuffer);

  glBufferStorage(GL_UNIFORM_BUFFER,
                  sizeof(MATRICES), 
                  nullptr, 
                  GL_MAP_WRITE_BIT);

  glGenBuffers(1, &buffers.textureHandleBuffer);
  glBindBuffer(GL_UNIFORM_BUFFER, buffers.textureHandleBuffer);

  glBufferStorage(GL_UNIFORM_BUFFER,
                  NUM_TEXTURES * sizeof(GLuint64) * 2,
                  nullptr,
                  GL_MAP_WRITE_BIT);

  GLuint64 *pHandles = 
      (GLuint64*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, 
                                  NUM_TEXTURES * sizeof(GLuint64) * 2, 
                                  GL_MAP_WRITE_BIT | 
                                  GL_MAP_INVALIDATE_BUFFER_BIT);

  for (i = 0; i < NUM_TEXTURES; ++i)
  {
    unsigned int r = (random_uint() & 0xFCFF3F) << (random_uint() % 12);

    glGenTextures(1, &textures[i].name);
    glBindTexture(GL_TEXTURE_2D)
  }
}

DECLARE_MAIN(TextureLevels)