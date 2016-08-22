#include "GMUI.h"

namespace ui {

#define YES_NO(val) (val == true ? "yes" : "no")
#define CONTROL_ID_LEN 5

/** Control **/

// regular control constructor. manager is a parent by default
control::control(rect pos):
  _scrolled_rect(0, 0, pos.w, pos.h),
  _visible(true), _proxy(false), _locked(false),
  _pos(pos), _parent(NULL), _id(newid())
{
  if (UI_Debug()) SDL_Log("control: {%s} created", identifier().c_str());
  manager::instance()->add_child(this);
}

// regular control constructor. manager is a parent by default with custom ID
control::control(rect pos, const std::string id):
  _visible(true), _proxy(false), _locked(false),
  _scrolled_rect(0, 0, pos.w, pos.h),
  _pos(pos), _parent(NULL), _id(id)
{
  if (UI_Debug()) SDL_Log("control: {%s} created", identifier().c_str());
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


std::string control::newid()
{
  std::stringstream ss;
  for(int i = 0; i < CONTROL_ID_LEN; ++i) ss << rand_char();
  return ss.str();
}

control::~control()
{
  if (UI_Debug()) SDL_Log("control: {%s} destroyed", identifier().c_str());
}

std::string control::get_type_name()
{
  static char * class_prefix = "class ";
  static size_t class_prefix_len = strlen(class_prefix);

  std::string n = typeid(this).name();
  if (n.find(class_prefix) == 0) {
    size_t next = n.find(" ", class_prefix_len);
    if (next == n.npos)
      next = n.size() - 1;
    
    std::string s = n.substr(class_prefix_len, next - class_prefix_len); 
    return s;
  }

  return n;
}

std::string control::tostr()
{
  std::stringstream ss;
  ss << "{" << get_type_name() \
     << " id: " << _id \
     << " pos: " << _pos.tostr() \
     << " visible: " << YES_NO(_visible) \
     << " proxy: " << YES_NO(_proxy) \
     << " children: " << _children.size()
     << "}";
  return ss.str();
}

void control::load(const data & d)
{
  std::string old_id = _id;
  _id = d.get("id", _id);
  _visible = d.get("visible", true);
  _proxy = d.get("proxy", false);

  if (UI_Debug()) SDL_Log("control: {%s} reloaded as {%s} proxy: %s, visible: %s",
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
    // offscreen rendered controls are affected by scrolled_rect
    // unless they are locked in position
    if (!is_locked())
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
  if (UI_Debug()) SDL_Log("control: {%s} changed owner from %s to %s", 
    identifier().c_str(), 
    _parent == NULL ? "<nobody>" : _parent->identifier().c_str(), 
    parent == NULL ? "<nobody>" : parent->identifier().c_str()
    );
  _parent = parent;
}

size_t control::find_child_index(control * c)
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
  // search children
  if (_children.size() > 0) {
    // iterate children in reverse order, because the
    // bottom is of the list is rendered at the top
    control_list::reverse_iterator it = _children.rbegin();
    for(; it != _children.rend(); ++it) {
      control * child = *it;
      if (child->visible() && child->get_absolute_pos().collide_point(at) )
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

control_list::iterator control::find_child(control* child)
{
  return std::find(_children.begin(), _children.end(), child);
}

void control::add_child(control* child)
{
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
  control_list::iterator it = find_child(child);
  if (it != _children.end()) {
    _children.erase(it);
  }
}

void control::insert_child(size_t idx, control * c)
{
  _children.insert(_children.begin() + idx, c);
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
  
  if (UI_Debug()) {
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