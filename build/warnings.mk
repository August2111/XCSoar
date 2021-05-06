WARNINGS = -Wall -Wextra
WARNINGS += -Wwrite-strings -Wcast-qual -Wpointer-arith -Wsign-compare
WARNINGS += -Wundef
WARNINGS += -Wmissing-declarations
WARNINGS += -Wredundant-decls

CXXFLAGS += $(WARNINGS)
CXXFLAGS += -Wmissing-noreturn
CXXFLAGS += -Wvla

# disable some warnings, we're not ready for them yet
CXXFLAGS += -Wno-unused-parameter
CXXFLAGS += -Wno-missing-field-initializers 
# disabled by August2111 (Json standalone):
CXXFLAGS += -Wno-cpp 
CXXFLAGS += -Wcast-align

# plain C warnings

CFLAGS += $(WARNINGS)
CFLAGS += -Wmissing-prototypes -Wstrict-prototypes
CFLAGS += -Wnested-externs

# make warnings fatal (for perfectionists)

WERROR ?= $(DEBUG)

ifeq ($(WERROR),y)
CXXFLAGS += -Werror
CFLAGS += -Werror
endif

#CXXFLAGS += -pedantic
#CXXFLAGS += -pedantic-errors

# -Wdisabled-optimization
# -Wunused -Wshadow -Wunreachable-code
# -Wfloat-equal
