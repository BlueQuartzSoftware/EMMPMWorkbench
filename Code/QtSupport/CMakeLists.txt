#///////////////////////////////////////////////////////////////////////////////
#  Copyright (c) 2010, Michael A. Jackson. BlueQuartz Software
#  All rights reserved.
#  BSD License: http://www.opensource.org/licenses/bsd-license.html
#///////////////////////////////////////////////////////////////////////////////
project(QtSupport)

if(QT4_FOUND)
    include( ${QT_USE_FILE} )
else()
    message(FATAL_ERROR "Qt 4 is Needed for this project.")
endif(QT4_FOUND)


set (Project_SRCS "")
set (Project_HDRS "")

set (QtSupport_SRCS
    ${QtSupport_SOURCE_DIR}/ApplicationAboutBoxDialog.cpp
    ${QtSupport_SOURCE_DIR}/QRecentFileList.cpp
    ${QtSupport_SOURCE_DIR}/QFileCompleter.cpp
    ${QtSupport_SOURCE_DIR}/ImageGraphicsDelegate.cpp
    ${QtSupport_SOURCE_DIR}/QFSDroppableGraphicsView.cpp
#    ${QtSupport_SOURCE_DIR}/ProcessQueueDialog.cpp
    ${QtSupport_SOURCE_DIR}/ProcessQueueController.cpp
    ${QtSupport_SOURCE_DIR}/ProcessQueueTask.cpp
    ${QtSupport_SOURCE_DIR}/ColorComboPicker.cpp
    ${QtSupport_SOURCE_DIR}/QFSDropLineEdit.cpp
    ${QtSupport_SOURCE_DIR}/QProgressLabel.cpp
    ${QtSupport_SOURCE_DIR}/QDisclosableGroupBox.cpp
)
set (QtSupport_HDRS
    ${QtSupport_SOURCE_DIR}/ApplicationAboutBoxDialog.h
    ${QtSupport_SOURCE_DIR}/QRecentFileList.h
    ${QtSupport_SOURCE_DIR}/QFileCompleter.h
    ${QtSupport_SOURCE_DIR}/ImageGraphicsDelegate.h
    ${QtSupport_SOURCE_DIR}/QFSDroppableGraphicsView.h
#    ${QtSupport_SOURCE_DIR}/ProcessQueueDialog.h
    ${QtSupport_SOURCE_DIR}/ProcessQueueController.h
    ${QtSupport_SOURCE_DIR}/ProcessQueueTask.h
    ${QtSupport_SOURCE_DIR}/ColorComboPicker.h
    ${QtSupport_SOURCE_DIR}/QFSDropLineEdit.h
    ${QtSupport_SOURCE_DIR}/QProgressLabel.h
    ${QtSupport_SOURCE_DIR}/QDisclosableGroupBox.h   
)
cmp_IDE_SOURCE_PROPERTIES("QtSupport" "${QtSupport_HDRS}" "${QtSupport_SRCS}" "0")


# --------------------------------------------------------------------
# The UI files As generated by Qt Designer
# --------------------------------------------------------------------
SET( MXA_QT_UI_FILES
#  ${QtSupport_SOURCE_DIR}/UI_Files/ProcessQueueDialog.ui
  ${QtSupport_SOURCE_DIR}/UI_Files/ApplicationAboutBoxDialog.ui
)
cmp_IDE_SOURCE_PROPERTIES( "QtSupport/UI_Files" "${MXA_QT_UI_FILES}" "" "0")

# and finally any resource files
#SET( QtSupport_RCS ${PROJECT_RESOURCES_DIR}/icons/images/Icons.qrc)

# --------------------------------------------------------------------
# Run on the files   
#QT4_ADD_RESOURCES( QtSupport_Generated_RC_SRCS ${QCrossCorrelation_RCS} )

# --------------------------------------------------------------------
# this will run uic on .ui files:
QT4_WRAP_UI( QtSupport_Generated_UI_HDRS ${MXA_QT_UI_FILES} )

# --------------------------------------------------------------------
# and finally this will run moc:
QT4_WRAP_CPP( QtSupport_Generated_MOC_SRCS ${QtSupport_HDRS} )

# --------------------------------------------------------------------
#-- Put the generated files into their own group for IDEs
cmp_IDE_SOURCE_PROPERTIES( "Generated" "${QtSupport_Generated_UI_HDRS}" "${QtSupport_Generated_MOC_SRCS};${QtSupport_Generated_RC_SRCS}" "0")



set (Project_SRCS ${Project_SRCS} 
        ${QtSupport_SRCS} 
        ${QtSupport_HDRS}
        ${QtSupport_Generated_UI_HDRS}
        ${QtSupport_Generated_MOC_SRCS}
        )

include_directories(${QtSupport_SOURCE_DIR}/..)
include_directories(${QtSupport_BINARY_DIR})
include_directories(${QtSupport_BINARY_DIR}/..)
add_definitions(-DQT_CORE_LIB)

add_library(QtSupport STATIC ${Project_SRCS})
target_link_libraries(QtSupport 
                    ${QT_QTCORE_LIBRARY} 
                    ${QT_QTGUI_LIBRARY} 
                    )

LibraryProperties( QtSupport ${EXE_DEBUG_EXTENSION})


