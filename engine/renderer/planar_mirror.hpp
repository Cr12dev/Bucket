#pragma once

// FBO that holds the reflected scene for a planar mirror (the floor at
// y = 0). The game renders the scene from the reflected camera into this
// texture, then surfaces with `paint::reflectivity > 0` sample it in the
// lighting shader.
class planar_mirror {
public:
  void init();
  void resize(int w, int h);

  void begin();  // binds FBO and clears
  void end();    // unbinds

  void bind_color(int unit) const;

  int width() const { return width_; }
  int height() const { return height_; }
  bool ready() const { return fbo_ != 0; }

private:
  unsigned int fbo_ = 0;
  unsigned int color_tex_ = 0;
  unsigned int depth_rbo_ = 0;
  int width_ = 0;
  int height_ = 0;
};
