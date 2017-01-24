#include "GMAnimation.h"
#include "RESLib.h"

// create new animation
anim::anim(const sprites_sheet * sheet,
           const anim_mode mode,
           unsigned int frame_duration):
  _timer(0),
  _mode(mode),
  _sheet(sheet),
  _frame_duration(frame_duration),
  _from(0),
  _to(_sheet->rows() * _sheet->cols() - 1),
  _mod( _to > _from ? -1 : 1),
  _current(_from)
{
}


anim::anim(const sprites_sheet * sheet,
           const anim_mode mode,
           unsigned int frame_duration,
           int from,
           int to):
  _timer(0),
  _mode(mode),
  _sheet(sheet),
  _frame_duration(frame_duration),
  _from(from),
  _to(to),
  _mod( _to > _from ? -1 : 1),
  _current(_from)
{
}
  
// load animation from resource data
anim::anim(const std::string & sprites_sheet_resource,
           int sprite_w, int sprite_h,
           const anim_mode mode,
           unsigned int frame_duration):
  _timer(0),
  _mode(mode),
  _sheet(resources::get_sprites_sheet(sprites_sheet_resource, sprite_w, sprite_h)),
  _frame_duration(frame_duration),
  _from(0),
  _to(_sheet->rows() * _sheet->cols() - 1),
  _mod( _to > _from ? 1 : -1),
  _current(_from)
{
}

 anim::anim(const std::string & sprites_sheet_resource,
           int sprite_w, int sprite_h,
           const anim_mode mode,
           unsigned int frame_duration,
           int from,
           int to):
  _mode(mode),
  _sheet(resources::get_sprites_sheet(sprites_sheet_resource, sprite_w, sprite_h)),
  _frame_duration(frame_duration),
  _from(from),
  _to(to),
  _mod( _to > _from ? 1 : -1),
  _current(_from)
{
}

bool is_it_the_end(anim * a, int next)
{
  return
    (a->modifier() > 0 && (next < a->from() || next > a->to()))
    ||
    (a->modifier() < 0 && (next > a->from() || next < a->to()));
}

unsigned int schedule_callback(unsigned int interval, void *param)
{
  anim * a = (anim*)param;
  
  // find out next animation frame index
  int next = a->current() + a->modifier();
  
  if (is_it_the_end(a, next)) {

    if (a->mode() == anim::once) {
      // reached end of animation for "once"
      a->stop(false);
      return 0;
    }

    if (a->mode() == anim::repeat) {
      // reach on the end borders for "repeat" mode
      // restart from the begining in same direction
      next = a->from();
    }

    if (a->mode() == anim::occilate) {
      // reach on the end borders for "occilate" mode
      // restart from current position backwards
      a->reset(a->to(), a->from());
      next = a->current();
    }
  }

  // update frame 
  a->set_current(next);
  // return duration of the next frame
  return a->frame_duration();
}

void anim::reset(int from, int to)
{
  _from = from; _to = to;
  _mod = (_to > _from ? 1 : -1);
}

void anim::start()
{
  if (is_running())
    throw std::runtime_error("Animation is already running");

  if (_from >= 0 && 
      _to >= 0 &&
      _mod != 0 &&
      _to != _from)
  {
    if (_timer) SDL_RemoveTimer(_timer);
    _timer = SDL_AddTimer(_frame_duration, 
                          schedule_callback,
                          this);
    // notify
    event start_ev(this);
    started(start_ev);
  }
  else
    throw std::runtime_error("Can not start an animation");
}

void anim::stop(bool reset_current)
{
    if (!is_running())
      throw std::runtime_error("Animation is already stopped");

    SDL_RemoveTimer(_timer);
    _timer = 0;
    if (reset_current)
      _current = _from;

    // notify
    event stop_ev(this);
    stopped(stop_ev);
}