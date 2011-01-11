#///////////////////////////////////////////////////////////////////////////////
#  Copyright (c) 2010, Michael A. Jackson. BlueQuartz Software
#  All rights reserved.
#  BSD License: http://www.opensource.org/licenses/bsd-license.html
#///////////////////////////////////////////////////////////////////////////////
project(EMMPMPlugin)

# --------------------------------------------------------------------
# Find the ITK Package
# --------------------------------------------------------------------
IF(ITK_FOUND)
  INCLUDE(${USE_ITK_FILE})
ELSE(ITK_FOUND)
  MESSAGE(FATAL_ERROR "Cannot build without ITK.  Please set ITK_DIR.")
ENDIF(ITK_FOUND)

# ------------------------------------------------------------------------------
#  Qt 4 Section
# ------------------------------------------------------------------------------
if(QT4_FOUND)
    include( ${QT_USE_FILE} )
else()
    message(FATAL_ERROR "Qt 4 is Needed for this project.")
endif(QT4_FOUND)


include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/..)
include_directories(${PROJECT_CODE_DIR} )
include_directories(${PROJECT_CODE_DIR}/QtSupport )

include_directories(${EIMApps_BINARY_DIR})
include_directories(${EIMApps_BINARY_DIR}/QtSupport)

set (EMMPMPlugin_HDRS
    ${PROJECT_SOURCE_DIR}/EMMPMPlugin.h
    ${PROJECT_SOURCE_DIR}/EMMPMInputUI.h
    ${PROJECT_SOURCE_DIR}/EMMPMTask.h
    ${PROJECT_SOURCE_DIR}/../QImageProcessingInputFrame.h
    )
    
set (EMMPMPlugin_SRCS 
    ${PROJECT_SOURCE_DIR}/EMMPMPlugin.cpp
    ${PROJECT_SOURCE_DIR}/EMMPMInputUI.cpp
    ${PROJECT_SOURCE_DIR}/EMMPMTask.cpp
    ${PROJECT_SOURCE_DIR}/../QImageProcessingInputFrame.cpp
    )
QT4_WRAP_UI( EMMPMPlugin_Generated_UI_HDRS 
            ${PROJECT_SOURCE_DIR}/EMMPMInputUI.ui )

QT4_WRAP_CPP( EMMPMPlugin_Generated_MOC_SRCS ${EMMPMPlugin_HDRS} )

set (EMMPMPlugin_PROJECT_SRCS
    ${EMMPMPlugin_SRCS}
    ${EMMPMPlugin_HDRS}
    ${PROJECT_SOURCE_DIR}/../QImageProcessingInterface.h
    ${EMMPMPlugin_Generated_MOC_SRCS} 
    ${EMMPMPlugin_Generated_UI_HDRS}
)

cmp_IDE_SOURCE_PROPERTIES("Code/" "${EMMPMPlugin_HDRS}" "${EMMPMPlugin_SRCS}" "0")
cmp_IDE_SOURCE_PROPERTIES("Generated" "${EMMPMPlugin_Generated_UI_HDRS}" "${EMMPMPlugin_Generated_MOC_SRCS}" "0")

# ---------- Setup output Directories -------------------------
if(NOT MSVC)
  SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${EIMApps_BINARY_DIR}/Bin/plugins  )
endif()
add_library(EMMPMPlugin MODULE ${EMMPMPlugin_PROJECT_SRCS})
target_link_libraries(EMMPMPlugin 
                    MXADataModel 
                    ${QT_QTCORE_LIBRARY}   
                    ${QT_QTGUI_LIBRARY}
                    ${ITK_LIBRARIES} 
                    QtSupport 
                    AIMLib )
PluginProperties(EMMPMPlugin "_debug" "${EIMApps_VERSION}" 
                "${IPHelper_BINARY_DIR}" "IPHelper_Plugins.txt")
if (NOT MSVC)
  SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${EIMApps_BINARY_DIR}/Bin  )
endif()
