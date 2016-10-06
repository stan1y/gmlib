#include "GMLib.h"
#include "GMUI.h"
#include "RESLib.h"

#include "GMUIBox.h"
#include "GMUIButton.h"
#include "GMUITextInput.h"
#include "GMUIPushButton.h"

class demo_screen : public screen {
private:
  ui::panel * _panel_with_buttons;
  ui::panel * _panel_from_json;
  multi_texture _back;

public:
  demo_screen():
    _panel_with_buttons(NULL),
    _panel_from_json(NULL),
    _back(GM_GetDisplayRect(), 1024 / 64, 768 / 64) 
  {
    add_component(ui::manager::instance());

    // build background
    texture const * start = resources::get_texture("test.png");
    _back.render_texture(GM_GetRenderer(), *start, point(rand_int(10, 500), rand_int(10, 350)));
    //_back.render_texture(GM_GetRenderer(), *start, point(32, 32));
    
    // build UI manually
    _panel_with_buttons = build_panel();
    
    // build UI automatically from ui.json file
    _panel_from_json = ui::build<ui::panel>(resources::get_data("demo.ui.json"));
    _panel_from_json->find_child("btn_one")->mouse_down \
      += boost::bind(&demo_screen::on_btn_clicked, this, _1);
    _panel_from_json->find_child("btn_two")->mouse_down \
      += boost::bind(&demo_screen::on_btn_clicked, this, _1);
    _panel_from_json->find_child("btn_three")->mouse_down \
      += boost::bind(&demo_screen::on_btn_clicked, this, _1);
    
    // populate list panel with some items
    ui::box * list = _panel_from_json->find_child<ui::box>("vbox_list");
    for(int i = 0; i < 25; ++i) {
      ui::label * lbl = new ui::label(rect(0, 0, list->pos().w, 15));
      std::stringstream ss;
      ss << "item-" << i;
      lbl->set_text(ss.str());
      list->add_child(lbl);
    }
  }

  ui::panel * build_panel()
  {
    auto t = ui::current_theme();
    auto pnl = new ui::panel(rect(50, 100, 200, 150));
    pnl->set_box_style(ui::box::fill);
    
    auto lbl = new ui::label(rect(200, 0, 80, 20));
    lbl->set_halign(ui::label::center);
    lbl->set_valign(ui::label::middle);
    lbl->set_idle_color(color::green());
    lbl->set_highlight_color(color::cyan());
    lbl->set_text("The Label with text in VBox");
    pnl->add_child(lbl);

    auto btn = new ui::btn(rect(0, 0, 0, 40));
    btn->set_idle_color(color::red());
    btn->set_highlight_color(color::magenta());
    btn->set_halign(ui::label::center);
    btn->set_valign(ui::label::middle);
    btn->set_text("Click Me!");
    pnl->add_child(btn);
    
    btn->mouse_up += boost::bind(&demo_screen::on_btn_clickme_clicked, this, _1);
    
    auto input = new ui::text_input(rect(0, 0, 200, 40));
    input->set_margin(2);
    input->set_text("alphanum text input");
    pnl->add_child(input);
    
    auto btn_group = new ui::box(rect(0, 0, 200, 50), ui::box::hbox, ui::box::box_style::pack_end, 5);
    // here we specify path to the file with icon with help of theme's relative resources path api
    // in json, the value for "icon_idle" and others is a resource id for simplification of the declarative use
    btn_group->add_child(new ui::push_btn(t.get_resource("pushbtn/Icons/ArrowLeft.png")));
    auto info_btn = new ui::push_btn(t.get_resource("pushbtn/Icons/InfoIcon.png"));
    info_btn->set_disabled(true);
    btn_group->add_child(info_btn);
    btn_group->add_child(new ui::push_btn(t.get_resource("pushbtn/Icons/ArrowRight.png")));
    pnl->add_child(btn_group);
    
    return pnl;
  }

  void on_btn_clickme_clicked(ui::control * target)
  {
  }

  void on_btn_clicked(ui::control * target)
  {
    ui::button * clicked = dynamic_cast<ui::button*>(target);
    if (clicked == nullptr) return;
    std::stringstream msg;
    msg << "You've clicked " << clicked->get_text();
    ui::message::alert(msg.str(), 30);
  }

  virtual void render(SDL_Renderer* r)
  {
    color(0, 0, 100, 255).apply(r);
    SDL_RenderClear(r);

    // render background
    _back.render(r);

    // render screen components
    screen::render(r);
  }
};

int main(int argc, char* argv[]) 
{
  if (argc != 2) {
    SDLEx_LogError("main: no config specified. Usage: \"%s  Pebble/config.json\"", argv[0]);
    return 1;
  }

  int rc = GM_Init(argv[1], "UI Demo");
  if (rc != 0) {
    SDLEx_LogError("main: failed to initialize GMLib. Exiting with code %d...", rc);
    return rc;
  }

  // setup demo screen
  auto demo = new demo_screen();
  screen::set_current(demo);

  // game loop
  while (!SDL_QuitRequested()) {
    GM_StartFrame();
    GM_UpdateFrame();
    GM_RenderFrame();
    GM_EndFrame();
  }

  SDL_Log("main: quiting...");
  GM_Quit();

  return rc;
}

