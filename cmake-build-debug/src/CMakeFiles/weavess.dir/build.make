# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.17

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


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

# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = C:\Users\Murph\AppData\Local\JetBrains\Toolbox\apps\CLion\ch-0\202.6397.106\bin\cmake\win\bin\cmake.exe

# The command to remove a file.
RM = C:\Users\Murph\AppData\Local\JetBrains\Toolbox\apps\CLion\ch-0\202.6397.106\bin\cmake\win\bin\cmake.exe -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = F:\ANNS\WEAVESS_v1.0

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = F:\ANNS\WEAVESS_v1.0\cmake-build-debug

# Include any dependencies generated for this target.
include src/CMakeFiles/weavess.dir/depend.make

# Include the progress variables for this target.
include src/CMakeFiles/weavess.dir/progress.make

# Include the compile flags for this target's objects.
include src/CMakeFiles/weavess.dir/flags.make

src/CMakeFiles/weavess.dir/index.cpp.obj: src/CMakeFiles/weavess.dir/flags.make
src/CMakeFiles/weavess.dir/index.cpp.obj: src/CMakeFiles/weavess.dir/includes_CXX.rsp
src/CMakeFiles/weavess.dir/index.cpp.obj: ../src/index.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=F:\ANNS\WEAVESS_v1.0\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/CMakeFiles/weavess.dir/index.cpp.obj"
	cd /d F:\ANNS\WEAVESS_v1.0\cmake-build-debug\src && F:\MinGW-W64\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\weavess.dir\index.cpp.obj -c F:\ANNS\WEAVESS_v1.0\src\index.cpp

src/CMakeFiles/weavess.dir/index.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/weavess.dir/index.cpp.i"
	cd /d F:\ANNS\WEAVESS_v1.0\cmake-build-debug\src && F:\MinGW-W64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E F:\ANNS\WEAVESS_v1.0\src\index.cpp > CMakeFiles\weavess.dir\index.cpp.i

src/CMakeFiles/weavess.dir/index.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/weavess.dir/index.cpp.s"
	cd /d F:\ANNS\WEAVESS_v1.0\cmake-build-debug\src && F:\MinGW-W64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S F:\ANNS\WEAVESS_v1.0\src\index.cpp -o CMakeFiles\weavess.dir\index.cpp.s

src/CMakeFiles/weavess.dir/index_component_coarse.h.cpp.obj: src/CMakeFiles/weavess.dir/flags.make
src/CMakeFiles/weavess.dir/index_component_coarse.h.cpp.obj: src/CMakeFiles/weavess.dir/includes_CXX.rsp
src/CMakeFiles/weavess.dir/index_component_coarse.h.cpp.obj: ../src/index_component_coarse.h.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=F:\ANNS\WEAVESS_v1.0\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/CMakeFiles/weavess.dir/index_component_coarse.h.cpp.obj"
	cd /d F:\ANNS\WEAVESS_v1.0\cmake-build-debug\src && F:\MinGW-W64\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\weavess.dir\index_component_coarse.h.cpp.obj -c F:\ANNS\WEAVESS_v1.0\src\index_component_coarse.h.cpp

src/CMakeFiles/weavess.dir/index_component_coarse.h.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/weavess.dir/index_component_coarse.h.cpp.i"
	cd /d F:\ANNS\WEAVESS_v1.0\cmake-build-debug\src && F:\MinGW-W64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E F:\ANNS\WEAVESS_v1.0\src\index_component_coarse.h.cpp > CMakeFiles\weavess.dir\index_component_coarse.h.cpp.i

src/CMakeFiles/weavess.dir/index_component_coarse.h.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/weavess.dir/index_component_coarse.h.cpp.s"
	cd /d F:\ANNS\WEAVESS_v1.0\cmake-build-debug\src && F:\MinGW-W64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S F:\ANNS\WEAVESS_v1.0\src\index_component_coarse.h.cpp -o CMakeFiles\weavess.dir\index_component_coarse.h.cpp.s

# Object files for target weavess
weavess_OBJECTS = \
"CMakeFiles/weavess.dir/index.cpp.obj" \
"CMakeFiles/weavess.dir/index_component_coarse.h.cpp.obj"

# External object files for target weavess
weavess_EXTERNAL_OBJECTS =

src/libweavess.a: src/CMakeFiles/weavess.dir/index.cpp.obj
src/libweavess.a: src/CMakeFiles/weavess.dir/index_component_coarse.h.cpp.obj
src/libweavess.a: src/CMakeFiles/weavess.dir/build.make
src/libweavess.a: src/CMakeFiles/weavess.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=F:\ANNS\WEAVESS_v1.0\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX static library libweavess.a"
	cd /d F:\ANNS\WEAVESS_v1.0\cmake-build-debug\src && $(CMAKE_COMMAND) -P CMakeFiles\weavess.dir\cmake_clean_target.cmake
	cd /d F:\ANNS\WEAVESS_v1.0\cmake-build-debug\src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\weavess.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/CMakeFiles/weavess.dir/build: src/libweavess.a

.PHONY : src/CMakeFiles/weavess.dir/build

src/CMakeFiles/weavess.dir/clean:
	cd /d F:\ANNS\WEAVESS_v1.0\cmake-build-debug\src && $(CMAKE_COMMAND) -P CMakeFiles\weavess.dir\cmake_clean.cmake
.PHONY : src/CMakeFiles/weavess.dir/clean

src/CMakeFiles/weavess.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" F:\ANNS\WEAVESS_v1.0 F:\ANNS\WEAVESS_v1.0\src F:\ANNS\WEAVESS_v1.0\cmake-build-debug F:\ANNS\WEAVESS_v1.0\cmake-build-debug\src F:\ANNS\WEAVESS_v1.0\cmake-build-debug\src\CMakeFiles\weavess.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : src/CMakeFiles/weavess.dir/depend

