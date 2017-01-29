# GMLib Makefile

include ../../flags.make

# GMLib demo application
S_SRC= $(wildcard ./*.cpp)

CFLAGS+=-std=c++11 \
		-Wall -Werror -pedantic \
		-Wno-unused-function
CFLAGS+=-Wsign-compare -Wshadow -Wpointer-arith -Wcast-qual

# include paths to GMLib and SDLEx
CFLAGS+=-I../../include \
		-I../../sdl_ex
# link to local build of libgm 
LDFLAGS+=-liconv -L../.. -lgm

ifeq ("$(OSNAME)", "darwin")
	LDFLAGS+=-framework Foundation\
			 -framework CoreFoundation \
			 -framework CoreAudio \
			 -framework AudioUnit \
			 -framework AppKit \
			 -framework IOKit \
			 -framework ApplicationServices \
			 -framework CoreVideo \
			 -framework ForceFeedback \
			 -framework Carbon \
			 -framework Cocoa

endif

S_OBJS=	$(S_SRC:%.cpp=$(OBJDIR)/%.o)

demo: $(OBJDIR) $(S_OBJS)
	$(CXX) $(LDFLAGS) $(S_OBJS) -o demo

linklibs:
	@rm -f libfreetype.$(SHARED_SUFFIX)
	@rm -f libboost_filesystem.$(SHARED_SUFFIX)
	@rm -f libboost_system.$(SHARED_SUFFIX)
	@ln -s $(FREETYPE_LIB)/libfreetype.$(SHARED_SUFFIX) libfreetype.$(SHARED_SUFFIX)
	@ln -s $(BOOST_LIB)/libboost_filesystem.$(SHARED_SUFFIX) libboost_filesystem.$(SHARED_SUFFIX)
	@ln -s $(BOOST_LIB)/libboost_system.$(SHARED_SUFFIX) libboost_system.$(SHARED_SUFFIX)

objects: $(OBJDIR) $(S_OBJS)

all: demo

$(OBJDIR):
	@mkdir -p $(OBJDIR)


$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	find . -type f -name \*.o -exec rm {} \;
	rm -rf demo

.PHONY: all clean
