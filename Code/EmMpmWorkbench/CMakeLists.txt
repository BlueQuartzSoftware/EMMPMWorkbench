#--////////////////////////////////////////////////////////////////////////////
#-- Copyright (c) 2009, Michael A. Jackson. BlueQuartz Software
#-- All rights reserved.
#-- BSD License: http://www.opensource.org/licenses/bsd-license.html
#-- This code was partly written under US Air Force Contract FA8650-07-D-5800
#--////////////////////////////////////////////////////////////////////////////
PROJECT( EmMpmWorkbench )
cmake_minimum_required(VERSION 2.8.6)

set (CMP_HEADER_DIR ${EmMpmWorkbench_BINARY_DIR})
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

# --------------------------------------------------------------------
# Generate a Header file with Compile Version variables
# --------------------------------------------------------------------
if (EXISTS ${EMMPMGuiProj_SOURCE_DIR}/.git)
cmpVersionStringsFromGit(GENERATED_HEADER_FILE_PATH ${CMP_HEADER_DIR}/${CMP_VERSION_HEADER_FILE_NAME}
                         GENERATED_SOURCE_FILE_PATH ${CMP_HEADER_DIR}/${CMP_VERSION_SOURCE_FILE_NAME}
                         NAMESPACE EmMpm_Gui
                         cmpProjectName EMMPMGui
                         WORKING_DIRECTORY ${EMMPMGuiProj_SOURCE_DIR} )
else()
   cmpGenerateVersionString(GENERATED_HEADER_FILE_PATH ${CMP_HEADER_DIR}/${CMP_VERSION_HEADER_FILE_NAME}
                         GENERATED_SOURCE_FILE_PATH ${CMP_HEADER_DIR}/${CMP_VERSION_SOURCE_FILE_NAME}
                         NAMESPACE EmMpm_Gui
                         cmpProjectName EMMPMGui
                         WORKING_DIRECTORY ${EMMPMGuiProj_SOURCE_DIR}
                         MAJOR 0 MINOR 0 PATCH 0)
endif()

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
    include_directories(${QWT_INCLUDE_DIR}/qwt)
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
include_directories( ${PROJECT_CODE_DIR}/EmMpmWorkbench )
include_directories( ${PROJECT_CODE_DIR}/QtSupport )

include_directories( ${EmMpmWorkbench_BINARY_DIR} )


# --------------------------------------------------------------------
# Set up all the source codes
SET( EmMpmWorkbench_SRCS
  ${EmMpmWorkbench_SOURCE_DIR}/main.cpp
  ${EmMpmWorkbench_SOURCE_DIR}/EmMpmWorkbench.cpp
  ${EmMpmWorkbench_SOURCE_DIR}/EMMPMGraphicsView.cpp
  ${EmMpmWorkbench_SOURCE_DIR}/UserInitArea.cpp
  ${EmMpmWorkbench_SOURCE_DIR}/UserInitAreaTableModel.cpp
  ${EmMpmWorkbench_SOURCE_DIR}/UserInitAreaDialog.cpp
  ${EmMpmWorkbench_SOURCE_DIR}/UserInitAreaWidget.cpp
  ${EmMpmWorkbench_SOURCE_DIR}/ManualInitTableModel.cpp
  ${EmMpmWorkbench_SOURCE_DIR}/ManualInitData.cpp
  ${EmMpmWorkbench_SOURCE_DIR}/AxisSettingsDialog.cpp
#  ${EmMpmWorkbench_SOURCE_DIR}/LayersDockWidget.cpp
  ${EmMpmWorkbench_SOURCE_DIR}/EMMPMTask.cpp
  ${EmMpmWorkbench_SOURCE_DIR}/ImageOpenDialog.cpp
  ${EmMpmWorkbench_SOURCE_DIR}/PerClassTableModel.cpp
  ${PROJECT_CODE_DIR}/AIM/Common/AIMImage.cpp
  ${EmMpmWorkbench_SOURCE_DIR}/PerClassItemData.cpp
)

