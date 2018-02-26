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

DECLARE_MAIN(TextureLevels)