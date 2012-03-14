#///////////////////////////////////////////////////////////////////////////////
#
#  Copyright (c) 2010, Michael A. Jackson. BlueQuartz Software
#  All rights reserved.
#  BSD License: http://www.opensource.org/licenses/bsd-license.html
#
#///////////////////////////////////////////////////////////////////////////////

project (QEMMPM)
cmake_minimum_required(VERSION 2.8)
include (CMakeParseArguments)

# ---------- Setup output Directories -------------------------
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY
  ${PROJECT_BINARY_DIR}/Bin
  CACHE PATH
  "Single Directory for all Libraries"
  )

# --------- Setup the Executable output Directory -------------
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
  ${PROJECT_BINARY_DIR}/Bin
  CACHE PATH
  "Single Directory for all Executables."
  )

# --------- Setup the Executable output Directory -------------
SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY
  ${PROJECT_BINARY_DIR}/Bin
  CACHE PATH
  "Single Directory for all static libraries."
  )


SET (PROJECT_CODE_DIR ${PROJECT_SOURCE_DIR}/Code)
SET (PROJECT_RESOURCES_DIR ${QEMMPM_SOURCE_DIR}/Resources)
SET (EXE_DEBUG_EXTENSION "_debug")
SET (PROJECT_INSTALL_HEADERS "0")
SET (PROJECT_INSTALL_EXECUTABLES "1")

set (CMP_SOURCE_DIR ${QEMMPM_SOURCE_DIR}/Support/cmp)
INCLUDE (${CMP_SOURCE_DIR}/cmpCMakeMacros.cmake )
set (CMP_ENABLE_PLUGINS "1")
set (CMP_PLUGIN_LIST_FILE ${PROJECT_BINARY_DIR}/QEMMPM_PluginList.txt)
file(WRITE ${CMP_PLUGIN_LIST_FILE} "")
set (CMP_PLUGIN_SEARCHDIR_FILE ${PROJECT_BINARY_DIR}/libsearchdirs.txt)
file(WRITE ${CMP_PLUGIN_SEARCHDIR_FILE} "${PROJECT_BINARY_DIR}/Bin/plugins;")
file(APPEND ${CMP_PLUGIN_SEARCHDIR_FILE} "${PROJECT_BINARY_DIR}/Bin;")
set (CMP_LIB_SEARCH_DIRS "")
set(CMP_MODULES_SOURCE_DIR ${CMP_SOURCE_DIR}/Modules CACHE INTERNAL "")

# --------------------------------------------------------------------
# In order to increment the version of QEMMPM simply adjust these variables.
# we are using a 3 part Version scheme commonly found in software development. The
# example would be Version 2.1.6543 which translates to Version [major].[minor].[patch].
# The Major version is set here in the CMake file. We will use Git 
# to figure out the "patch" leve of the version by using "git describe" which will
# calculate how many commits since the last "tag" was applied to the repository.
# this basically gives us an always incrementing number for the patch level
# and we can set the Major and Minor versions as we see fit. 
set(CMP_VERSION_MAJOR "v2")


# --------------------------------------------------------------------
# Over ride CMake's built in module directory by prepending cmp's module
# directory first
SET (CMAKE_MODULE_PATH ${CMP_MODULES_SOURCE_DIR} ${CMAKE_MODULE_PATH})

# Build shared libraries
OPTION (BUILD_SHARED_LIBS "Build Shared Libraries" OFF)
SET (LIB_TYPE STATIC)
IF (BUILD_SHARED_LIBS)
    SET (LIB_TYPE SHARED)
    list(APPEND CMP_LIB_SEARCH_DIRS  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )
    if (MSVC)
        list(APPEND CMP_LIB_SEARCH_DIRS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug
                                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release )
    endif()
ENDIF (BUILD_SHARED_LIBS)

# ---------- Find Boost Headers/Libraries -----------------------
SET (Boost_FIND_REQUIRED TRUE)
SET (Boost_DEBUG FALSE)
set (Boost_USE_MULTITHREADED TRUE)
set (Boost_USE_STATIC_LIBS TRUE)
SET (Boost_ADDITIONAL_VERSIONS "1.44.0" "1.44" "1.36" "1.36.0" "1.41" "1.41.0" "1.39" "1.39.0")
set (Boost_FIND_COMPONENTS "")
set (Boost_NO_BOOST_CMAKE 1)
FIND_PACKAGE(Boost)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

