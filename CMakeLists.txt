#///////////////////////////////////////////////////////////////////////////////
#
#  Copyright (c) 2010, Michael A. Jackson. BlueQuartz Software
#  All rights reserved.
#  BSD License: http://www.opensource.org/licenses/bsd-license.html
#
#///////////////////////////////////////////////////////////////////////////////

# ------------------------------------------------------------------------------
# This project has dependencies on ITK. If you build ITK as Dynamic Libraries then
# the app bundling cmake code (at least for OS X) will most likely NOT be able to
# find those libraries. Building ITK as STATIC libraries will alleviate this 
# problem and allow you to create a self contained OS X app bundle with out any
# additional issues.
# ------------------------------------------------------------------------------

project (EIMApps)
cmake_minimum_required(VERSION 2.8)

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
SET (PROJECT_RESOURCES_DIR ${EIMApps_SOURCE_DIR}/Resources)
SET (PROJECT_PREFIX "MXA" CACHE STRING "The Prefix to be used for Preprocessor definitions")
SET (EXE_DEBUG_EXTENSION "_debug")
SET (PROJECT_INSTALL_HEADERS "0")
SET (PROJECT_INSTALL_EXECUTABLES "1")

#-------------------------------------------------------------------------------
# Include the cmp project to perform all the basic configuration tests for the 
# project.
#------------------------------------------------------------------------------- 
set (CMP_HEADER_DIR ${EIMApps_BINARY_DIR}/MXA)
set (CMP_CONFIGURATION_FILE_NAME "CMPConfiguration.h")
set (CMP_TYPES_FILE_NAME "MXATypes.h")
set (CMP_VERSION_HEADER_FILE_NAME "MXAVersion.h")
set (CMP_EXTRA_CONFIGURATION_FILE "MXAConfiguration.h")
set (CMP_PROJECT_NAMESPACE "MXA")
set (CMP_PROJECT_NAME "MXA")
SET (CMP_INSTALL_FILES "1")
set (CMP_ENABLE_PLUGINS "0")
#set (CMP_PLUGIN_LIST_FILE ${PROJECT_BINARY_DIR}/EIMAppsPluginList.txt)
INCLUDE (${PROJECT_RESOURCES_DIR}/cmp/cmpProject.cmake)

# --------------------------------------------------------------------
# Generate a Header file with Compile Version variables
# --------------------------------------------------------------------
cmpGenerateVersionString( "${EIMApps_BINARY_DIR}/${PROJECT_NAME}/Common/${PROJECT_NAME}Version.h" 
                          "EIMApps" 
                          "EIMApps")
                          
# --------------------------------------------------------------------
# Generate our ReadMe and License Files
configure_file(${PROJECT_SOURCE_DIR}/License.txt.in
                ${PROJECT_BINARY_DIR}/License.txt )
configure_file(${PROJECT_SOURCE_DIR}/ReadMe.txt.in
                ${PROJECT_BINARY_DIR}/ReadMe.txt )

# --------------------------------------------------------------------
# Generate the License File Headers
include(${PROJECT_SOURCE_DIR}/License/License.cmake)

SET (MXA_INSTALL_FILES "0")
SET (MXA_SOURCE_DIR ${PROJECT_CODE_DIR})

# Include some directories for the compiler
include_directories(${PROJECT_BINARY_DIR})


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
SET (MXA_BOOST_COMPONENTS "")
FIND_PACKAGE(Boost COMPONENTS  ${MXA_BOOST_COMPONENTS} )
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

# --------------------------------------------------------------------
# Find the ITK Package
# --------------------------------------------------------------------
FIND_PACKAGE(ITK)
# --------------------------------------------------------------------
# Find the ITK Package
# --------------------------------------------------------------------
#IF(ITK_FOUND)
#  file(APPEND ${CMP_PLUGIN_SEARCHDIR_FILE} "${ITK_LIBRARY_DIRS};")
#ELSE(ITK_FOUND)
#  MESSAGE(FATAL_ERROR "Cannot build without ITK.  Please set ITK_DIR.")
#ENDIF(ITK_FOUND)
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


# --------------------------------------------------------------------
# Compile the support libraries
add_subdirectory(${PROJECT_CODE_DIR}/AIM ${PROJECT_BINARY_DIR}/AIMLib)
add_subdirectory(${PROJECT_CODE_DIR}/QtSupport ${PROJECT_BINARY_DIR}/QtSupport)
add_subdirectory(${PROJECT_CODE_DIR}/MXA ${PROJECT_BINARY_DIR}/MXA)

set(IPHelper_BINARY_DIR ${PROJECT_BINARY_DIR}/Applications/IPHelper)
# --------------------------------------------------------------------
# Option to compile the Qt Based EIMApps Application
option(EIMApps_BUILD_IPHELPER_GUI "Build the EIMApps GUI Application" ON)
if (EIMApps_BUILD_IPHELPER_GUI)
    # Clear these files each time otherwise we get duplicates
    file(WRITE "${IPHelper_BINARY_DIR}/IPHelper_Plugins.txt" "")
    file(WRITE "${IPHelper_BINARY_DIR}/LibSearchDirs.txt" "")
    
    # Compile the image processing plugins
    add_subdirectory(${PROJECT_CODE_DIR}/IPHelper/plugins/EMMPM ${PROJECT_BINARY_DIR}/plugins/EMMPM)
    add_subdirectory(${PROJECT_CODE_DIR}/IPHelper/plugins/CrossCorrelation ${PROJECT_BINARY_DIR}/plugins/CrossCorrelation)
    #add_subdirectory(${PROJECT_CODE_DIR}/IPHelper/plugins/MutualInformation ${PROJECT_BINARY_DIR}/plugins/MutualInformation)

    # Compile the main Application - this MUST be last due to how some variables are built up.
    add_subdirectory(${PROJECT_CODE_DIR}/IPHelper/Application ${IPHelper_BINARY_DIR})
endif()


# --------------------------------------------------------------------
# Option to Compile the Enhanced EM/MPM Qt Base GUI application
option(EIMApps_BUILD_EMMPM_GUI "Build the EM/MPM GUI Application" ON)

if (EIMApps_BUILD_EMMPM_GUI)
    add_subdirectory(${PROJECT_CODE_DIR}/EmMpmGui ${PROJECT_BINARY_DIR}/Applications/EmMpmGui)
endif()

# --------------------------------------------------------------------
# This was left over from the Mutual Information code that never matialized
# add_subdirectory(${PROJECT_CODE_DIR}/AngReader ${PROJECT_BINARY_DIR}/AngReader)

#add_subdirectory(${PROJECT_SOURCE_DIR}/Test ${PROJECT_BINARY_DIR}/Applications/ProgressLabel)


# Should we build the HTML documentation using DOxygen
option(EIMApps_BUILD_API_DOCS "Use Doxygen to create the HTML based API documentation" OFF)
if(EIMApps_BUILD_API_DOCS)
  FIND_PACKAGE(Doxygen)
  if (NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Doxygen is needed to build the documentation. Please install it correctly")
  endif()
  configure_file(${PROJECT_RESOURCES_DIR}/EIMApps.doxyfile.in 
                 ${PROJECT_BINARY_DIR}/Doxyfile  @ONLY IMMEDIATE)
  add_custom_target(docs COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile)
endif()


# Include code to Package our Executables into a Zip or NSIS Installer
include(${PROJECT_RESOURCES_DIR}/CPack/PackageProject.cmake)
