#include "GMUI.h"

namespace ui {

/** Control **/

// regular control constructor. manager is a parent by default
control::control(rect pos):
  _scrolled_rect(0, 0, pos.w, pos.h),
  _visible(true), _proxy(false),
  
  _pos(pos), _parent(NULL), _id(newid())
{
  SDL_Log("control: {%s} created", identifier().c_str());
  manager::instance()->add_child(this);
}

// regular control constructor. manager is a parent by default with custom ID
control::control(rect pos, const std::string id):
  _visible(true), _proxy(false),
  _scrolled_rect(0, 0, pos.w, pos.h),
  _pos(pos), _parent(NULL), _id(id)
{
  SDL_Log("control: {%s} created", identifier().c_str());
  manager::instance()->add_child(this);
}

// manager's constructor
control::control():
  _visible(true), _proxy(false),
  _pos(rect(GM_GetDisplayRect())), 
  _scrolled_rect(0, 0, _pos.w, _pos.h),
  _parent(NULL), _id("root")
{
  SDL_Log("manager: ui ready");
}

#define CONTROL_ID_LEN 5
std::string control::newid()
{
  std::stringstream ss;
  for(int i = 0; i < CONTROL_ID_LEN; ++i) ss << rand_char();
  return ss.str();
}

control::~control()
{
  SDL_Log("control: {%s} destroyed", identifier().c_str());
}

#define YES_NO(val) (val == true ? "yes" : "no")

void control::load(data & d)
{
  std::string old_id = _id;
  _id = d.get("id", _id);
  _visible = d.get("visible", true);
  _proxy = d.get("proxy", false);
  
  SDL_Log("control: {%s} reloaded as {%s} proxy: %s, visible: %s",
    old_id.c_str(),
    _id.c_str(),
    YES_NO(_proxy),
    YES_NO(_visible)
  );
}

rect control::get_absolute_pos()
{
  if (_parent != NULL) {
    rect parent_rect = _parent->get_absolute_pos();
    rect pos = _pos + parent_rect.topleft();
    // offscreen controls are affected by scrolled_rect
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
  SDL_Log("control: {%s} changed owner from %s to %s", 
    identifier().c_str(), 
    _parent == NULL ? "<nobody>" : _parent->identifier().c_str(), 
    parent == NULL ? "<nobody>" : parent->identifier().c_str()
    );
  _parent = parent;
}

control * control::find_child_at(uint32_t x, uint32_t y)
{
  point at(x, y);
  // search children
  if (_children.size() > 0) {
    control_list::iterator it = _children.begin();
    for(; it != _children.end(); ++it) {
      control * child = *it;
      if (child->visible() && child->get_absolute_pos().collide_point(at) )
        return child->find_child_at(x, y);
    }
  }

  // if manager didnt find any controls return NULL
  if (this == manager::instance()) 
    return NULL;
  
  // if control didnt find any children and matches, return it
  if (get_absolute_pos().collide_point(at)) {
    return this;
  }
  return NULL;
}

control_list::iterator control::find_child(control* child)
{
  control_list::iterator it = _children.begin();
  for(; it != _children.end(); ++it) {
    if ( (*it) == child) {
      break;
    }
  }
  return it;
}

void control::add_child(control* child)
{
  if (find_child(child) == _children.end()) {
    _children.push_back(child);
    if (child->parent()) {
      child->parent()->remove_child(child);
    }
    child->set_parent(this);
  }
}

void control::remove_child(control* child)
{
  control_list::iterator it = find_child(child);
  if (it != _children.end()) {
    _children.erase(it);
  }
}

void control::render(SDL_Renderer* r, const rect & dst)
{
  control_list::iterator it = _children.begin();
  for(; it != _children.end(); ++it) {
    control * c = *it;
    if (!c->visible()) continue;
    rect control_dst = c->get_absolute_pos();
    c->render(r, control_dst);
  }
  if (manager::instance()->get_flags() & manager::ui_debug) {
    if (manager::instance()->get_hovered_control() == this) {
      static color red(255, 0, 0, 255);
      static color green(0, 255, 0, 255);
      red.apply(r);
      
      SDL_RenderDrawRect(r, &dst);
      if (_parent) {
        rect pdst = _parent->get_absolute_pos();
        pdst = pdst + rect(-1, -1, 2, 2);
        green.apply(r);
        SDL_RenderDrawRect(r, &pdst);
      }
    }
  }
}

void control::update()
{
  control_list::iterator it = _children.begin();
  for(; it != _children.end(); ++it) {
    (*it)->update();
  }
}

}; //namespace ui