# another list, this time it includes all header files that should
#  be treated with moc
SET( EmMpmWorkbench_MOC_HDRS
  ${EmMpmWorkbench_SOURCE_DIR}/EmMpmWorkbench.h
  ${EmMpmWorkbench_SOURCE_DIR}/EMMPMGraphicsView.h
  ${EmMpmWorkbench_SOURCE_DIR}/UserInitArea.h
  ${EmMpmWorkbench_SOURCE_DIR}/UserInitAreaTableModel.h
  ${EmMpmWorkbench_SOURCE_DIR}/UserInitAreaDialog.h
  ${EmMpmWorkbench_SOURCE_DIR}/UserInitAreaWidget.h
  ${EmMpmWorkbench_SOURCE_DIR}/ManualInitTableModel.h
  ${EmMpmWorkbench_SOURCE_DIR}/ManualInitData.h
  ${EmMpmWorkbench_SOURCE_DIR}/ManualInitDataItemDelegate.h
  ${EmMpmWorkbench_SOURCE_DIR}/AxisSettingsDialog.h
#  ${EmMpmWorkbench_SOURCE_DIR}/LayersDockWidget.h
  ${EmMpmWorkbench_SOURCE_DIR}/UserInitAreaItemDelegate.h
  ${EmMpmWorkbench_SOURCE_DIR}/ImageOpenDialog.h
  ${EmMpmWorkbench_SOURCE_DIR}/EMMPMTask.h
  ${EmMpmWorkbench_SOURCE_DIR}/PerClassTableModel.h
  ${EmMpmWorkbench_SOURCE_DIR}/PerClassItemDelegate.h
  ${EmMpmWorkbench_SOURCE_DIR}/PerClassItemData.h
)
cmp_IDE_SOURCE_PROPERTIES(EmMpmWorkbench "${EmMpmWorkbench_MOC_HDRS}" "${EmMpmWorkbench_SRCS}" "0")


SET (EmMpmWorkbench_HDRS
#    ${EmMpmWorkbench_SOURCE_DIR}/histogram_item.h
    ${PROJECT_CODE_DIR}/AIM/Common/AIMImage.h
)
cmp_IDE_SOURCE_PROPERTIES(EmMpmWorkbench "${EmMpmWorkbench_HDRS}" "${EmMpmWorkbench_SRCS}" "0")



# The UI files (As generated by Qt Designer
SET( EmMpmWorkbench_UIS
 ${EmMpmWorkbench_SOURCE_DIR}/UI_Files/EmMpmWorkbench.ui
 ${EmMpmWorkbench_SOURCE_DIR}/UI_Files/UserInitAreaDialog.ui
 ${EmMpmWorkbench_SOURCE_DIR}/UI_Files/UserInitAreaWidget.ui
 ${EmMpmWorkbench_SOURCE_DIR}/UI_Files/AxisSettingsDialog.ui
 ${EmMpmWorkbench_SOURCE_DIR}/UI_Files/LayersDockWidget.ui
 ${EmMpmWorkbench_SOURCE_DIR}/UI_Files/ImageOpenDialog.ui
)
cmp_IDE_SOURCE_PROPERTIES(EmMpmWorkbench/UI_Files "${EmMpmWorkbench_UIS}" "" "0")

# Organize the .qrc files that were generated from the various license files
cmp_IDE_GENERATED_PROPERTIES(EmMpmWorkbench/Generated/License "${QRC_LICENSE_FILES}" "" "0")

# and finally any resource files
SET( EmMpmWorkbench_RCS
 ${PROJECT_RESOURCES_DIR}/icons/images/Icons.qrc
)
cmp_IDE_SOURCE_PROPERTIES(EmMpmWorkbench/QrcFiles "${EmMpmWorkbench_RCS}" "" "0")

# -- Run MOC and UIC on the necessary files
QT4_ADD_RESOURCES( Generated_RC_SRCS ${EmMpmWorkbench_RCS}  ${QRC_LICENSE_FILES})

# this will run uic on .ui files:
QT4_WRAP_UI( Generated_UI_HDRS ${EmMpmWorkbench_UIS} )

# and finally this will run moc:
QT4_WRAP_CPP( Generated_MOC_SRCS ${EmMpmWorkbench_MOC_HDRS} )

#-- Put the generated files into their own group for IDEs
cmp_IDE_GENERATED_PROPERTIES(EmMpmWorkbench/Generated "${Generated_UI_HDRS}" "${Generated_MOC_SRCS};${Generated_RC_SRCS}")

set (Generated_Version_SRCS
    ${CMP_HEADER_DIR}/${CMP_VERSION_HEADER_FILE_NAME}
    ${CMP_HEADER_DIR}/${CMP_VERSION_SOURCE_FILE_NAME}
)
cmp_IDE_GENERATED_PROPERTIES(EmMpmWorkbench/Generated "${Generated_Version_SRCS}" "")

# -- Finally set the Project Sources to include all the sources
SET (${PROJECT_NAME}_PROJECT_SRCS
  ${EmMpmWorkbench_SRCS}
  ${EmMpmWorkbench_MOC_HDRS}
  ${EmMpmWorkbench_HDRS}
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

