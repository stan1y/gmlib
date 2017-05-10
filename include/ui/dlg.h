#ifndef DLG_H
#define DLG_H

#include "engine.h"
#include "manager.h"

namespace ui {

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
