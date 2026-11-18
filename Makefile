#
#	Makefile template
#
#	This is an example Makefile that can be used by anyone who is building
#	his or her own PHP extensions using the PHP-CPP library. 
#
#	In the top part of this file we have included variables that can be
#	altered to fit your configuration, near the bottom the instructions and
#	dependencies for the compiler are defined. The deeper you get into this
#	file, the less likely it is that you will have to change anything in it.
#

#
#	Name of your extension
#
#	This is the name of your extension. Based on this extension name, the
#	name of the library file (name.so) and the name of the config file (name.ini)
#	are automatically generated
#

NAME	= pdf
VERSION	= 0.12.0
PROJECT_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
SOURCE_DIR := $(PROJECT_DIR)src
PHP_CONFIG ?= php-config
PHP ?= $(or $(shell "$(PHP_CONFIG)" --php-binary 2>/dev/null),php)
PHP_EXTENSION_DIR = $(shell "$(PHP_CONFIG)" --extension-dir 2>/dev/null)
RUN_TESTS ?= $(firstword $(wildcard $(PHP_EXTENSION_DIR)/../build/run-tests.php $(PHP_EXTENSION_DIR)/build/run-tests.php))

#
#	Php.ini directories
#
#	In the past, PHP used a single php.ini configuration file. Today, most
#	PHP installations use a conf.d directory that holds a set of config files,
#	one for each extension. Use this variable to specify this directory.
#

INI_DIR	=	$(shell "$(PHP_CONFIG)" --ini-dir)

#
#	The extension dirs
#
#	This is normally a directory like /usr/lib/php5/20121221 (based on the 
#	PHP version that you use. We make use of the command line 'php-config' 
#	instruction to find out what the extension directory is, you can override
#	this with a different fixed directory
#

EXTENSION_DIR	=	$(PHP_EXTENSION_DIR)


#
#	The name of the extension and the name of the .ini file
#
#	These two variables are based on the name of the extension. We simply add
#	a certain extension to them (.so or .ini)
#

EXTENSION 	=	${NAME}.so
INI 		=	${NAME}.ini


#
#	Compiler
#
#	By default, the GNU C++ compiler is used. If you want to use a different
#	compiler, you can change that here. You can change this for both the 
#	compiler (the program that turns the c++ files into object files) and for
#	the linker (the program that links all object files into the single .so
#	library file. By default, g++ (the GNU C++ compiler) is used for both.
#

COMPILER	=	g++
LINKER		=	g++


#
#	Compiler and linker flags
#
#	This variable holds the flags that are passed to the compiler. By default, 
# 	we include the -O2 flag. This flag tells the compiler to optimize the code, 
#	but it makes debugging more difficult. So if you're debugging your application, 
#	you probably want to remove this -O2 flag. At the same time, you can then 
#	add the -g flag to instruct the compiler to include debug information in
#	the library (but this will make the final libphpcpp.so file much bigger, so
#	you want to leave that flag out on production servers).
#
#	If your extension depends on other libraries (and it does at least depend on
#	one: the PHP-CPP library), you should update the LINKER_DEPENDENCIES variable
#	with a list of all flags that should be passed to the linker.
#
POPPLER_HAS_JS := $(shell grep -qn has_js /usr/include/poppler/cpp/poppler-document.h && echo 1 || echo 0)
POPPLER_HAS_NEW_DATE_T := $(shell grep -qn info_date_t /usr/include/poppler/cpp/poppler-document.h && echo 1 || echo 0)

COMPILER_DEFINES    = -DVERSION=\"${VERSION}\" -DPOPPLER_HAS_NEW_DATE_T=${POPPLER_HAS_NEW_DATE_T} -DPOPPLER_HAS_JS=${POPPLER_HAS_JS}
COMPILER_FLAGS		= -Wall -g -O2 -std=c++11 -fPIC `pkg-config poppler-cpp fontconfig openssl --cflags` $(CFLAGS) $(COMPILER_DEFINES)
LINKER_FLAGS		= -shared
LINKER_DEPENDENCIES	= $(LIBFLAGS) -lphpcpp -lPDFWriter `pkg-config poppler-cpp fontconfig openssl --libs`


#
#	Command to remove files, copy files and create directories.
#
#	I've never encountered a *nix environment in which these commands do not work. 
#	So you can probably leave this as it is
#

RM		=	rm -f
CP		=	cp -f
MKDIR	=	mkdir -p


BUILD_DIR := .build
SOURCES := $(wildcard $(SOURCE_DIR)/*.cpp)
OBJECTS := $(patsubst $(SOURCE_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))
DEPFILES := $(OBJECTS:.o=.d)

#
#	From here the build instructions start
#

.PHONY: all test install uninstall clean dist
.DELETE_ON_ERROR:

all: ${EXTENSION}

test:
	+@set -eu; \
	if [ ! -r "$(RUN_TESTS)" ]; then \
		echo "PHP's run-tests.php was not found. Set RUN_TESTS=/path/to/run-tests.php." >&2; \
		exit 1; \
	fi; \
	test_dir=$$(mktemp -d); \
	trap 'rm -rf "$$test_dir"' EXIT; \
	$(MAKE) -C "$$test_dir" -f "$(PROJECT_DIR)Makefile" all; \
	cp -R "$(PROJECT_DIR)tests" "$$test_dir/tests"; \
	cp "$(RUN_TESTS)" "$$test_dir/run-tests.php"; \
	extension="$$test_dir/$(EXTENSION)"; \
	php=$$(command -v "$(PHP)"); \
	case "$$php" in /*) ;; *) php="$(CURDIR)/$$php";; esac; \
	cd "$$test_dir"; \
	"$$php" -n -d "extension=$$extension" -r 'exit(extension_loaded("pdf") ? 0 : 1);'; \
	REPORT_EXIT_STATUS=1 TEST_PHP_SRCDIR="$$test_dir" "$$php" -n run-tests.php -p "$$php" \
		-q -n -d "extension=$$extension" --show-diff tests

${EXTENSION}:	${OBJECTS}
	${LINKER} ${LINKER_FLAGS} -o $@ ${OBJECTS} ${LINKER_DEPENDENCIES}

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(COMPILER_FLAGS) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -MMD -MP -MF "$(@:.o=.d)" -c -o "$@" "$<"

$(BUILD_DIR):
	$(MKDIR) "$@"

install:		
	${CP} ${EXTENSION} $(DESTDIR)/${EXTENSION_DIR}
	${CP} ${INI} $(DESTDIR)/${INI_DIR}/60-${INI}

uninstall:
	rm $(DESTDIR)/${EXTENSION_DIR}/${EXTENSION}
	rm $(DESTDIR)/${INI_DIR}/60-${INI}

clean:
	${RM} ${EXTENSION} ${OBJECTS} ${DEPFILES}

dist: clean
	tar --exclude-vcs-ignores --exclude-vcs --exclude=test.php --exclude=*tar.bz2 --transform 's,^\.,php-${NAME}-${VERSION},' -cjf ../php-${NAME}-${VERSION}.tar.bz2 .
	cp ../php-${NAME}-${VERSION}.tar.bz2 .

-include $(DEPFILES)
