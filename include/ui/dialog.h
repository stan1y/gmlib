#ifndef DLG_H
#define DLG_H

#include "engine.h"
#include "manager.h"
#include "box.h"

namespace ui {

class dialog {
public:

  typedef enum {
    dialog_accept,
    dialog_cancel

  } dialog_result;

  dialog(const std::string & text,
         const std::string & file_name);
  virtual ~dialog() = 0;

  panel * root() { return _root; }
  box * main() { return _root->find_child<box>("main"); }
  box * buttons() { return _root->find_child<box>("buttons"); }

  ui::label * caption() { return _caption; }
  ui::button * accept_btn() { return _root->find_child<button>("accept"); }
  ui::button * cancel_btn() { return _root->find_child<button>("cancel"); }

  void on_cancel(ui::control *);
  void on_accept(ui::control *);

  void show() { _root->set_visible(true); }
  void hide() { _root->set_visible(false); }

  dialog_result get_result() const { return _res; }

protected:
  dialog_result _res;
  panel * _root;
  ui::label * _caption;
};

class load_file_dialog: public dialog {
public:
  load_file_dialog(const std::string & file_name,
                   const path & dir,
                   const std::string & ext);
  virtual ~load_file_dialog();

  void refresh();
  void reset();

  const std::string & get_filename();
  const path & get_path();

  void on_file_clicked(ui::control *);
  void on_refresh(ui::control *);
  ui::button * refresh_btn() { return _root->find_child<button>("refresh"); }

protected:
  void find_files(paths_list & out, const std::string & ext);

private:
  std::string _ext;
  path _root_dir;
  path _selected;
};

/**
  @brief the ui message dialog
  UI Message control.
  Fade-out top level alert with text
  */
class message: public control {
public:

  virtual ~message();
  virtual void render(SDL_Renderer * r, const rect & dst);
  virtual void update();
  void show();

  virtual std::string get_type_name() const { return "message"; }

  /* show global alert */
  static void alert_ex(const std::string & text, const ttf_font * f, const color & c, uint32_t timeout_ms);
  static void alert(const std::string & text, uint32_t timeout_ms);

protected:
  /* use static methods to create new instances of global message alert */
  message(const std::string & text, const ttf_font * f, const color & c, uint32_t timeout_ms);

private:
  void reset(const std::string & text, const ttf_font * f, const color & c, uint32_t timeout_ms);
  uint32_t _timeout_ms;
  timer _timer;
  std::string _text;
  texture _tx;
};

}

#endif // DLG_H
