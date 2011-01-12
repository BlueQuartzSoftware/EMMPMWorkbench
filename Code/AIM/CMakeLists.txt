#///////////////////////////////////////////////////////////////////////////////
#  Copyright (c) 2010, Michael A. Jackson. BlueQuartz Software
#  All rights reserved.
#  BSD License: http://www.opensource.org/licenses/bsd-license.html
#///////////////////////////////////////////////////////////////////////////////
project(AIMLib)

IF(ITK_FOUND)
  INCLUDE(${USE_ITK_FILE})
ELSE(ITK_FOUND)
  MESSAGE(FATAL_ERROR "Cannot build without ITK.  Please set ITK_DIR.")
ENDIF(ITK_FOUND)

if(QT4_FOUND)
    include( ${QT_USE_FILE} )
else()
    message(FATAL_ERROR "Qt 4 is Needed for this project.")
endif(QT4_FOUND)

# Set this export symbol so classes are properly exported on Windows
SET (${PROJECT_NAME}_BUILT_AS_DYNAMIC_LIB)
IF (BUILD_SHARED_LIBS)
    SET (${PROJECT_NAME}_BUILT_AS_DYNAMIC_LIB 1)
    IF (WIN32)
        ADD_DEFINITIONS("-D${PROJECT_NAME}_BUILT_AS_DYNAMIC_LIB")
    ENDIF (WIN32)
ENDIF (BUILD_SHARED_LIBS)


set (Project_SRCS "")

Macro(MXACompileDir DirName)
    include(${AIMLib_SOURCE_DIR}/${DirName}/SourceList.cmake)
    set (Project_SRCS ${Project_SRCS} ${AIM_${DirName}_SRCS} ${AIM_${DirName}_HDRS})
endmacro()

MXACompileDir(Common)
MXACompileDir(ImageFilters)

include_directories(${AIMLib_SOURCE_DIR}/..)
include_directories(${AIMLib_BINARY_DIR})
include_directories(${AIMLib_BINARY_DIR}/..)
add_definitions(-DQT_CORE_LIB)


add_library(${PROJECT_NAME} ${LIB_TYPE} ${Project_SRCS})
target_link_libraries(${PROJECT_NAME} ${ITK_LIBRARIES} ${QT_QTCORE_LIBRARY})

LibraryProperties( ${PROJECT_NAME} ${EXE_DEBUG_EXTENSION})
set(install_dir "tools")
set(lib_install_dir "lib")
if (WIN32)
    set (install_dir ".")
    set(lib_install_dir "")
endif()

if (BUILD_SHARED_LIBS)
    INSTALL(TARGETS ${PROJECT_NAME} 
        COMPONENT Applications
        RUNTIME DESTINATION ${install_dir}
        LIBRARY DESTINATION ${lib_install_dir}
        ARCHIVE DESTINATION ${lib_install_dir}       
        BUNDLE DESTINATION "."
    ) 
endif() 