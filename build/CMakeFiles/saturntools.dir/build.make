# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.24

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
CMAKE_COMMAND = /usr/local/cmake/bin/cmake

# The command to remove a file.
RM = /usr/local/cmake/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/txkj/cpp_test

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/txkj/cpp_test/build

# Include any dependencies generated for this target.
include CMakeFiles/saturntools.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/saturntools.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/saturntools.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/saturntools.dir/flags.make

CMakeFiles/saturntools.dir/src/deteObj.cpp.o: CMakeFiles/saturntools.dir/flags.make
CMakeFiles/saturntools.dir/src/deteObj.cpp.o: /home/txkj/cpp_test/src/deteObj.cpp
CMakeFiles/saturntools.dir/src/deteObj.cpp.o: CMakeFiles/saturntools.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/txkj/cpp_test/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/saturntools.dir/src/deteObj.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/saturntools.dir/src/deteObj.cpp.o -MF CMakeFiles/saturntools.dir/src/deteObj.cpp.o.d -o CMakeFiles/saturntools.dir/src/deteObj.cpp.o -c /home/txkj/cpp_test/src/deteObj.cpp

CMakeFiles/saturntools.dir/src/deteObj.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/saturntools.dir/src/deteObj.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/txkj/cpp_test/src/deteObj.cpp > CMakeFiles/saturntools.dir/src/deteObj.cpp.i

CMakeFiles/saturntools.dir/src/deteObj.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/saturntools.dir/src/deteObj.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/txkj/cpp_test/src/deteObj.cpp -o CMakeFiles/saturntools.dir/src/deteObj.cpp.s

CMakeFiles/saturntools.dir/src/deteRes.cpp.o: CMakeFiles/saturntools.dir/flags.make
CMakeFiles/saturntools.dir/src/deteRes.cpp.o: /home/txkj/cpp_test/src/deteRes.cpp
CMakeFiles/saturntools.dir/src/deteRes.cpp.o: CMakeFiles/saturntools.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/txkj/cpp_test/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/saturntools.dir/src/deteRes.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/saturntools.dir/src/deteRes.cpp.o -MF CMakeFiles/saturntools.dir/src/deteRes.cpp.o.d -o CMakeFiles/saturntools.dir/src/deteRes.cpp.o -c /home/txkj/cpp_test/src/deteRes.cpp

CMakeFiles/saturntools.dir/src/deteRes.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/saturntools.dir/src/deteRes.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/txkj/cpp_test/src/deteRes.cpp > CMakeFiles/saturntools.dir/src/deteRes.cpp.i

CMakeFiles/saturntools.dir/src/deteRes.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/saturntools.dir/src/deteRes.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/txkj/cpp_test/src/deteRes.cpp -o CMakeFiles/saturntools.dir/src/deteRes.cpp.s

CMakeFiles/saturntools.dir/src/fileOperateUtil.cpp.o: CMakeFiles/saturntools.dir/flags.make
CMakeFiles/saturntools.dir/src/fileOperateUtil.cpp.o: /home/txkj/cpp_test/src/fileOperateUtil.cpp
CMakeFiles/saturntools.dir/src/fileOperateUtil.cpp.o: CMakeFiles/saturntools.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/txkj/cpp_test/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/saturntools.dir/src/fileOperateUtil.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/saturntools.dir/src/fileOperateUtil.cpp.o -MF CMakeFiles/saturntools.dir/src/fileOperateUtil.cpp.o.d -o CMakeFiles/saturntools.dir/src/fileOperateUtil.cpp.o -c /home/txkj/cpp_test/src/fileOperateUtil.cpp

CMakeFiles/saturntools.dir/src/fileOperateUtil.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/saturntools.dir/src/fileOperateUtil.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/txkj/cpp_test/src/fileOperateUtil.cpp > CMakeFiles/saturntools.dir/src/fileOperateUtil.cpp.i

CMakeFiles/saturntools.dir/src/fileOperateUtil.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/saturntools.dir/src/fileOperateUtil.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/txkj/cpp_test/src/fileOperateUtil.cpp -o CMakeFiles/saturntools.dir/src/fileOperateUtil.cpp.s

CMakeFiles/saturntools.dir/src/operateDeteRes.cpp.o: CMakeFiles/saturntools.dir/flags.make
CMakeFiles/saturntools.dir/src/operateDeteRes.cpp.o: /home/txkj/cpp_test/src/operateDeteRes.cpp
CMakeFiles/saturntools.dir/src/operateDeteRes.cpp.o: CMakeFiles/saturntools.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/txkj/cpp_test/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/saturntools.dir/src/operateDeteRes.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/saturntools.dir/src/operateDeteRes.cpp.o -MF CMakeFiles/saturntools.dir/src/operateDeteRes.cpp.o.d -o CMakeFiles/saturntools.dir/src/operateDeteRes.cpp.o -c /home/txkj/cpp_test/src/operateDeteRes.cpp

CMakeFiles/saturntools.dir/src/operateDeteRes.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/saturntools.dir/src/operateDeteRes.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/txkj/cpp_test/src/operateDeteRes.cpp > CMakeFiles/saturntools.dir/src/operateDeteRes.cpp.i

