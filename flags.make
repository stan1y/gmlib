# GMLib Makefile
#
# Build flags supported:
# SDL2_INCLUDE 
# SDL2_IMAGE_INCLUDE
# SDL2_TFF_INCLUDE
# BOOST_INCLUDE
# PYTHON_INCLUDE
# JANSSON_INCLUDE
# FREETYPE_INCLUDE
#
# SDL2_LIB
# SDL2_IMAGE_LIB
# SDL2_TTF_LIB
# BOOST_LIB
# JANSSON_LIB
# PYTHON_LIB
# FREETYPE_LIB

CXX?=c++
PREFIX?=/usr/local
OSNAME=$(shell uname -s | sed -e 's/[-_].*//g' | tr A-Z a-z)
STATIC_SUFFIX=a
SHARED_SUFFIX=so
OBJDIR?=.obj

ifeq ("$(OSNAME)", "darwin")
    SHARED_SUFFIX=dylib
endif

# Debug build
ifneq ("$(DEBUG)", "")
    CFLAGS+=-g -D GM_DEBUG
endif

# User defined paths to includes
ifneq ("$(PYTHON_SRC)", "")
    CFLAGS += -I $(PYTHON_SRC)/Include
else
    $(error PYTHON_SRC is not set)
endif
ifneq ("$(ZLIB_SRC)", "")
    CFLAGS += -I $(ZLIB_SRC)
else
    $(error ZLIB_SRC is not set)
endif
ifneq ("$(BOOST_INCLUDE)", "")
    CFLAGS +=-I $(BOOST_INCLUDE)
else
    $(error BOOST_INCLUDE is not set)
endif
ifneq ("$(FREETYPE_INCLUDE)", "")
    CFLAGS +=-I $(FREETYPE_INCLUDE)
else
    $(error FREETYPE_INCLUDE is not set)
endif
ifneq ("$(SDL2_INCLUDE)", "")
    CFLAGS +=-I $(SDL2_INCLUDE)
else
    $(error  SDL2_INCLUDE is not set)
endif
ifneq ("$(SDL2_IMAGE_INCLUDE)", "")
    CFLAGS +=-I $(SDL2_IMAGE_INCLUDE)
else
    $(error  SDL2_IMAGE_INCLUDE is not set)
endif
ifneq ("$(SDL2_TTF_INCLUDE)", "")
    CFLAGS +=-I $(SDL2_TTF_INCLUDE)
else
    $(error  SDL2_TTF_INCLUDE is not set)
endif
ifneq ("$(JSON_INCLUDE)", "")
    CFLAGS +=-I $(JSON_INCLUDE)
else
    $(error  JSON_INCLUDE is not set)
endif

# User defined paths to libraries
ifneq ("$(PYTHON_SRC)", "")
    LDFLAGS +=-L $(PYTHON_SRC) \
              -l python3.6m
else
    $(error  PYTHON_SRC is not set)
endif
ifneq ("$(ZLIB_SRC)", "")
    LDFLAGS +=-L $(ZLIB_SRC) \
              -l z
else
    $(error  PYTHON_SRC is not set)
endif
ifneq ("$(BOOST_LIB)", "")
    LDFLAGS +=-L $(BOOST_LIB) \
              -l boost_filesystem \
              -l boost_system
else
    $(error  BOOST_LIB is not set)
endif
ifneq ("$(FREETYPE_LIB)", "")
    LDFLAGS +=-L $(FREETYPE_LIB) \
              -l freetype
else
    $(error  FREETYPE_LIB is not set)
endif
ifneq ("$(SDL2_LIB)", "")
    LDFLAGS +=-L $(SDL2_LIB) \
              -l SDL2
else
    $(error  SDL2_LIB is not set)
endif
ifneq ("$(SDL2_IMAGE_LIB)", "")
    LDFLAGS +=-L $(SDL2_IMAGE_LIB) \
              -l SDL2_image
else
    $(error  SDL2_IMAGE_LIB is not set)
endif
ifneq ("$(SDL2_TTF_LIB)", "")
    LDFLAGS +=-L $(SDL2_TTF_LIB) \
              -l SDL2_ttf
else
    $(error  SDL2_TTF_LIB is not set)
endif
