#--////////////////////////////////////////////////////////////////////////////
#-- Copyright (c) 2009, Michael A. Jackson. BlueQuartz Software
#-- All rights reserved.
#-- BSD License: http://www.opensource.org/licenses/bsd-license.html
#-- This code was partly written under US Air Force Contract FA8650-07-D-5800
#--////////////////////////////////////////////////////////////////////////////
PROJECT( EmMpmGui )
cmake_minimum_required(VERSION 2.8)

set (CMP_HEADER_DIR ${EmMpmGui_BINARY_DIR})
set (CMP_VERSION_HEADER_FILE_NAME "EMMPMGuiVersion.h")
set (CMP_VERSION_SOURCE_FILE_NAME "EMMPMGuiVersion.cpp")

# --------------------------------------------------------------------
# In order to increment the version of EMMPMGuiProj simply adjust these variables.
# we are using a 3 part Version scheme commonly found in software development. The
# example would be Version 2.1.6543 which translates to Version [major].[minor].[patch].
# The Major version is set here in the CMake file. We will use Git 
# to figure out the "patch" leve of the version by using "git describe" which will
# calculate how many commits since the last "tag" was applied to the repository.
# this basically gives us an always incrementing number for the patch level
# and we can set the Major and Minor versions as we see fit. 
set(CMP_VERSION_MAJOR "v2")

cmpVersionStringsFromGit(GENERATED_HEADER_FILE_PATH ${CMP_HEADER_DIR}/${CMP_VERSION_HEADER_FILE_NAME} 
                         GENERATED_SOURCE_FILE_PATH ${CMP_HEADER_DIR}/${CMP_VERSION_SOURCE_FILE_NAME}
                         NAMESPACE EmMpm_Gui 
                         cmpProjectName EMMPMGui 
                         WORKING_DIRECTORY ${EMMPMGuiProj_SOURCE_DIR} )
                         
# ------------------------------------------------------------------------------
#  Qt 4 Section
# ------------------------------------------------------------------------------
if(QT4_FOUND)
    include( ${QT_USE_FILE} )
    CMP_COPY_QT4_RUNTIME_LIBRARIES( "QtSvg;QtScript" )
    CMP_QT_LIBRARIES_INSTALL_RULES("QtSvg;QtScript" ".")
else()
    message(FATAL_ERROR "Qt 4 is Needed for this project.")
endif(QT4_FOUND)

# Set the GUI type for different platforms
SET(GUI_TYPE )

# ------------------------------------------------------------------------------
#  Qwt Section
# ------------------------------------------------------------------------------
find_package(Qwt)
if (QWT_FOUND)
    include_directories(${QWT_INCLUDE_DIR})
    CMP_COPY_DEPENDENT_LIBRARIES("qwt")
    CMP_LIBRARIES_INSTALL_RULES("qwt" ".")
else()
    message(FATAL_ERROR "Qwt is required for this project")
endif()

# ------------------------------------------------------------------------------
#  Tiff Library Section
# ------------------------------------------------------------------------------
FIND_PACKAGE(Tiff)
IF (NOT TIFF_FOUND)
      MESSAGE (FATAL_ERROR " Tiff Library was NOT Found and is needed.")
ENDIF (NOT TIFF_FOUND)
#CMP_COPY_DEPENDENT_LIBRARIES("tiff")
#CMP_LIBRARIES_INSTALL_RULES("tiff" ".")
INCLUDE_DIRECTORIES(${TIFF_INCLUDE_DIRS})

set(PROJECT_PREFIX "QEMMPM")
include(${EMMPMGuiProj_SOURCE_DIR}/License/License.cmake)

# Set some Win32 Specific Settings
IF(WIN32)
 SET(GUI_TYPE WIN32)
ENDIF(WIN32)

# Set some include directories
include_directories( ${PROJECT_CODE_DIR} )
include_directories( ${PROJECT_CODE_DIR}/EmMpmGui )
include_directories( ${PROJECT_CODE_DIR}/QtSupport )