CMakeFiles/saturntools.dir/src/operateDeteRes.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/saturntools.dir/src/operateDeteRes.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/txkj/cpp_test/src/operateDeteRes.cpp -o CMakeFiles/saturntools.dir/src/operateDeteRes.cpp.s

CMakeFiles/saturntools.dir/src/tools/pystring.cpp.o: CMakeFiles/saturntools.dir/flags.make
CMakeFiles/saturntools.dir/src/tools/pystring.cpp.o: /home/txkj/cpp_test/src/tools/pystring.cpp
CMakeFiles/saturntools.dir/src/tools/pystring.cpp.o: CMakeFiles/saturntools.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/txkj/cpp_test/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/saturntools.dir/src/tools/pystring.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/saturntools.dir/src/tools/pystring.cpp.o -MF CMakeFiles/saturntools.dir/src/tools/pystring.cpp.o.d -o CMakeFiles/saturntools.dir/src/tools/pystring.cpp.o -c /home/txkj/cpp_test/src/tools/pystring.cpp

CMakeFiles/saturntools.dir/src/tools/pystring.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/saturntools.dir/src/tools/pystring.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/txkj/cpp_test/src/tools/pystring.cpp > CMakeFiles/saturntools.dir/src/tools/pystring.cpp.i

CMakeFiles/saturntools.dir/src/tools/pystring.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/saturntools.dir/src/tools/pystring.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/txkj/cpp_test/src/tools/pystring.cpp -o CMakeFiles/saturntools.dir/src/tools/pystring.cpp.s

CMakeFiles/saturntools.dir/src/tools/tinyxml2.cpp.o: CMakeFiles/saturntools.dir/flags.make
CMakeFiles/saturntools.dir/src/tools/tinyxml2.cpp.o: /home/txkj/cpp_test/src/tools/tinyxml2.cpp
CMakeFiles/saturntools.dir/src/tools/tinyxml2.cpp.o: CMakeFiles/saturntools.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/txkj/cpp_test/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/saturntools.dir/src/tools/tinyxml2.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/saturntools.dir/src/tools/tinyxml2.cpp.o -MF CMakeFiles/saturntools.dir/src/tools/tinyxml2.cpp.o.d -o CMakeFiles/saturntools.dir/src/tools/tinyxml2.cpp.o -c /home/txkj/cpp_test/src/tools/tinyxml2.cpp

CMakeFiles/saturntools.dir/src/tools/tinyxml2.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/saturntools.dir/src/tools/tinyxml2.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/txkj/cpp_test/src/tools/tinyxml2.cpp > CMakeFiles/saturntools.dir/src/tools/tinyxml2.cpp.i

CMakeFiles/saturntools.dir/src/tools/tinyxml2.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/saturntools.dir/src/tools/tinyxml2.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/txkj/cpp_test/src/tools/tinyxml2.cpp -o CMakeFiles/saturntools.dir/src/tools/tinyxml2.cpp.s

# Object files for target saturntools
saturntools_OBJECTS = \
"CMakeFiles/saturntools.dir/src/deteObj.cpp.o" \
"CMakeFiles/saturntools.dir/src/deteRes.cpp.o" \
"CMakeFiles/saturntools.dir/src/fileOperateUtil.cpp.o" \
"CMakeFiles/saturntools.dir/src/operateDeteRes.cpp.o" \
"CMakeFiles/saturntools.dir/src/tools/pystring.cpp.o" \
"CMakeFiles/saturntools.dir/src/tools/tinyxml2.cpp.o"

# External object files for target saturntools
saturntools_EXTERNAL_OBJECTS =

/home/txkj/cpp_test/lib/libsaturntools.so: CMakeFiles/saturntools.dir/src/deteObj.cpp.o
/home/txkj/cpp_test/lib/libsaturntools.so: CMakeFiles/saturntools.dir/src/deteRes.cpp.o
/home/txkj/cpp_test/lib/libsaturntools.so: CMakeFiles/saturntools.dir/src/fileOperateUtil.cpp.o
/home/txkj/cpp_test/lib/libsaturntools.so: CMakeFiles/saturntools.dir/src/operateDeteRes.cpp.o
/home/txkj/cpp_test/lib/libsaturntools.so: CMakeFiles/saturntools.dir/src/tools/pystring.cpp.o
/home/txkj/cpp_test/lib/libsaturntools.so: CMakeFiles/saturntools.dir/src/tools/tinyxml2.cpp.o
/home/txkj/cpp_test/lib/libsaturntools.so: CMakeFiles/saturntools.dir/build.make
/home/txkj/cpp_test/lib/libsaturntools.so: CMakeFiles/saturntools.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/txkj/cpp_test/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Linking CXX shared library /home/txkj/cpp_test/lib/libsaturntools.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/saturntools.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/saturntools.dir/build: /home/txkj/cpp_test/lib/libsaturntools.so
.PHONY : CMakeFiles/saturntools.dir/build

CMakeFiles/saturntools.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/saturntools.dir/cmake_clean.cmake
.PHONY : CMakeFiles/saturntools.dir/clean

CMakeFiles/saturntools.dir/depend:
	cd /home/txkj/cpp_test/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/txkj/cpp_test /home/txkj/cpp_test /home/txkj/cpp_test/build /home/txkj/cpp_test/build /home/txkj/cpp_test/build/CMakeFiles/saturntools.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/saturntools.dir/depend

