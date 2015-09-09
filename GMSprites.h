#ifndef _GM_SPRITES_H_
#define _GM_SPRITES_H_

#include "GMLib.h"
#include "GMTexture.h"

/* Sprites Sheet */

class sprites_sheet : public texture {
public:
  sprites_sheet(const std::string & res, uint32_t sprite_w, uint32_t sprite_h);
  sprites_sheet();

  rect get_sprite_cliprect(size_t idx);
  uint32_t sprite_width() { return width() / _cols; }
  uint32_t sprite_height() { return height() / _rows; }
  uint32_t rows() { return _rows; }
  uint32_t cols() { return _cols; }

private:
  uint32_t _cols;
  uint32_t _rows;
};

/* Sprite */

class sprite {
public:
  /* Sprite constants */
  static const size_t invalid = _UI32_MAX; // invalid sprite index 
  static const uint8_t alpha_full = 255;   // alpha modifiers
  static const uint8_t alpha_half = 127;
  static const uint8_t alpha_none = 0;

  /* sprite index in sheet and size */
  size_t idx;
  uint32_t w;
  uint32_t h;

  /* rotation info */
  SDL_RendererFlip flip;
  int angle;

  inline bool valid() { return idx != invalid; }
  inline bool operator== (sprite& other) {
      return (idx == other.idx && w == other.w && h == other.h && flip == other.flip && angle == other.angle && _sheet == other._sheet);
  }
  inline bool operator!= (sprite& other) {
      return !(*this == other);
  }

  /* create new empty sprite */
  sprite();

  /* create new sprite with texture index */
  sprite(size_t tex_idx, int px_w, int px_h, sprites_sheet* sheet);
  virtual ~sprite() {}

  /* get center point based on rect */
  SDL_Point center();

  /* render sprite with sheet at screen position rect */
  virtual void render(SDL_Renderer * r, point & topleft, uint8_t alpha = sprite::alpha_full) const;
  virtual void render(SDL_Renderer * r, rect & dst, uint8_t alpha = sprite::alpha_full) const;

protected:
  /* sprites sheet*/
  sprites_sheet* _sheet;
};

/*
    Animations
*/

/* SDL_Timer-based animation index */
class anim {
public:
    /* Animation consts */
    static const uint32_t once     = 0;
    static const uint32_t repeat   = 1;
    static const uint32_t occilate = 2;

    /* list of running animations */
    static locked_vector<anim*> running;
    /* animate running items */
    static void update_running();

    /* Sprite(s) to animate */
    sprite** target;
    size_t targets_count;

    /* Animation item task properties */
    bool is_running;
    uint32_t mode;
    uint32_t period_ms;
    uint32_t last_updated;
    uint32_t repeats; // 0 = repeat forever if mode != once

    inline bool operator== (anim other) {
        return (target == other.target && period_ms == other.period_ms && from == other.from && to == other.to && step == other.step && base == other.base);
    }
    inline bool operator!= (anim other) {
        return !(*this == other);
    }

    /* create new animation task */
    anim(sprite* s, size_t _from, size_t _to, size_t _step, uint32_t _period_ms, uint32_t _mode);
    anim(sprite** ss, size_t _count, size_t _base, size_t _from, size_t _to, size_t _step, uint32_t _period_ms, uint32_t _mode);

    virtual ~anim() {};
    void release_targets();
    
    /* start/stop animation task */
    void start(uint32_t repeat = 0);
    void stop();

    /* animate this item */
    void update();

private:
  void init(sprite** s, size_t _count, size_t _base, size_t _from, size_t _to, size_t _step, uint32_t _period_ms, uint32_t _mode);
  void reset();

  /* Animated frame info */
  size_t base;
  size_t step;
  int from;
  int to;
  int current; //<current> = base + slide_index. slide_index => [from .. to] 
  int modifier; //animation direction. <next> = slide_index + modifier. modifier => [-1; 1]
};

#endif //_GM_SPRITES_H_