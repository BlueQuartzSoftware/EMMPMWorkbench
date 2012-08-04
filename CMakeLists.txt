#///////////////////////////////////////////////////////////////////////////////
#
#  Copyright (c) 2010, Michael A. Jackson. BlueQuartz Software
#  All rights reserved.
#  BSD License: http://www.opensource.org/licenses/bsd-license.html
#
#///////////////////////////////////////////////////////////////////////////////

project (EMMPMGuiProj)
cmake_minimum_required(VERSION 2.8.6)
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


SET (PROJECT_CODE_DIR ${EMMPMGuiProj_SOURCE_DIR}/Code)
SET (PROJECT_RESOURCES_DIR ${EMMPMGuiProj_SOURCE_DIR}/Resources)
SET (EXE_DEBUG_EXTENSION "_debug")
SET (PROJECT_INSTALL_HEADERS "0")
SET (PROJECT_INSTALL_EXECUTABLES "1")

if (NOT DEFINED CMP_SOURCE_DIR)
    set(CMP_SOURCE_DIR ${EMMPMGuiProj_SOURCE_DIR}/../CMP)
endif()

INCLUDE (${CMP_SOURCE_DIR}/cmpCMakeMacros.cmake )

set (CMP_ENABLE_PLUGINS "1")
set (CMP_PLUGIN_LIST_FILE ${PROJECT_BINARY_DIR}/EMMPMGuiProj_PluginList.txt)
file(WRITE ${CMP_PLUGIN_LIST_FILE} "")
set (CMP_PLUGIN_SEARCHDIR_FILE ${PROJECT_BINARY_DIR}/libsearchdirs.txt)
file(WRITE ${CMP_PLUGIN_SEARCHDIR_FILE} "${PROJECT_BINARY_DIR}/Bin/plugins;")
file(APPEND ${CMP_PLUGIN_SEARCHDIR_FILE} "${PROJECT_BINARY_DIR}/Bin;")
set (CMP_LIB_SEARCH_DIRS "")
set (CMP_MODULES_SOURCE_DIR ${CMP_SOURCE_DIR}/Modules CACHE INTERNAL "")
set (CMP_CONFIGURED_FILES_SOURCE_DIR ${CMP_SOURCE_DIR}/ConfiguredFiles CACHE INTERRNAL "")


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

# Should we build the HTML documentation using DOxygen
option(EMMPMGuiProj_BUILD_API_DOCS "Use Doxygen to create the HTML based API documentation" OFF)
if(EMMPMGuiProj_BUILD_API_DOCS)
  FIND_PACKAGE(Doxygen)
  if (NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Doxygen is needed to build the documentation. Please install it correctly")
  endif()
  configure_file(${PROJECT_RESOURCES_DIR}/EMMPMGuiProj.doxyfile.in 
                 ${EMMPMGuiProj_BINARY_DIR}/Doxyfile  @ONLY IMMEDIATE)
  add_custom_target(docs COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile)
  
endif()


option(EMMPMGuiProj_BUILD_TESTING "Build All unit tests" OFF)
if (EMMPMGuiProj_BUILD_TESTING)
    ENABLE_TESTING()
endif()
# --------------------------------------------------------------------
# Bring in the MXADataModel Library
if (NOT TARGET MXADataModel)
    set(MXA_BUILD_TESTING ${EMMPMGuiProj_BUILD_TESTING})
    set(MXA_BUILD_EXAMPLES OFF CACHE BOOL "")
    set(MXA_BUILD_UTILITIES OFF CACHE BOOL "")
    set(MXA_BUILD_API_DOCS ${EMMPMGuiProj_BUILD_API_DOCS})
    set(MXA_USE_XML OFF CACHE BOOL "")
    set(MXA_USE_HDF5 OFF CACHE BOOL "")
    set(MXA_USE_TIFF OFF CACHE BOOL "")
    add_subdirectory(${EMMPMGuiProj_SOURCE_DIR}/../MXADataModel ${EMMPMGuiProj_BINARY_DIR}/MXADataModel)
endif()
include_directories(${MXADataModel_SOURCE_DIR}/Code)
include_directories(${MXADataModel_BINARY_DIR})

# --------------------------------------------------------------------
# Bring in the EM/MPM Library Next
if (NOT TARGET EMMPMLib)
    set (EMMPMLib_BUILD_API_DOCS ${EMMPMGuiProj_BUILD_API_DOCS})
    add_subdirectory(${EMMPMGuiProj_SOURCE_DIR}/../emmpm ${EMMPMGuiProj_BINARY_DIR}/emmpm)
endif()
include_directories(${EMMPMProj_SOURCE_DIR}/Code)
include_directories(${EMMPMProj_BINARY_DIR})


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
SET (EMMPMGuiProj_INSTALL_FILES "${PROJECT_INSTALL_HEADERS}")

# --------------------------------------------------------------------
# Generate our ReadMe and License Files
configure_file(${PROJECT_SOURCE_DIR}/License.txt.in
                ${PROJECT_BINARY_DIR}/License.txt )
configure_file(${PROJECT_SOURCE_DIR}/ReadMe.txt.in
                ${PROJECT_BINARY_DIR}/ReadMe.txt )
                
# Include some directories for the compiler
include_directories(${PROJECT_BINARY_DIR})

# --------------------------------------------------------------------
# Compile the support libraries
add_subdirectory(${PROJECT_CODE_DIR}/QtSupport ${PROJECT_BINARY_DIR}/QtSupport)
include_directories(${PROJECT_BINARY_DIR}/QtSupport)

# --------------------------------------------------------------------
# Compile the Enhanced EM/MPM Qt Base GUI application
add_subdirectory(${PROJECT_CODE_DIR}/EmMpmGui ${PROJECT_BINARY_DIR}/Applications/EmMpmGui)


# Include code to Package our Executables into a Zip or NSIS Installer
include(${PROJECT_RESOURCES_DIR}/CPack/PackageProject.cmake)