include_directories( ${EmMpmGui_BINARY_DIR} )

 
# --------------------------------------------------------------------
# Set up all the source codes
SET( EmMpmGui_SRCS
  ${EmMpmGui_SOURCE_DIR}/main.cpp
  ${EmMpmGui_SOURCE_DIR}/EmMpmGui.cpp
  ${EmMpmGui_SOURCE_DIR}/EMMPMGraphicsView.cpp
  ${EmMpmGui_SOURCE_DIR}/UserInitArea.cpp
  ${EmMpmGui_SOURCE_DIR}/UserInitAreaTableModel.cpp
  ${EmMpmGui_SOURCE_DIR}/UserInitAreaDialog.cpp
  ${EmMpmGui_SOURCE_DIR}/UserInitAreaWidget.cpp
  ${EmMpmGui_SOURCE_DIR}/ManualInitTableModel.cpp
  ${EmMpmGui_SOURCE_DIR}/ManualInitData.cpp
  ${EmMpmGui_SOURCE_DIR}/AxisSettingsDialog.cpp
  ${EmMpmGui_SOURCE_DIR}/LayersDockWidget.cpp
  ${EmMpmGui_SOURCE_DIR}/EMMPMTask.cpp
  ${EmMpmGui_SOURCE_DIR}/ImageOpenDialog.cpp
  ${PROJECT_CODE_DIR}/AIM/Common/AIMImage.cpp
)

# another list, this time it includes all header files that should 
#  be treated with moc
SET( EmMpmGui_MOC_HDRS
  ${EmMpmGui_SOURCE_DIR}/EmMpmGui.h
  ${EmMpmGui_SOURCE_DIR}/EMMPMGraphicsView.h
  ${EmMpmGui_SOURCE_DIR}/UserInitArea.h
  ${EmMpmGui_SOURCE_DIR}/UserInitAreaTableModel.h
  ${EmMpmGui_SOURCE_DIR}/UserInitAreaDialog.h
  ${EmMpmGui_SOURCE_DIR}/UserInitAreaWidget.h
  ${EmMpmGui_SOURCE_DIR}/ManualInitTableModel.h
  ${EmMpmGui_SOURCE_DIR}/ManualInitData.h
  ${EmMpmGui_SOURCE_DIR}/ManualInitDataItemDelegate.h
  ${EmMpmGui_SOURCE_DIR}/AxisSettingsDialog.h
  ${EmMpmGui_SOURCE_DIR}/LayersDockWidget.h
  ${EmMpmGui_SOURCE_DIR}/UserInitAreaItemDelegate.h
  ${EmMpmGui_SOURCE_DIR}/ImageOpenDialog.h
  ${EmMpmGui_SOURCE_DIR}/EMMPMTask.h
)
cmp_IDE_SOURCE_PROPERTIES(EmMpmGui "${EmMpmGui_MOC_HDRS}" "${EmMpmGui_SRCS}" "0")


SET (EmMpmGui_HDRS
#    ${EmMpmGui_SOURCE_DIR}/histogram_item.h
    ${PROJECT_CODE_DIR}/AIM/Common/AIMImage.h
)
cmp_IDE_SOURCE_PROPERTIES(EmMpmGui "${EmMpmGui_HDRS}" "${EmMpmGui_SRCS}" "0")



# The UI files (As generated by Qt Designer
SET( EmMpmGui_UIS
 ${EmMpmGui_SOURCE_DIR}/UI_Files/EmMpmGui.ui
 ${EmMpmGui_SOURCE_DIR}/UI_Files/UserInitAreaDialog.ui
 ${EmMpmGui_SOURCE_DIR}/UI_Files/UserInitAreaWidget.ui
 ${EmMpmGui_SOURCE_DIR}/UI_Files/AxisSettingsDialog.ui
 ${EmMpmGui_SOURCE_DIR}/UI_Files/LayersDockWidget.ui
 ${EmMpmGui_SOURCE_DIR}/UI_Files/ImageOpenDialog.ui
)
cmp_IDE_SOURCE_PROPERTIES(EmMpmGui/UI_Files "${EmMpmGui_UIS}" "" "0")

