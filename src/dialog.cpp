#include "dialog.h"
#include "button.h"
#include "text.h"

namespace ui {

/* Message Alert Dialog */

static message * g_message = NULL;
static std::mutex g_message_mx;

message::message(const std::string & text, const ttf_font * f, const color & c, uint32_t timeout_ms):
  control(f->get_text_rect(text)),
  _timeout_ms(timeout_ms)
{
  reset(text, f, c, timeout_ms);
}

void message::reset(const std::string & text, const ttf_font * f, const color & c, uint32_t timeout_ms)
{
  _timer.stop();
  set_visible(false);
  _text = text;
  SDL_Surface *s = f->print_solid(_text, c);
  _tx.set_surface(s);
  SDL_FreeSurface(s);
  rect display = GM_GetDisplayRect();

  _pos.w = _tx.width();
  _pos.h = _tx.height();
  // middle of the screen with Y offset
  _pos.x = (display.w - _pos.w) / 2;
  _pos.y = 25 + _pos.h;
  show();
}

void message::update()
{
  if (!_timer.is_started())
    return;

  g_message_mx.lock();
  uint32_t ticked = _timer.get_ticks();
  uint32_t step = _timeout_ms / 255;
  //deplete 1 alpha each step
  uint32_t depleted = ticked / step;
  if (depleted >= 255) {
    _timer.stop();

    //self-destruct when alpha depleted
    ui::destroy(this);
    g_message = NULL;
  }

  uint8_t a = 255 - uint32_to_uint8(depleted);
  _tx.set_alpha(a);
  g_message_mx.unlock();
}

void message::show()
{
  set_visible(true);
  _timer.start();
}

void message::alert(const std::string & text, uint32_t timeout_ms)
{
  alert_ex(text,
    ui::manager::instance()->get_font("alert"),
    ui::manager::instance()->get_highlight_color("alert"),
    timeout_ms);
}

void message::alert_ex(const std::string & text, const ttf_font * f, const color & c, uint32_t timeout_ms)
{
  g_message_mx.lock();
  if (g_message == NULL)
    g_message = new message(text, f, c, timeout_ms);
  else
    g_message->reset(text, f, c, timeout_ms);
  g_message_mx.unlock();
}

message::~message()
{

}

void message::render(SDL_Renderer * r, const rect & dst)
{
  g_message_mx.lock();
  _tx.render(r, dst);
  g_message_mx.unlock();
}

/* Generic Dialog */

dialog::dialog(const std::string & caption,
               const std::string & file_name):
  _res(dialog_cancel),
  _root(ui::build_file<panel>(file_name))
{
  _caption = _root->find_child<ui::label>("caption");
  _caption->set_text(caption);
  cancel_btn()->mouse_up += boost::bind(&dialog::on_cancel, this, _1);
  accept_btn()->mouse_up += boost::bind(&dialog::on_accept, this, _1);
}

dialog::~dialog()
{

}

void dialog::on_cancel(ui::control *)
{
  _res = dialog_cancel;
  _root->set_visible(false);
}

void dialog::on_accept(ui::control *)
{
  _res = dialog_accept;
  _root->set_visible(false);
}

/* Load File Dialog */

load_file_dialog::load_file_dialog(const std::string & file_name,
                                   const path & dir,
                                   const std::string & ext):
  dialog(dir.leaf().string(), file_name),
  _ext(ext),
  _root_dir(dir)
{
  // setup custom buttons
  refresh_btn()->mouse_up += boost::bind(&load_file_dialog::on_refresh, this, _1);
}

load_file_dialog::~load_file_dialog()
{

}

void load_file_dialog::on_refresh(ui::control* target)
{

}

void load_file_dialog::find_files(paths_list & out, const std::string & ext)
{
  for(auto entry = directory_iterator(_root_dir);
      entry != directory_iterator();
      ++entry) {
        const path & p = *entry;

        if (p.filename().string()[0] == '.')
          continue;

        if (is_directory(p))
         continue;

        if (is_regular_file(p))
          out.push_back(p);
    }
}

const std::string & load_file_dialog::get_filename()
{
  return _selected.filename().string();
}

const path & load_file_dialog::get_path()
{
  return _selected;
}

void load_file_dialog::reset()
{
  _root->find_child<ui::text_input>("filename")->set_text("");
  _selected = path();
}

void load_file_dialog::on_file_clicked(ui::control * target)
{
  ui::button * clicked = dynamic_cast<ui::button*> (target);
  ui::text_input * inp = _root->find_child<ui::text_input>("filename");
  inp->set_text(clicked->get_text());
  _selected = path(clicked->get_user_data());
}

void load_file_dialog::refresh()
{
  main()->clear_children();
  paths_list found;
  find_files(found, _ext);

  paths_list::iterator it = found.begin();
  for(; it != found.end(); ++it) {
    ui::button * btn = new ui::button(rect(0, 0, 320, 30));
    btn->set_halign(ui::h_align::left);
    ui::padding pad(0, 5, 0, 0);
    btn->set_padding(pad);
    btn->set_text(it->filename().string());
    btn->set_user_data(it->string());
    btn->mouse_up += boost::bind(&load_file_dialog::on_file_clicked, this, _1);
    main()->add_child(btn);
  }
}

} // namespace ui
