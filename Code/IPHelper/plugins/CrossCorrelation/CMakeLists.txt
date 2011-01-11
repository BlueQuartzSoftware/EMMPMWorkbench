#///////////////////////////////////////////////////////////////////////////////
#  Copyright (c) 2010, Michael A. Jackson. BlueQuartz Software
#  All rights reserved.
#  BSD License: http://www.opensource.org/licenses/bsd-license.html
#///////////////////////////////////////////////////////////////////////////////
project(CrossCorrelationPlugin)

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

set (CrossCorrelationPlugin_HDRS
    ${PROJECT_SOURCE_DIR}/CrossCorrelationPlugin.h
    ${PROJECT_SOURCE_DIR}/CrossCorrelationInputUI.h
    ${PROJECT_SOURCE_DIR}/CrossCorrelationTask.h
    ${PROJECT_SOURCE_DIR}/../QImageProcessingInputFrame.h
    )

set (CrossCorrelationPlugin_SRCS 
    ${PROJECT_SOURCE_DIR}/CrossCorrelationPlugin.cpp
    ${PROJECT_SOURCE_DIR}/CrossCorrelationInputUI.cpp
    ${PROJECT_SOURCE_DIR}/CrossCorrelationTask.cpp
    ${PROJECT_SOURCE_DIR}/../QImageProcessingInputFrame.cpp
    )
    
QT4_WRAP_UI( CrossCorrelationPlugin_Generated_UI_HDRS 
    ${PROJECT_SOURCE_DIR}/CrossCorrelationInputUI.ui
    )

QT4_WRAP_CPP( CrossCorrelationPlugin_Generated_MOC_SRCS ${CrossCorrelationPlugin_HDRS} )

cmp_IDE_SOURCE_PROPERTIES("Code/" "${CrossCorrelationPlugin_HDRS}" "${CrossCorrelationPlugin_SRCS}" "0")
cmp_IDE_SOURCE_PROPERTIES("Generated" "${CrossCorrelationPlugin_Generated_UI_HDRS}" "${CrossCorrelationPlugin_Generated_MOC_SRCS}" "0")

set (CrossCorrelationPlugin_EXTRA_HDRS
    ${CrossCorrelationPlugin_SOURCE_DIR}/CrossCorrelation.h
    ${CrossCorrelationPlugin_SOURCE_DIR}/CrossCorrelationData.h
    ${CrossCorrelationPlugin_SOURCE_DIR}/CrossCorrelationTable.h
    ${PROJECT_SOURCE_DIR}/../QImageProcessingInterface.h
	)
set (CrossCorrelationPlugin_EXTRA_SRCS
    ${CrossCorrelationPlugin_SOURCE_DIR}/CrossCorrelation.cpp
    ${CrossCorrelationPlugin_SOURCE_DIR}/CrossCorrelationData.cpp
    ${CrossCorrelationPlugin_SOURCE_DIR}/CrossCorrelationTable.cpp
	)

cmp_IDE_SOURCE_PROPERTIES("Code/" "${CrossCorrelationPlugin_EXTRA_HDRS}" "${CrossCorrelationPlugin_EXTRA_SRCS}" "0")

set (CrossCorrelationPlugin_PROJECT_SRCS
    ${CrossCorrelationPlugin_EXTRA_SRCS}
	${CrossCorrelationPlugin_EXTRA_HDRS}
    ${CrossCorrelationPlugin_SRCS}
    ${CrossCorrelationPlugin_HDRS}
    ${CrossCorrelationPlugin_Generated_MOC_SRCS} 
    ${CrossCorrelationPlugin_Generated_UI_HDRS}
)

# ---------- Setup output Directories -------------------------
if(NOT MSVC)
  SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${EIMApps_BINARY_DIR}/Bin/plugins  )
endif()

add_library(CrossCorrelationPlugin MODULE ${CrossCorrelationPlugin_PROJECT_SRCS})
target_link_libraries(CrossCorrelationPlugin
                    MXADataModel 
                    ${QT_QTCORE_LIBRARY}   
                    ${QT_QTGUI_LIBRARY}
                    ${ITK_LIBRARIES} 
                    QtSupport 
                    AIMLib )
PluginProperties(CrossCorrelationPlugin "_debug" "${EIMApps_VERSION}" 
                "${IPHelper_BINARY_DIR}" "IPHelper_Plugins.txt")

if (NOT MSVC)
  SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${EIMApps_BINARY_DIR}/Bin  )
endif()
