NDK := ~/Projects-on-SSD/android_aarch64_ndk_linux-x86_64

BIN_GCC := gcc
BIN_CLANG := clang
BIN_WINDOWS_X86_CC := x86_64-w64-mingw32-gcc
BIN_ANDROID_CC := $(NDK)/bin/aarch64-linux-android23-clang

USER_WIN := $(shell echo %USER%)
USER_LINUX := $(shell whoami)

INST_PATH_WIN := C:\\Users\\Program\ Files\ \(x86\)\\reval

INST_PATH_LINUX := /usr/bin

BLD_LOGS := 0

CLANG_USING := 1
ASAN_USING := 0
DEBUG := 0

STATIC := 0

ifeq ($(BLD_LOGS), 0)
	GCC := @$(BIN_GCC)
	CLANG := @$(BIN_CLANG)
	WINDOWS_X86_CC := @$(BIN_WINDOWS_X86_CC)
	ANDROID_CC := @$(BIN_ANDROID_CC)
else
	GCC := $(BIN_GCC)
	CLANG := $(BIN_CLANG)
	WINDOWS_X86_CC := $(BIN_WINDOWS_X86_CC)
	ANDROID_CC := $(BIN_ANDROID_CC)
endif

ifeq ($(ASAN_USING), 1)
	DEBUG := 1
endif

BASE_CFLAGS := \
	-Werror=sign-compare -Werror=bool-operation -Werror=char-subscripts \
	-Werror=return-type -Werror=int-in-bool-context -Wno-unused-parameter \
	-Werror=uninitialized -Werror=init-self -Werror=logical-not-parentheses \
	-Wmemset-transposed-args -Wmisleading-indentation \
	-Werror=implicit-function-declaration -Werror=address -Werror=type-limits \
	-Werror=shadow -Werror=pointer-arith -Werror=cast-align -Werror=float-conversion \
	-Werror=undef -Werror=nonnull -Wparentheses -Werror=sequence-point \
	-Werror=sizeof-pointer-div -Wsizeof-pointer-memaccess \
	-Wswitch -Werror=tautological-compare -Werror=trigraphs -Wunused-function \
	-Werror=empty-body -Wimplicit-fallthrough \
	-Werror=shift-negative-value -Werror=unused-but-set-parameter \
	-Waddress-of-packed-member -Werror=format -funsigned-char \
	-Werror=implicit-fallthrough -Werror=strict-aliasing

LINK_CFLAGS := 

ifeq ($(ASAN_USING), 1)
	BASE_CFLAGS += -fsanitize=address -fsanitize=leak -fsanitize=undefined -g

	LINK_CFLAGS += -fsanitize=address -fsanitize=leak -fsanitize=undefined -g
endif

ifeq ($(DEBUG), 1)
	BASE_CFLAGS += -g
endif

ifeq ($(STATIC), 1)
	BASE_CFLAGS += -static

	LINK_CFLAGS += -static
endif

CLANG_CFLAGS := -Werror=unknown-warning-option -Wall -Wextra -Werror=array-bounds $(BASE_CFLAGS)

GCC_CFLAGS := -Werror=pragmas -Wall -Wextra -Werror=array-bounds=1 -Werror=bool-compare $(BASE_CFLAGS) \
					-Wmemset-elt-size -Werror=multistatement-macros -Werror=maybe-uninitialized \
					-Werror=nonnull-compare

REVAL_SRCFILES := $(shell find sources -name "*.c")

REVAL_OBJFILES := $(REVAL_SRCFILES:.c=.o)

REVAL_LINUX_OBJFILES := \
	$(addprefix obj/linux_x86/, $(REVAL_OBJFILES))

REVAL_LINUX_AARCH64_OBJFILES := \
	$(addprefix obj/linux_aarch64/, $(REVAL_OBJFILES))

REVAL_ANDROID_AARCH64_OBJFILES := \
	$(addprefix obj/android_aarch64/, $(REVAL_OBJFILES))

REVAL_WINDOWS_OBJFILES := \
	$(addprefix obj/windows_x86/, $(REVAL_OBJFILES))

.SUFFIXES:

all: release_reval_all clean

release_reval_all: reval_linux_x86 clean reval_win_x86 clean reval_linux_aarch64 clean reval_android_aarch64 clean 

reval_linux_x86_all: reval_linux_x86 clean
# 	@./reval_linux_x86 program.bin

reval_linux_aarch64_all: reval_linux_aarch64 clean
# 	@./reval_linux_aarch64 program.bin

reval_android_aarch64_all: reval_android_aarch64 clean
# 	@./reval_android_aarch64 program.bin

