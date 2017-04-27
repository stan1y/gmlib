# GMLib Makefile
include buildenv
include flags.make
include python.make

GMLIB=libgm
VERSION=1.0

INSTALL_DIR=$(PREFIX)/lib/
INCLUDE_DIR=$(PREFIX)/include/gmlib

# GMLib and SDLEx sources 
S_SRC= $(wildcard src/*.cpp) $(wildcard sdl_ex/*.cpp)

CFLAGS+=-std=c++11 \
		-Wall -Werror -pedantic \
		-Wno-unused-function
CFLAGS+=-Wsign-compare -Wshadow -Wpointer-arith -Wcast-qual
CFLAGS+=-DPREFIX='"$(PREFIX)"' \
    -I./sdl_ex  \
		-I./include \
    -I./include/ui
LDFLAGS+="-liconv"

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


S_OBJS=	$(S_SRC:%.cpp=$(OBJDIR)/%.o)

static: $(OBJDIR) $(S_OBJS)
	@ar rcs $(GMLIB).a $(S_OBJS)
	@echo "AR $(GMLIB).a"

shared: $(OBJDIR) $(S_OBJS)
	@$(CXX) -shared $(LDFLAGS) $(S_OBJS) -o $(GMLIB).$(SHARED_SUFFIX)
	@echo "CC -shared $(GMLIB).$(SHARED_SUFFIX)"

objects: $(OBJDIR) $(S_OBJS)

all: python static 

$(OBJDIR):
	@mkdir -p $(OBJDIR)/src
	@mkdir -p $(OBJDIR)/sdl_ex

install:
	mkdir -p $(INCLUDE_DIR)
	mkdir -p $(INSTALL_DIR)
	install -m 555 $(GMLIB).$(STATIC_SUFFIX) $(INSTALL_DIR)/$(GMLIB).$(STATIC_SUFFIX).$(VERSION)
	install -m 555 $(GMLIB).$(SHARED_SUFFIX) $(INSTALL_DIR)/$(GMLIB).$(SHARED_SUFFIX).$(VERSION)
	install -m 644 *.h $(INCLUDE_DIR)
	@ln -s $(INSTALL_DIR)/$(GMLIB).$(STATIC_SUFFIX).$(VERSION) $(INSTALL_DIR)/$(GMLIB).$(STATIC_SUFFIX)
	@ln -s $(INSTALL_DIR)/$(GMLIB).so.$(VERSION) $(INSTALL_DIR)/$(GMLIB).so

uninstall:
	@rm -f $(INSTALL_DIR)/$(GMLIB).$(STATIC_SUFFIX) $(INSTALL_DIR)/$(GMLIB).$(STATIC_SUFFIX.$(VERSION)
	@rm -f $(INSTALL_DIR)/$(GMLIB).$(SHARED_SUFFIX) $(INSTALL_DIR)/$(GMLIB).$(SHARED_SUFFIX.$(VERSION)
	@rm -rf $(INCLUDE_DIR)

python:
	@rm -f python3.6m.zip
	@cd $(PYTHON_LIB)/Lib; zip -r python3.6m.zip $(GM_EMBEDDED_MODULES)
	@mv $(PYTHON_LIB)/Lib/python3.6m.zip python3.6m.zip

$(OBJDIR)/%.o: %.cpp
	@$(CXX) $(CFLAGS) -c $< -o $@
	@echo "CC $<"

clean:
	find . -type f -name \*.o -exec rm {} \;
	rm -rf $(GMLIB).$(STATIC_SUFFIX) $(GMLIB).$(SHARED_SUFFIX) $(OBJDIR)

.PHONY: all clean