# ------------------------------------------------------------------------------
#  Qt 4 Section
# ------------------------------------------------------------------------------
# by default only QtCore and QtGui modules are enabled
# other modules must be enabled like this:  
IF (WIN32)
    SET (QT_USE_QTMAIN TRUE)
ENDIF (WIN32)
# this command finds Qt4 libraries and sets all required variables
# note that it's Qt4, not QT4 or qt4. add some useful macros and variables
# (QT_USE_FILE is a variable defined by FIND_PACKAGE( Qt4 ) that 
# contains a path to CMake script)
FIND_PACKAGE( Qt4 REQUIRED )
IF (QT4_FOUND)
    CMP_COPY_QT4_RUNTIME_LIBRARIES( "QtCore;QtGui")
    CMP_QT_LIBRARIES_INSTALL_RULES("QtCore;QtGui" ".")
    INCLUDE( ${QT_USE_FILE} )
   
    # Append the locations of the Qt libraries to our Library Search Paths
    list(APPEND CMP_LIB_SEARCH_DIRS ${QT_BINARY_DIR} ${QT_LIBRARY_DIR} ${QT_PLUGINS_DIR}/imageformats)
    
    # Get the list of Qt4 Image Format Plugins that we need
    # Find all the Qt Plugins after which we should have some more variables to our disposal
    # FindQt4Plugins("qgif;qjpeg;qtiff" "${PROJECT_BINARY_DIR}/QTPLUGINS.txt" "${CMP_PLUGIN_SEARCHDIR_FILE}" "imageformats" )
endif()

# We do NOT want to install any of the headers
SET (QEMMPM_INSTALL_FILES "${PROJECT_INSTALL_HEADERS}")

# --------------------------------------------------------------------
# Generate our ReadMe and License Files
configure_file(${PROJECT_SOURCE_DIR}/License.txt.in
                ${PROJECT_BINARY_DIR}/License.txt )
configure_file(${PROJECT_SOURCE_DIR}/ReadMe.txt.in
                ${PROJECT_BINARY_DIR}/ReadMe.txt )
                
# Include some directories for the compiler
include_directories(${PROJECT_BINARY_DIR})

# Bring in the EM/MPM Library first
add_subdirectory(${QEMMPM_SOURCE_DIR}/Support/emmpm ${QEMMPM_BINARY_DIR}/emmpm)
include_directories(${QEMMPM_SOURCE_DIR}/Support/emmpm/Code)
include_directories(${QEMMPM_BINARY_DIR}/emmpm)

# --------------------------------------------------------------------
# Compile the support libraries
add_subdirectory(${PROJECT_CODE_DIR}/QtSupport ${PROJECT_BINARY_DIR}/QtSupport)
add_subdirectory(${PROJECT_CODE_DIR}/MXA ${PROJECT_BINARY_DIR}/MXA)


# --------------------------------------------------------------------
# Compile the Enhanced EM/MPM Qt Base GUI application
add_subdirectory(${PROJECT_CODE_DIR}/EmMpmGui ${PROJECT_BINARY_DIR}/Applications/EmMpmGui)


# Should we build the HTML documentation using DOxygen
option(QEMMPM_BUILD_API_DOCS "Use Doxygen to create the HTML based API documentation" OFF)
if(QEMMPM_BUILD_API_DOCS)
  FIND_PACKAGE(Doxygen)
  if (NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Doxygen is needed to build the documentation. Please install it correctly")
  endif()
  configure_file(${PROJECT_RESOURCES_DIR}/QEMMPM.doxyfile.in 
                 ${PROJECT_BINARY_DIR}/Doxyfile  @ONLY IMMEDIATE)
  add_custom_target(docs COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile)
endif()


# Include code to Package our Executables into a Zip or NSIS Installer
include(${PROJECT_RESOURCES_DIR}/CPack/PackageProject.cmake)
