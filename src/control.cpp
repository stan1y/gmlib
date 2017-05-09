#include "manager.h"

namespace ui {

#define CONTROL_ID_LEN 5

/** Control **/

// regular control constructor. manager is a parent by default
control::control(const rect & pos):
  _parent(NULL),
	_pos(pos),
  _scrolled_rect(0, 0, pos.w, pos.h),
  _visible(true), _proxy(false), _locked(false), 
  _destroyed(false), _disabled(false),
  _id(rand_string(CONTROL_ID_LEN))
  
{
  manager::instance()->add_child(this);
}

// regular control constructor. manager is a parent by default with custom ID
control::control(const rect & pos, const std::string id):
  _parent(NULL), _pos(pos),
  _scrolled_rect(0, 0, pos.w, pos.h),
  _visible(true), _proxy(false), _locked(false), 
  _destroyed(false), _disabled(false),
  _id(id)
{
  manager::instance()->add_child(this);
}

// manager's constructor
control::control():
  _parent(NULL), _pos(GM_GetDisplayRect()),
  _scrolled_rect(GM_GetDisplayRect()),
  _visible(true), _proxy(false), _locked(false), 
  _destroyed(false), _disabled(false),
  _id("root")
{
  SDL_Log("ui::manager - initialized %s",
    _pos.tostr().c_str());
}

control::~control()
{
  // destroy children of this control
  control_list::iterator it = _children.begin();
  for(; it != _children.end(); ++it) {
    control * child = *it;
    delete child;
  }
}

std::string control::tostr() const
{
  std::stringstream ss;
  if (_destroyed) {
    ss << "{" << get_type_name() \
       << " id=" << _id \
       << ", !zombie}";
  }
  else if (_proxy) {
    ss << "{" << get_type_name() \
        << " id=" << _id \
        << ", pos=" << _pos.tostr() \
        << ", !proxy=yes" \
        << ", parent=" << (_parent == nullptr ? "<null>" : _parent->identifier().c_str()) \
        << "}";
  }
  else {
    ss << "{" << get_type_name() \
       << " id=" << _id \
       << ", pos=" << _pos.tostr() \
       << ", visible=" << YES_NO(_visible) \
       << ", disabled=" << YES_NO(_disabled) \
       << ", parent=" << (_parent == nullptr ? "<null>" : _parent->identifier().c_str()) \
       << "}";
  }
  return ss.str();
}

void control::load(const json & d)
{
  if (d.find("id") != d.end())
    _id = d["id"];
  if (d.find("visible") != d.end())
    _visible = d["visible"];
  if (d.find("proxy") != d.end())
    _proxy = d["proxy"];
}

rect control::get_absolute_pos() const
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
  _parent = parent;
}

size_t control::find_child_index(const control * c)
{
  control_list::const_iterator found = find_child(c);
  if (found == _children.end())
    throw std::runtime_error("child control not found");

  return found - _children.begin();
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
  if (idx >= _children.size()) {
    SDL_Log("control::get_child_at_index - invalid index: %zu", idx);
    throw std::runtime_error("invalid child index to get");
  }
  return _children[idx];
}

void control::insert_child(size_t idx, control * c)
{
  lock_container(_children);
  _children.insert(_children.begin() + idx, c);
}

void control::draw(SDL_Renderer* r, const rect & dst)
{
  if (_children.size() == 0)
    return;

  lock_container(_children);
  control_list::iterator it = _children.begin();
  for(; it != _children.end(); ++it) {
    control * c = *it;
    if (c->destroyed() || !c->visible()) continue;
    rect control_dst = c->get_absolute_pos();
    c->draw(r, control_dst);
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

} //namespace ui
