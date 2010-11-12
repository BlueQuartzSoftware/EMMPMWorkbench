#/* --------------------------------------------------------------------------*
#* This source code has been cleared for public release by the                *
#* US Air Force 88th Air Base Wing Public Affairs Office under                *
#* case number 88ABW-2010-4857 on Sept. 7, 2010.                              *
#* -------------------------------------------------------------------------- */
#--////////////////////////////////////////////////////////////////////////////
#--  Copyright (c) 2009, Michael A. Jackson. BlueQuartz Software
#--  All rights reserved.
#--  BSD License: http://www.opensource.org/licenses/bsd-license.html
#--////////////////////////////////////////////////////////////////////////////

# - Find EM/MPM
# Find the native EMMPM headers and libraries.
#
#  EMMPM_INCLUDE_DIRS - where to find EM/MPM.h, etc.
#  EMMPM_LIBRARIES    - List of libraries when using EM/MPM.
#  EMMPM_LIBRARY_DEBUG - Debug version of Library
#  EMMPM_LIBRARY_RELEASE - Release Version of Library
#  EMMPM_FOUND        - True if EM/MPM found.
#  EMMPM_RESOURCES_DIR - The directory where supporting cmake files and other files can be found

# Look for the header file.

############################################
#
# Check the existence of the libraries.
#
############################################
# This macro was taken directly from the FindQt4.cmake file that is included
# with the CMake distribution. This is NOT my work. All work was done by the
# original authors of the FindQt4.cmake file. Only minor modifications were
# made to remove references to Qt and make this file more generally applicable
#########################################################################

