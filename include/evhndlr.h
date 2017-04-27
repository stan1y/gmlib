#ifndef GM_EVENT_HANDLER_H_
#define GM_EVENT_HANDLER_H_

#include <boost/bind.hpp>
#include <boost/function.hpp>

/**
 * the event_handler class
 * subscription to events with boost::function callbacks
 */
template<typename Event>
class event_handler {
public:
  typedef boost::function<void (Event)> event_callback;
  typedef std::list<event_callback> callbacks_list;

  event_handler() {}

  void operator+=(event_callback clb)
  {
    _callbacks.push_back(clb);
  }

  void operator()(Event arg)
  {
    typename callbacks_list::iterator it = _callbacks.begin();
    for(; it != _callbacks.end(); ++it) {
      event_callback & clb = *it;
      clb(arg);
    }
  }

private:
  callbacks_list _callbacks;
};

#endif //GM_EVENT_HANDLER_H_
