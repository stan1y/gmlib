# GMLib Build 

CC?=c++
PREFIX?=/usr/local
OBJDIR?=obj
GMLIB=libgm
INSTALL_DIR=$(PREFIX)/lib/
INCLUDE_DIR=$(PREFIX)/include/gmlib

S_SRC= $(wildcard *.cpp)

CFLAGS+=-Wall -Werror -std=c++11 -pedantic
CFLAGS+=-Wsign-compare -Wshadow -Wpointer-arith -Wcast-qual
CFLAGS+=-DPREFIX='"$(PREFIX)"'
CFLAGS+=-I. -I/usr/local/include
LDFLAGS=-shared

# User defined paths to includes
ifneq ("$(BOOST_INCLUDE)", "")
	CFLAGS +=-I$(BOOST_INCLUDE)
endif
ifneq ("$(SDL_INCLUDE)", "")
	CFLAGS +=-I$(SDL_INCLUDE)
endif
ifneq ("$(SDL_IMAGE_INCLUDE)", "")
	CFLAGS +=-I$(SDL_IMAGE_INCLUDE)
endif
ifneq ("$(SDL_TTF_INCLUDE)", "")
	CFLAGS +=-I$(SDL_TTF_INCLUDE)
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

$(GMLIB): $(OBJDIR) $(S_OBJS)
	$(CC) $(S_OBJS) $(LDFLAGS) -o $(GMLIB)

objects: $(OBJDIR) $(S_OBJS)

all: $(GMLIB)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

install:
	mkdir -p $(INCLUDE_DIR)
	mkdir -p $(INSTALL_DIR)
	install -m 555 $(GMLIB) $(INSTALL_DIR)/$(GMLIB)
	install -m 644 *.h $(INCLUDE_DIR)

uninstall:
	rm -f $(INSTALL_DIR)/$(GMLIB)
	rm -rf $(INCLUDE_DIR)

$(OBJDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	find . -type f -name \*.o -exec rm {} \;
	rm -rf $(GMLIB) $(OBJDIR)

.PHONY: all clean