reval_win_x86_all: reval_win_x86 clean
# 	@wine reval_win_x86.exe program.bin

install_linux: # reval_linux_x86_all
	@echo "Installing for $(shell uname -s) $(shell uname -r) to $(INST_PATH_LINUX)..."

ifneq ($(shell whoami), root)
	@echo "Installation failed: not runned as root, try again running as root"

	@exit 1
endif

	@cp reval_linux_x86 $(INST_PATH_LINUX)/reval

uninstall_linux:
	@echo "Uninstalling for $(shell uname -s) $(shell uname -r) from $(INST_PATH_LINUX)..."

ifneq ($(shell whoami), root)
	@echo "Uninstallation failed: not runned as root, try again running as root"

	@exit 1
endif
ifeq ($(wildcard $(INST_PATH_LINUX)/reval),)
	@echo "Uninstallation failed: not installed"

	@exit 1
endif
	@rm $(INST_PATH_LINUX)/reval

install_win: # reval_win_x86_all
	@echo "Installing for $(shell ver) $(USER_WIN) to $(INST_PATH_WIN)..."

	@cp reval_win_x86.exe $(INST_PATH_WIN)\\reval.exe

uninstall_win:
	@echo "Uninstalling for $(shell ver) from $(INST_PATH_WIN)..."

ifeq ($(wildcard $(INST_PATH_WIN)/reval.exe),)
	@echo "Uninstallation failed: not installed"

	@exit 1
endif
	@rd /s /q $(INST_PATH_WIN)

reval_win_x86: $(REVAL_WINDOWS_OBJFILES)
	@echo using $(BIN_WINDOWS_X86_CC) \"$(shell which $(BIN_WINDOWS_X86_CC))\" with flags \"$(LINK_CFLAGS)\" for linking and compile

	$(WINDOWS_X86_CC) $(LINK_CFLAGS) $^ -o $@

	@echo "Builded $@"

reval_linux_x86: $(REVAL_LINUX_OBJFILES)
ifeq ($(CLANG_USING), 1)
	@echo using clang \"$(shell which $(BIN_CLANG))\" with flags \"$(LINK_CFLAGS)\" for linking and compile
	
	$(CLANG) $(LINK_CFLAGS) $^ -o $@
else
	@echo using gcc \"$(shell which $(BIN_GCC))\" with flags \"$(LINK_CFLAGS)\" for linking and compile

	$(GCC) $(LINK_CFLAGS) $^ -o $@
endif
	@echo "Builded $@"

reval_linux_aarch64: $(REVAL_LINUX_AARCH64_OBJFILES)
	@echo using clang \"$(shell which $(BIN_CLANG))\" with flags \"$(LINK_CFLAGS)\" for linking and compile

	$(CLANG) $(LINK_CFLAGS) --target=aarch64-linux-gnu $^ -o $@

	@echo "Builded $@"

reval_android_aarch64: $(REVAL_ANDROID_AARCH64_OBJFILES)
	@echo using NDK Android Clang \"$(shell which $(BIN_ANDROID_CC))\" with flags \"$(LINK_CFLAGS)\" for linking and compile

	$(ANDROID_CC) $(LINK_CFLAGS) $^ -o $@

	@echo "Builded $@"

obj/linux_x86/%.o: %.c
	@mkdir -p $(dir $@)

ifeq ($(CLANG_USING), 1)
	$(CLANG) -Iinclude $(CLANG_CFLAGS) -o $@ -c $^
else
	$(GCC) -Iinclude $(GCC_CFLAGS) -o $@ -c $^
endif

obj/windows_x86/%.o: %.c
	@mkdir -p $(dir $@)

	$(WINDOWS_X86_CC) -Iinclude $(GCC_CFLAGS) -o $@ -c $^

obj/linux_aarch64/%.o: %.c
	@mkdir -p $(dir $@)

	$(CLANG) --target=aarch64-linux-gnu -Iinclude $(CLANG_CFLAGS) -o $@ -c $^

obj/android_aarch64/%.o: %.c
	@mkdir -p $(dir $@)

	$(ANDROID_CC) -Iinclude $(CLANG_CFLAGS) -o $@ -c $^

clean:
	@rm -f $(REVAL_LINUX_AARCH64_OBJFILES) $(REVAL_ANDROID_AARCH64_OBJFILES) $(REVAL_LINUX_OBJFILES) $(REVAL_WINDOWS_OBJFILES)

clean_all: clean
	@rm -f reval_win_x86.exe reval_linux_x86 reval_linux_aarch64 reval_android_aarch64
