cmake_minimum_required (VERSION 2.6)
project (Sudoku)
INCLUDE(FindPackageHandleStandardArgs)

#Find Doxygen
Find_Package(Doxygen)
if (DOXYGEN_FOUND)
	configure_file(doxyconf.in doxyconf @ONLY)
	add_custom_target(doc ${DOXYGEN_EXECUTABLE} doxyconf comment "Generating API documentation with Doxygen" verbatim)
endif (DOXYGEN_FOUND)

#Find NCurses
set (CURSES_NEED_WIDE TRUE)
FIND_PACKAGE(Curses)
if (NOT USE_CURSES)
	message("Curses support deactivated. Text interface will not be available.")
elseif (NOT CURSES_FOUND)
	message("Curses library unavailable. Text interface will be deactivated.")
else (NOT USE_CURSES)
	set (HAVE_CURSES 1)
endif (NOT USE_CURSES)

#Options
set(DEBUG_LEVEL 0)
OPTION(USE_CURSES "Compile with Ncurses support (if available) and enables GUI" ON)

#General configuration
SET(CMAKE_CXX_FLAGS "-Wall -Werror -std=c++11")
SET(CMAKE_CXX_FLAGS_DEBUG
		"${GCC_DEBUG_FLAGS} -O0 -g"
		CACHE STRING "Flags used by the C++ compiler during debug builds."
		FORCE )
SET(CMAKE_C_FLAGS_DEBUG
		"${GCC_DEBUG_FLAGS} -O0 -g"
		CACHE STRING "Flags used by the C compiler during debug builds."
		FORCE )

#Profiling build type
SET(CMAKE_CXX_FLAGS_PROFILE
    "${GCC_DEBUG_FLAGS} -O0 -g -pg -fprofile-arcs -ftest-coverage"
    CACHE STRING "Flags used by the C++ compiler during coverage builds."
    FORCE )
SET(CMAKE_C_FLAGS_PROFILE
    "${GCC_DEBUG_FLAGS} -O0 -g -pg -fprofile-arcs -ftest-coverage"
    CACHE STRING "Flags used by the C compiler during coverage builds."
    FORCE )
SET(CMAKE_EXE_LINKER_FLAGS_PROFILE
    ""
    CACHE STRING "Flags used for linking binaries during coverage builds."
    FORCE )
SET(CMAKE_SHARED_LINKER_FLAGS_PROFILE
    ""
    CACHE STRING "Flags used by the shared libraries linker during coverage builds."
    FORCE )
MARK_AS_ADVANCED(
    CMAKE_CXX_FLAGS_PROFILE
    CMAKE_C_FLAGS_PROFILE
    CMAKE_EXE_LINKER_FLAGS_PROFILE
    CMAKE_SHARED_LINKER_FLAGS_PROFILE )
	
configure_file(config.h.in config.h)

include_directories()
link_directories()

file(GLOB source_files *.cpp *.h)

add_executable (sudoku ${source_files})

target_link_libraries(sudoku)
if (CURSES_FOUND)
	target_link_libraries(sudoku ${CURSES_LIBRARY})
endif (CURSES_FOUND)
