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
	CFLAGS+=-g -DGM_UIDEBUG -DGM_DEBUG_MULTITEXTURE
	NOOPT=1
endif

# User defined paths to includes
ifneq ("$(PYTHON_INCLUDE)", "")
	CFLAGS +=-I$(PYTHON_INCLUDE)
else
	echo "error: PYTHON_INCLUDE is not set"
	exit 1
endif
ifneq ("$(BOOST_INCLUDE)", "")
	CFLAGS +=-I$(BOOST_INCLUDE)
else
	echo "error: BOOST_INCLUDE is not set"
	exit 1
endif
ifneq ("$(FREETYPE_INCLUDE)", "")
	CFLAGS +=-I$(FREETYPE_INCLUDE)
else
	echo "error: FREETYPE_INCLUDE is not set"
	exit 1
endif
ifneq ("$(SDL2_INCLUDE)", "")
	CFLAGS +=-I$(SDL2_INCLUDE)
else
	echo "error: SDL2_INCLUDE is not set"
	exit 1
endif
ifneq ("$(SDL2_IMAGE_INCLUDE)", "")
	CFLAGS +=-I$(SDL2_IMAGE_INCLUDE)
else
	echo "error: SDL2_IMAGE_INCLUDE is not set"
	exit 1
endif
ifneq ("$(SDL2_TTF_INCLUDE)", "")
	CFLAGS +=-I$(SDL2_TTF_INCLUDE)
else
	echo "error: SDL2_TTF_INCLUDE is not set"
	exit 1
endif
ifneq ("$(JANSSON_INCLUDE)", "")
	CFLAGS +=-I$(JANSSON_INCLUDE)
else
	echo "error: JANSSON_INCLUDE is not set"
	exit 1
endif

# User defined paths to libraries
ifneq ("$(PYTHON_LIB)", "")
	LDFLAGS +=-L$(PYTHON_LIB) \
              -lpython3.6m
else
	echo "error: PYTHON_LIB is not set"
	exit 1
endif
ifneq ("$(BOOST_LIB)", "")
	LDFLAGS +=-L$(BOOST_LIB) \
	          -lboost_filesystem \
	          -lboost_system
else
	echo "error: BOOST_LIB is not set"
	exit 1
endif
ifneq ("$(FREETYPE_LIB)", "")
	LDFLAGS +=-L$(FREETYPE_LIB) \
			  -lfreetype
else
	echo "error: FREETYPE_LIB is not set"
	exit 1
endif
ifneq ("$(SDL2_LIB)", "")
	LDFLAGS +=-L$(SDL2_LIB) \
			  -lSDL2
else
	echo "error: SDL2_LIB is not set"
	exit 1
endif
ifneq ("$(SDL2_IMAGE_LIB)", "")
	LDFLAGS +=-L$(SDL2_IMAGE_LIB) \
			  -lSDL2_image
else
	echo "error: SDL2_IMAGE_LIB is not set"
	exit 1
endif
ifneq ("$(SDL2_TTF_LIB)", "")
	LDFLAGS +=-L$(SDL2_TTF_LIB) \
			  -lSDL2_ttf
else
	echo "error: SDL2_TTF_LIB is not set"
	exit 1
endif
ifneq ("$(JANSSON_LIB)", "")
	LDFLAGS +=-L$(JANSSON_LIB) \
			  -ljansson
else
	echo "error: JANSSON_LIB is not set"
	exit 1
endif