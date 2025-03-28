# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.24

# Default target executed when no arguments are given to make.
default_target: all
.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/local/bin/cmake

# The command to remove a file.
RM = /opt/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/om/C/6502

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/om/C/6502

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake cache editor..."
	/opt/local/bin/ccmake -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache
.PHONY : edit_cache/fast

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/opt/local/bin/cmake --regenerate-during-build -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache
.PHONY : rebuild_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /Users/om/C/6502/CMakeFiles /Users/om/C/6502//CMakeFiles/progress.marks
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /Users/om/C/6502/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean
.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named 6502

# Build rule for target.
6502: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 6502
.PHONY : 6502

# fast build rule for target.
6502/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/6502.dir/build.make CMakeFiles/6502.dir/build
.PHONY : 6502/fast

deps/cjson/cJSON.o: deps/cjson/cJSON.c.o
.PHONY : deps/cjson/cJSON.o

# target to build an object file
deps/cjson/cJSON.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/6502.dir/build.make CMakeFiles/6502.dir/deps/cjson/cJSON.c.o
.PHONY : deps/cjson/cJSON.c.o

deps/cjson/cJSON.i: deps/cjson/cJSON.c.i
.PHONY : deps/cjson/cJSON.i

# target to preprocess a source file
deps/cjson/cJSON.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/6502.dir/build.make CMakeFiles/6502.dir/deps/cjson/cJSON.c.i
.PHONY : deps/cjson/cJSON.c.i

deps/cjson/cJSON.s: deps/cjson/cJSON.c.s
.PHONY : deps/cjson/cJSON.s

# target to generate assembly for a file
deps/cjson/cJSON.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/6502.dir/build.make CMakeFiles/6502.dir/deps/cjson/cJSON.c.s
.PHONY : deps/cjson/cJSON.c.s

src/lib/cpu.o: src/lib/cpu.c.o
.PHONY : src/lib/cpu.o

# target to build an object file
src/lib/cpu.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/6502.dir/build.make CMakeFiles/6502.dir/src/lib/cpu.c.o
.PHONY : src/lib/cpu.c.o

src/lib/cpu.i: src/lib/cpu.c.i
.PHONY : src/lib/cpu.i

# target to preprocess a source file
src/lib/cpu.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/6502.dir/build.make CMakeFiles/6502.dir/src/lib/cpu.c.i
.PHONY : src/lib/cpu.c.i

src/lib/cpu.s: src/lib/cpu.c.s
.PHONY : src/lib/cpu.s

# target to generate assembly for a file
src/lib/cpu.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/6502.dir/build.make CMakeFiles/6502.dir/src/lib/cpu.c.s
.PHONY : src/lib/cpu.c.s

src/test/emulator.o: src/test/emulator.c.o
.PHONY : src/test/emulator.o

# target to build an object file
src/test/emulator.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/6502.dir/build.make CMakeFiles/6502.dir/src/test/emulator.c.o
.PHONY : src/test/emulator.c.o

src/test/emulator.i: src/test/emulator.c.i
.PHONY : src/test/emulator.i

# target to preprocess a source file
src/test/emulator.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/6502.dir/build.make CMakeFiles/6502.dir/src/test/emulator.c.i
.PHONY : src/test/emulator.c.i

src/test/emulator.s: src/test/emulator.c.s
.PHONY : src/test/emulator.s

# target to generate assembly for a file
src/test/emulator.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/6502.dir/build.make CMakeFiles/6502.dir/src/test/emulator.c.s
.PHONY : src/test/emulator.c.s

src/test/main.o: src/test/main.c.o
.PHONY : src/test/main.o

# target to build an object file
src/test/main.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/6502.dir/build.make CMakeFiles/6502.dir/src/test/main.c.o
.PHONY : src/test/main.c.o

src/test/main.i: src/test/main.c.i
.PHONY : src/test/main.i

# target to preprocess a source file
src/test/main.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/6502.dir/build.make CMakeFiles/6502.dir/src/test/main.c.i
.PHONY : src/test/main.c.i

src/test/main.s: src/test/main.c.s
.PHONY : src/test/main.s

# target to generate assembly for a file
src/test/main.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/6502.dir/build.make CMakeFiles/6502.dir/src/test/main.c.s
.PHONY : src/test/main.c.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... edit_cache"
	@echo "... rebuild_cache"
	@echo "... 6502"
	@echo "... deps/cjson/cJSON.o"
	@echo "... deps/cjson/cJSON.i"
	@echo "... deps/cjson/cJSON.s"
	@echo "... src/lib/cpu.o"
	@echo "... src/lib/cpu.i"
	@echo "... src/lib/cpu.s"
	@echo "... src/test/emulator.o"
	@echo "... src/test/emulator.i"
	@echo "... src/test/emulator.s"
	@echo "... src/test/main.o"
	@echo "... src/test/main.i"
	@echo "... src/test/main.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

