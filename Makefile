# GMLib Build 
VERSION=1.0

CXX?=c++
PREFIX?=/usr/local
OBJDIR?=.obj
GMLIB=libgm
INSTALL_DIR=$(PREFIX)/lib/
INCLUDE_DIR=$(PREFIX)/include/gmlib


PYTHON_LDFLAGS="$(shell python3-config --libs)"
ifneq ("$(PYTHON_LIB)", "")
	PYTHON_LDFLAGS=-L$(PYTHON_LIB) -lpython3.6m
endif

# GMLib and SDLEx sources 
S_SRC= $(wildcard src/*.cpp) $(wildcard sdl_ex/*.cpp)

CFLAGS+=-std=c++11 \
		-Wall -Werror -pedantic \
		-Wno-unused-function
CFLAGS+=-Wsign-compare -Wshadow -Wpointer-arith -Wcast-qual
CFLAGS+=-DPREFIX='"$(PREFIX)"' \
		-I./include \
		-I./sdl_ex
LDFLAGS+="-liconv"


OSNAME=$(shell uname -s | sed -e 's/[-_].*//g' | tr A-Z a-z)

ifeq ("$(OSNAME)", "darwin")
	SHARED_SUFFIX=dylib
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

STATIC_SUFFIX=a
SHARED_SUFFIX=so

ifeq ("$(OSNAME)", "darwin")
	SHARED_SUFFIX=dylib
endif

# User defined paths to includes
ifneq ("$(PYTHON_INCLUDE)", "")
	CFLAGS +=-I$(PYTHON_INCLUDE)
endif
ifneq ("$(BOOST_INCLUDE)", "")
	CFLAGS +=-I$(BOOST_INCLUDE)
endif
ifneq ("$(SDL2_INCLUDE)", "")
	CFLAGS +=-I$(SDL2_INCLUDE)
endif
ifneq ("$(SDL2_IMAGE_INCLUDE)", "")
	CFLAGS +=-I$(SDL2_IMAGE_INCLUDE)
endif
ifneq ("$(SDL2_TTF_INCLUDE)", "")
	CFLAGS +=-I$(SDL2_TTF_INCLUDE)
endif
ifneq ("$(JANSSON_INCLUDE)", "")
	CFLAGS +=-I$(JANSSON_INCLUDE)
endif

ifneq ("$(DEBUG)", "")
	CFLAGS+=-DGM_DEBUG -g
	NOOPT=1
endif

ifneq ("$(UI_DEBUG)", "")
	CFLAGS+=-DGM_UIDEBUG -g
	NOOPT=1
endif

S_OBJS=	$(S_SRC:%.cpp=$(OBJDIR)/%.o)


# Link to user specified libraries for shared lib build
LDFLAGS+=$(PYTHON_LDFLAGS) \
		 -L$(JANSSON_LIB) -ljansson \
		 -L$(BOOST_LIB) -lboost_filesystem -lboost_system \
		 -L$(SDL2_LIB) -lSDL2 \
		 -L$(SDL2_IMAGE_LIB) -lSDL2_image \
		 -L$(SDL2_TTF_LIB) -lSDL2_ttf \

static: $(OBJDIR) $(S_OBJS)
	ar rcs $(GMLIB).a $(S_OBJS)

shared: $(OBJDIR) $(S_OBJS)
	$(CXX) -shared $(LDFLAGS) $(S_OBJS) -o $(GMLIB).$(SHARED_SUFFIX)

objects: $(OBJDIR) $(S_OBJS)

all: static

$(OBJDIR):
	@mkdir -p $(OBJDIR)/src
	@mkdir -p $(OBJDIR)/sdl_ex

install:
	mkdir -p $(INCLUDE_DIR)
	mkdir -p $(INSTALL_DIR)
	install -m 555 $(GMLIB).$(STATIC_SUFFIX) $(INSTALL_DIR)/$(GMLIB).$(STATIC_SUFFIX).$(VERSION)
	install -m 555 $(GMLIB).$(SHARED_SUFFIX) $(INSTALL_DIR)/$(GMLIB).$(SHARED_SUFFIX).$(VERSION)
	install -m 644 *.h $(INCLUDE_DIR)
	ln -s $(INSTALL_DIR)/$(GMLIB).$(STATIC_SUFFIX).$(VERSION) $(INSTALL_DIR)/$(GMLIB).$(STATIC_SUFFIX)
	ln -s $(INSTALL_DIR)/$(GMLIB).so.$(VERSION) $(INSTALL_DIR)/$(GMLIB).so

uninstall:
	rm -f $(INSTALL_DIR)/$(GMLIB).$(STATIC_SUFFIX) $(INSTALL_DIR)/$(GMLIB).$(STATIC_SUFFIX.$(VERSION)
	rm -f $(INSTALL_DIR)/$(GMLIB).$(SHARED_SUFFIX) $(INSTALL_DIR)/$(GMLIB).$(SHARED_SUFFIX.$(VERSION)
	rm -rf $(INCLUDE_DIR)

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	find . -type f -name \*.o -exec rm {} \;
	rm -rf $(GMLIB).$(STATIC_SUFFIX) $(GMLIB).$(SHARED_SUFFIX) $(OBJDIR)

.PHONY: all clean
