#include "GMUI.h"

namespace ui {

#define CONTROL_ID_LEN 5

/** Control **/

// regular control constructor. manager is a parent by default
control::control(const rect & pos):
  _visible(true), _proxy(false), _locked(false), 
  _destroyed(false), _disabled(false),
  _pos(pos), _parent(NULL), _id(newid()),
  _scrolled_rect(0, 0, pos.w, pos.h)
{
#ifdef GM_DEBUG_UI
  SDL_Log("control::control created new %s", tostr().c_str());
#endif
  manager::instance()->add_child(this);
}

// regular control constructor. manager is a parent by default with custom ID
control::control(const rect & pos, const std::string id):
  _visible(true), _proxy(false), _locked(false), 
  _destroyed(false), _disabled(false),
  _pos(pos), _parent(NULL), _id(id),
  _scrolled_rect(0, 0, pos.w, pos.h)
{
#ifdef GM_DEBUG_UI
  SDL_Log("control::control created new %s", tostr().c_str());
#endif
  manager::instance()->add_child(this);
}

// manager's constructor
control::control():
  _id("root"), _parent(NULL), 
  _visible(true), _proxy(false), _locked(false), 
  _destroyed(false), _disabled(false),
  _pos(rect(GM_GetDisplayRect())), 
  _scrolled_rect(0, 0, _pos.w, _pos.h)
{
#ifdef GM_DEBUG_UI
  SDL_Log("%s - ui manager intialized with %s", 
    __METHOD_NAME__,
    _pos.tostr().c_str());
#endif
}


std::string control::newid()
{
  std::stringstream ss;
  for(int i = 0; i < CONTROL_ID_LEN; ++i) ss << rand_char();
  return ss.str();
}

control::~control()
{
  // destroy children of this control
#ifdef GM_DEBUG_UI
  SDL_Log("%s - destroying %d children of id: %s",
    __METHOD_NAME__,
    _children.size(),
    identifier().c_str());
#endif
  control_list::iterator it = _children.begin();
  for(; it != _children.end(); ++it) {
    control * child = *it;
    delete child;
  }
#ifdef GM_DEBUG_UI
  SDL_Log("%s - destroyed id: %s", 
    __METHOD_NAME__,
    identifier().c_str());
#endif
}

std::string control::tostr() const
{
  std::stringstream ss;
  if (_destroyed)
    ss << "{" << get_type_name() \
       << " id=" << _id \
       << ", !zombie}";
  else if (_proxy)
    if (_destroyed)
      ss << "{" << get_type_name() \
         << " id=" << _id \
         << ", pos=" << _pos.tostr() \
         << ", !proxy=yes" \
         << ", parent=" << (_parent == nullptr ? "<null>" : _parent->identifier().c_str()) \
         << "}";
  else
    ss << "{" << get_type_name() \
       << " id=" << _id \
       << ", pos=" << _pos.tostr() \
       << ", visible=" << YES_NO(_visible) \
       << ", disabled=" << YES_NO(_disabled) \
       << ", parent=" << (_parent == nullptr ? "<null>" : _parent->identifier().c_str()) \
       << "}";
  return ss.str();
}

void control::load(const data & d)
{
  std::string old_id = _id;
  _id = d.get("id", _id);
  _visible = d.get("visible", true);
  _proxy = d.get("proxy", false);

#ifdef GM_DEBUG_UI
  SDL_Log("control::load id: %s reloaded as %s",
    old_id.c_str(),
    tostr().c_str()
  );
#endif
}

rect control::get_absolute_pos()
{
  if (_parent != NULL) {
    rect parent_rect = _parent->get_absolute_pos();
    rect pos = _pos + parent_rect.topleft();
    // offscreen rendered controls are affected by scrolled_rect
    // unless they are locked in position
    if (!locked())
      pos = pos - _parent->get_scrolled_rect().topleft();
    // absolute pos is a visible part of the control (within parent's rect)
    pos.clip(parent_rect);
    return pos;
  }
  return _pos;
}

void control::set_parent(control* parent)
{
  manager * mgr = manager::instance();
  std::string s;
#ifdef GM_DEBUG_UI
  SDL_Log("control::set_parent id: %s changed parent from %s to %s", 
    identifier().c_str(), 
    _parent == NULL ? "<nobody>" : _parent->identifier().c_str(), 
    parent == NULL ? "<nobody>" : parent->identifier().c_str()
    );
#endif
  _parent = parent;
}

size_t control::find_child_index(const control * c)
{
  return find_child(c) - _children.begin();
}

size_t control::zlevel()
{
  if (_parent) {
    return _parent->find_child_index(this);
  }
  return 0;
}

control * control::find_child_at(uint32_t x, uint32_t y)
{
  return find_child_at(point(x, y));
}

control * control::find_child_at(const point & at)
{
  lock_container(_children);
  // search children
  if (_children.size() > 0) {
    // iterate children in reverse order, because the
    // bottom of the list is rendered at the top
    control_list::reverse_iterator it = _children.rbegin();
    for(; it != _children.rend(); ++it) {
      control * child = *it;
      if (!child->destroyed() && child->visible() && child->get_absolute_pos().collide_point(at) )
        return child->find_child_at(at);
    }
  }

  // if manager didnt find any controls return NULL
  if (this == manager::instance()) 
    return NULL;
  
  // if control didnt find any children and itself matches, return it
  if (get_absolute_pos().collide_point(at)) {
    return this;
  }
  // nothing at all
  return NULL;
}

control_list::const_iterator control::find_child(const control * child)
{
  control_list::const_iterator start = _children.begin();
  control_list::const_iterator finish = _children.end();
  return std::find(start, finish, child);
}

control_list::iterator control::find_child(control * child)
{
  return std::find(_children.begin(), _children.end(), child);
}

void control::add_child(control* child)
{
  lock_container(_children);
  if (find_child(child) == _children.end()) {
    // append childen at the bottom of the list
    _children.push_back(child);
    if (child->parent()) {
      child->parent()->remove_child(child);
    }
    child->set_parent(this);
  }
}

void control::remove_child(control* child)
{
  lock_container(_children);
  control_list::iterator it = find_child(child);
  if (it != _children.end()) {
    _children.erase(it);
  }
}

control * control::get_child_at_index(size_t idx)
{
  if (idx < 0 || idx >= _children.size()) {
    SDLEx_LogError("control::get_child_at_index - invalid index: %d", idx);
    throw std::exception("invalid child index to get");
  }
  return _children[idx];
}

void control::insert_child(size_t idx, control * c)
{
  lock_container(_children);
  _children.insert(_children.begin() + idx, c);
}

void control::render(SDL_Renderer* r, const rect & dst)
{
  lock_container(_children);
  control_list::iterator it = _children.begin();
  
  for(; it != _children.end(); ++it) {
    control * c = *it;
    if (c->destroyed() || !c->visible()) continue;
    rect control_dst = c->get_absolute_pos();
    c->render(r, control_dst);
  }
}

void control::update()
{
  lock_container(_children);
  control_list::iterator it = _children.begin();
  for(; it != _children.end(); ++it) {
    control * child = *it;
    if (!child->destroyed() && !child->proxy()) {
      child->update();
    }
  }
}

}; //namespace ui