MACRO (_adjust_lib_var_names basename)
  IF (${basename}_INCLUDE_DIR)

  # if only the release version was found, set the debug variable also to the release version
  IF (${basename}_LIBRARY_RELEASE AND NOT ${basename}_LIBRARY_DEBUG)
    SET(${basename}_LIBRARY_DEBUG ${${basename}_LIBRARY_RELEASE})
    SET(${basename}_LIBRARY       ${${basename}_LIBRARY_RELEASE})
    SET(${basename}_LIBRARIES     ${${basename}_LIBRARY_RELEASE})
  ENDIF (${basename}_LIBRARY_RELEASE AND NOT ${basename}_LIBRARY_DEBUG)

  # if only the debug version was found, set the release variable also to the debug version
  IF (${basename}_LIBRARY_DEBUG AND NOT ${basename}_LIBRARY_RELEASE)
    SET(${basename}_LIBRARY_RELEASE ${${basename}_LIBRARY_DEBUG})
    SET(${basename}_LIBRARY         ${${basename}_LIBRARY_DEBUG})
    SET(${basename}_LIBRARIES       ${${basename}_LIBRARY_DEBUG})
  ENDIF (${basename}_LIBRARY_DEBUG AND NOT ${basename}_LIBRARY_RELEASE)
  IF (${basename}_LIBRARY_DEBUG AND ${basename}_LIBRARY_RELEASE)
    # if the generator supports configuration types then set
    # optimized and debug libraries, or if the CMAKE_BUILD_TYPE has a value
    IF (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
      SET(${basename}_LIBRARY       optimized ${${basename}_LIBRARY_RELEASE} debug ${${basename}_LIBRARY_DEBUG})
    ELSE(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
      # if there are no configuration types and CMAKE_BUILD_TYPE has no value
      # then just use the release libraries
      SET(${basename}_LIBRARY       ${${basename}_LIBRARY_RELEASE} )
    ENDIF(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
    SET(${basename}_LIBRARIES       optimized ${${basename}_LIBRARY_RELEASE} debug ${${basename}_LIBRARY_DEBUG})
  ENDIF (${basename}_LIBRARY_DEBUG AND ${basename}_LIBRARY_RELEASE)

  SET(${basename}_LIBRARY ${${basename}_LIBRARY} CACHE FILEPATH "The ${basename} library")

  IF (${basename}_LIBRARY)
    SET(${basename}_FOUND 1)
  ENDIF (${basename}_LIBRARY)

ENDIF (${basename}_INCLUDE_DIR )

  # Make variables changeble to the advanced user
  MARK_AS_ADVANCED(${basename}_LIBRARY ${basename}_LIBRARY_RELEASE ${basename}_LIBRARY_DEBUG ${basename}_INCLUDE_DIR )
ENDMACRO (_adjust_lib_var_names)
  
  
#MESSAGE (STATUS "Finding EM/MPM library and headers..." )
SET (EMMPM_DEBUG 1)

SET(EMMPM_INCLUDE_SEARCH_DIRS
  $ENV{EMMPM_INSTALL}/include
)

SET (EMMPM_LIB_SEARCH_DIRS
  $ENV{EMMPM_INSTALL}/lib
  )

SET (EMMPM_BIN_SEARCH_DIRS
  $ENV{EMMPM_INSTALL}/bin
)

FIND_PATH(EMMPM_INCLUDE_DIR 
  NAMES emmpm/public/EMMPM.h
  PATHS ${EMMPM_INCLUDE_SEARCH_DIRS}
  NO_DEFAULT_PATH
)

SET (EMMPM_SEARCH_DEBUG_NAMES "EMMPMLib_debug;EMMPMLib_D;libEMMPMLib_D;libEMMPMLib_debug")
SET (EMMPM_SEARCH_RELEASE_NAMES "EMMPMLib;libEMMPMLib")



IF (EMMPM_DEBUG)
    message (STATUS "EMMPM_INCLUDE_SEARCH_DIRS: ${EMMPM_INCLUDE_SEARCH_DIRS}")
    message (STATUS "EMMPM_LIB_SEARCH_DIRS: ${EMMPM_LIB_SEARCH_DIRS}")
    message (STATUS "EMMPM_BIN_SEARCH_DIRS: ${EMMPM_BIN_SEARCH_DIRS}")
    message (STATUS "EMMPM_SEARCH_RELEASE_NAMES: ${EMMPM_SEARCH_RELEASE_NAMES}")
    message (STATUS "EMMPM_SEARCH_DEBUG_NAMES: ${EMMPM_SEARCH_DEBUG_NAMES}")
    message (STATUS "EMMPM_INCLUDE_DIR: ${EMMPM_INCLUDE_DIR}")
ENDIF (EMMPM_DEBUG)

# Look for the library.
FIND_LIBRARY(EMMPM_LIBRARY_DEBUG 
  NAMES ${EMMPM_SEARCH_DEBUG_NAMES}
  PATHS ${EMMPM_LIB_SEARCH_DIRS} 
  NO_DEFAULT_PATH
  )
  
FIND_LIBRARY(EMMPM_LIBRARY_RELEASE 
  NAMES ${EMMPM_SEARCH_RELEASE_NAMES}
  PATHS ${EMMPM_LIB_SEARCH_DIRS} 
  NO_DEFAULT_PATH
  )
 
_adjust_lib_var_names(EMMPM)

# MESSAGE( STATUS "EMMPM_LIBRARY: ${EMMPM_LIBRARY}")

# Copy the results to the output variables.
IF(EMMPM_INCLUDE_DIR AND EMMPM_LIBRARY)
  SET(EMMPM_FOUND 1)
  SET(EMMPM_LIBRARIES ${EMMPM_LIBRARY})
  SET(EMMPM_INCLUDE_DIRS ${EMMPM_INCLUDE_DIR})
  IF (EMMPM_LIBRARY_DEBUG)
    GET_FILENAME_COMPONENT(EMMPM_LIBRARY_PATH ${EMMPM_LIBRARY_DEBUG} PATH)
    SET(EMMPM_LIB_DIR ${EMMPM_LIBRARY_PATH})
    SET(EMMPM_BIN_DIR ${EMMPM_LIBRARY_PATH})
  ELSEIF(EMMPM_LIBRARY_RELEASE)
    GET_FILENAME_COMPONENT(EMMPM_LIBRARY_PATH ${EMMPM_LIBRARY_RELEASE} PATH)
    SET(EMMPM_LIB_DIR ${EMMPM_LIBRARY_PATH})
    SET(EMMPM_BIN_DIR ${EMMPM_LIBRARY_PATH})
  ENDIF(EMMPM_LIBRARY_DEBUG)
ELSE(EMMPM_INCLUDE_DIR AND EMMPM_LIBRARY)
  SET(EMMPM_FOUND 0)
  SET(EMMPM_LIBRARIES)
  SET(EMMPM_INCLUDE_DIRS)
ENDIF(EMMPM_INCLUDE_DIR AND EMMPM_LIBRARY)

# Report the results.
IF(NOT EMMPM_FOUND)
  SET(EMMPM_DIR_MESSAGE
    "EMMPM was not found. Make sure EMMPM_LIBRARY and EMMPM_INCLUDE_DIR are set or set the EMMPM_INSTALL environment variable.")
  IF(NOT EMMPM_FIND_QUIETLY)
    MESSAGE(STATUS "${EMMPM_DIR_MESSAGE}")
  ELSE(NOT EMMPM_FIND_QUIETLY)
    IF(EMMPM_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "EM/MPM was NOT found and is Required by this project")
    ENDIF(EMMPM_FIND_REQUIRED)
  ENDIF(NOT EMMPM_FIND_QUIETLY)
ENDIF(NOT EMMPM_FOUND)

IF(EMMPM_DEBUG)
 MESSAGE(STATUS "EMMPM_INCLUDE_DIRS: ${EMMPM_INCLUDE_DIRS}")
 MESSAGE(STATUS "EMMPM_LIBRARY_DEBUG: ${EMMPM_LIBRARY_DEBUG}")
 MESSAGE(STATUS "EMMPM_LIBRARY_RELEASE: ${EMMPM_LIBRARY_RELEASE}")
 MESSAGE(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
ENDIF(EMMPM_DEBUG)


IF (EMMPM_FOUND)
  INCLUDE(CheckSymbolExists)
  #############################################
  # Find out if EMMPM was built using dll's
  #############################################
  # Save required variable
  SET(CMAKE_REQUIRED_INCLUDES_SAVE ${CMAKE_REQUIRED_INCLUDES})
  SET(CMAKE_REQUIRED_FLAGS_SAVE    ${CMAKE_REQUIRED_FLAGS})
  # Add EMMPM_INCLUDE_DIR to CMAKE_REQUIRED_INCLUDES
  SET(CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES};${EMMPM_INCLUDE_DIRS}")

  CHECK_SYMBOL_EXISTS(EMMPM_BUILT_AS_DYNAMIC_LIB "emmpm/common/EMMPMConfiguration.h" HAVE_EMMPM_DLL)

    IF (HAVE_EMMPM_DLL STREQUAL "TRUE")
        SET (HAVE_EMMPM_DLL "1")
    ENDIF (HAVE_EMMPM_DLL STREQUAL "TRUE")

  # Restore CMAKE_REQUIRED_INCLUDES and CMAKE_REQUIRED_FLAGS variables
  SET(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES_SAVE})
  SET(CMAKE_REQUIRED_FLAGS    ${CMAKE_REQUIRED_FLAGS_SAVE})
  #
  #############################################

ENDIF (EMMPM_FOUND)
