#ifndef _GM_ANIMATION_H_
#define _GM_ANIMATION_H_

#include "GMLib.h"
#include "GMSprites.h"
#include "GMData.h"
#include "GMEventHandler.h"

/* basic timer-based animation */
class anim {
public:

  // animation event details
  struct event {
    anim * target;
    event(anim * a):target(a) {}
  };
  typedef ::event_handler<event &> anim_event_handler;

  /* Animation consts */
  typedef enum {
    once   = 0,
    repeat = 1,
    occilate = 2
  } anim_mode;

  inline bool operator== (anim & other) {
    return (_sheet == other._sheet && _mode == other._mode && _frame_duration == other._frame_duration);
  }
  inline bool operator!= (anim & other) {
    return !(*this == other);
  }

  // create empty animation
  anim();

  // create new animation
  anim(const sprites_sheet * sheet,
       const anim_mode mode,
       unsigned int frame_duration);

  anim(const sprites_sheet * sheet,
       const anim_mode mode,
       unsigned int frame_duration,
       int from,
       int to);
  
  // create new animation with sprite sheet from resources
  anim(const std::string & sprites_sheet_resource,
       int sprite_w, int sprite_h,
       const anim_mode mode,
       unsigned int frame_duration);

  anim(const std::string & sprites_sheet_resource,
       int sprite_w, int sprite_h,
       const anim_mode mode,
       unsigned int frame_duration,
       int from,
       int to);

  // animation mode
  const anim_mode mode() const { return _mode; }

  // animation direction modifier [-1, 1]
  const int modifier() const { return _mod; }
  const int from() const { return _from; }
  const int to() const { return _to; }

  // reset animation from->to with mod
  void reset(int from, int to);

  // get a number of miliseconds each animation frame should be displayed
  const unsigned int frame_duration() const { return _frame_duration; }

  // check animation status
  bool is_running() const { return _timer != 0; }

  // get animation size
  int width() const { return _sheet->sprite_width(); }
  int height() const { return _sheet->sprite_height(); }

  // start & stop this animation
  void start();
  void stop(bool reset_current = true);

  // set current frame index
  void set_current(int idx) { _current = idx; }

  // total number of animation frames for a sprites sheet
  int total_frames() const { return _sheet->rows() * _sheet->cols(); }
  
  // get current frame
  int current() const { return _current; }

  // get sprite corresponding to current frame
  sprite current_sprite() const { return sprite(_current, width(), height(), _sheet); }
  
  // get sprites_sheet powering this animation
  const sprites_sheet * sheet() const { return _sheet; }

  /* Animation events */
  anim_event_handler started;
  anim_event_handler stopped;

private:
  
  // animation details
  SDL_TimerID _timer;
  anim_mode _mode;
  const sprites_sheet * _sheet;
  unsigned int _frame_duration;
  
  // animation direction information
  int _from;
  int _to;
  int _mod;
  int _current;
};


#endif //_GM_ANIMATION_H_