# Organize the .qrc files that were generated from the various license files
cmp_IDE_GENERATED_PROPERTIES(EmMpmGui/Generated/License "${QRC_LICENSE_FILES}" "" "0")

# and finally any resource files
SET( EmMpmGui_RCS
 ${PROJECT_RESOURCES_DIR}/icons/images/Icons.qrc
)
cmp_IDE_SOURCE_PROPERTIES(EmMpmGui/QrcFiles "${EmMpmGui_RCS}" "" "0")

# -- Run MOC and UIC on the necessary files   
QT4_ADD_RESOURCES( Generated_RC_SRCS ${EmMpmGui_RCS}  ${QRC_LICENSE_FILES})

# this will run uic on .ui files:
QT4_WRAP_UI( Generated_UI_HDRS ${EmMpmGui_UIS} )

# and finally this will run moc:
QT4_WRAP_CPP( Generated_MOC_SRCS ${EmMpmGui_MOC_HDRS} )

#-- Put the generated files into their own group for IDEs
cmp_IDE_GENERATED_PROPERTIES(EmMpmGui/Generated "${Generated_UI_HDRS}" "${Generated_MOC_SRCS};${Generated_RC_SRCS}")

set (Generated_Version_SRCS
    ${CMP_HEADER_DIR}/${CMP_VERSION_HEADER_FILE_NAME}
    ${CMP_HEADER_DIR}/${CMP_VERSION_SOURCE_FILE_NAME}
)
cmp_IDE_GENERATED_PROPERTIES(EmMpmGui/Generated "${Generated_Version_SRCS}" "")

# -- Finally set the Project Sources to include all the sources
SET (${PROJECT_NAME}_PROJECT_SRCS 
  ${EmMpmGui_SRCS} 
  ${EmMpmGui_MOC_HDRS}
  ${EmMpmGui_HDRS}
  ${Generated_RC_SRCS} 
  ${Generated_UI_HDRS} 
  ${Generated_MOC_SRCS}
  ${Generated_Version_SRCS}
)

IF(APPLE)
    set(APPLICATION_ICON ${PROJECT_RESOURCES_DIR}/icons/icns/${PROJECT_NAME}.icns)
ENDIF (APPLE)

IF(Q_WS_WIN)
    set(APPLICATION_ICON ${PROJECT_RESOURCES_DIR}/icons/ico/${PROJECT_NAME}.ico)
ENDIF(Q_WS_WIN)

# Clear the file first otherwise we get duplicates
file(WRITE ${PROJECT_BINARY_DIR}/Qt_Plugins.txt "")
FindQt4Plugins("qgif;qjpeg;qtiff" "${PROJECT_BINARY_DIR}/Qt_Plugins.txt" 
                "${PROJECT_BINARY_DIR}/LibSearchDirs.txt" 
                "imageformats" )
                
file(READ ${PROJECT_BINARY_DIR}/Qt_Plugins.txt QT_PLUGINS)
BuildQtAppBundle(
    TARGET ${PROJECT_NAME}
    SOURCES ${${PROJECT_NAME}_PROJECT_SRCS}
    DEBUG_EXTENSION ${EXE_DEBUG_EXTENSION}
    ICON_FILE  ${APPLICATION_ICON}
    VERSION_MAJOR ${QEMMPM_VER_MAJOR}
    VERSION_MINOR ${QEMMPM_VER_MINOR}
    VERSION_PATCH ${QEMMPM_VER_PATCH}
    BINARY_DIR    ${${PROJECT_NAME}_BINARY_DIR}
    LINK_LIBRARIES ${QT_LIBRARIES} MXADataModel EMMPMLib QtSupport ${QWT_LIBRARIES} 
    LIB_SEARCH_DIRS ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    QT_PLUGINS    ${QT_PLUGINS}
#        OTHER_PLUGINS ${OTHER_PLUGINS}
    COMPONENT     Applications
    INSTALL_DEST  "."
)   

