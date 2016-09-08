#include "GMLib.h"
#include "GMUI.h"
#include "RESLib.h"

#include "GMUIBox.h"
#include "GMUIButton.h"

class demo_screen : public screen {
private:
  ui::panel * _panel_with_buttons;
  ui::panel * _panel_from_json;
  texture _back;

public:
  demo_screen():
    _panel_with_buttons(NULL),
    _panel_from_json(NULL)
  {
    _back.load(resources::find("start.png"));

    _panel_with_buttons = build_panel();

    _panel_from_json = ui::build<ui::panel>(resources::get_data("demo.ui.json"));
    _panel_from_json->find_child("btn_one")->mouse_down \
      += boost::bind(&demo_screen::on_btn_clicked, this, _1);
    _panel_from_json->find_child("btn_two")->mouse_down \
      += boost::bind(&demo_screen::on_btn_clicked, this, _1);
    _panel_from_json->find_child("btn_three")->mouse_down \
      += boost::bind(&demo_screen::on_btn_clicked, this, _1);
  }

  ui::panel * build_panel()
  {
    auto pnl = new ui::panel(rect(50, 100, 200, 150));
    pnl->set_box_style(ui::box::fill);
    
    auto lbl = new ui::label(rect(200, 0, 80, 20));
    lbl->set_halign(ui::label::center);
    lbl->set_valign(ui::label::middle);
    lbl->set_font_idle_color(color::green());
    lbl->set_font_hover_color(color::cyan());
    lbl->set_text("The Label with Text");
    pnl->add_child(lbl);

    auto btn = new ui::btn(rect(0, 0, 0, 40));
    btn->set_font_idle_color(color::red());
    btn->set_font_hover_color(color::magenta());
    btn->set_halign(ui::label::center);
    btn->set_valign(ui::label::middle);
    btn->set_text("Click Me!");
    pnl->add_child(btn);

    return pnl;
  }

  void on_btn_clicked(ui::control * target)
  {
    ui::button * clicked = dynamic_cast<ui::button*>(target);
    if (clicked == nullptr) return;
    std::stringstream msg;
    msg << "You've clicked " << clicked->get_text();
    ui::message::alert(msg.str(), 30);
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

  // enable debug UI 
  ui::manager::instance()->set_debug(true);

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

