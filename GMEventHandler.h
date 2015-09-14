#ifndef GM_EVENT_HANDLER_H_
#define GM_EVENT_HANDLER_H_

#include <boost/bind.hpp>
#include <boost/function.hpp>

/******************************************************************
* Event Handler
*/
template<typename ClassT>
class event_handler {
public:
  typedef boost::function<void (ClassT*)> event_callback;
  typedef std::list<event_callback> callbacks_list;

  event_handler() {}

  void operator+=(event_callback clb)
  {
    _callbacks.push_back(clb);
  }

  void operator()(ClassT * target_instance)
  {
    callbacks_list::iterator it = _callbacks.begin();
    for(; it != _callbacks.end(); ++it) {
      event_callback & clb = *it;
      clb(target_instance);
    }
  }

private:
  callbacks_list _callbacks;
};

#endif //GM_EVENT_HANDLER_H_