#///////////////////////////////////////////////////////////////////////////////
#  Copyright (c) 2010, Michael A. Jackson. BlueQuartz Software
#  All rights reserved.
#  BSD License: http://www.opensource.org/licenses/bsd-license.html
#///////////////////////////////////////////////////////////////////////////////
project(MutualInformation)

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


include_directories(${MutualInformation_SOURCE_DIR}  ${PROJECT_BINARY_DIR})
include_directories(${PROJECT_CODE_DIR} )
include_directories(${EIMApps_BINARY_DIR})
include_directories(${EIMApps_BINARY_DIR}/MXA)


set (MutualInformation_HDRS
    ${MutualInformation_SOURCE_DIR}/MutualInformation.h
    ${MutualInformation_SOURCE_DIR}/EMMPMInputUI.h
    ${MutualInformation_SOURCE_DIR}/EMMPMTask.h
    ${MutualInformation_SOURCE_DIR}/../QImageProcessingInputFrame.h
    )
    
set (MutualInformation_SRCS 
    ${MutualInformation_SOURCE_DIR}/MutualInformation.cpp
    ${MutualInformation_SOURCE_DIR}/EMMPMInputUI.cpp
    ${MutualInformation_SOURCE_DIR}/EMMPMTask.cpp
    ${MutualInformation_SOURCE_DIR}/../QImageProcessingInputFrame.cpp
    )
QT4_WRAP_UI( MutualInformation_Generated_UI_HDRS 
            ${MutualInformation_SOURCE_DIR}/EMMPMInputUI.ui )

QT4_WRAP_CPP( MutualInformation_Generated_MOC_SRCS ${MutualInformation_HDRS} )

set (MutualInformation_PROJECT_SRCS
#    ${MutualInformation_SRCS}
#    ${MutualInformation_HDRS}
#    ${MutualInformation_Generated_MOC_SRCS} 
#    ${MutualInformation_Generated_UI_HDRS}
    ${MutualInformation_SOURCE_DIR}/MITest.cpp
    ${MutualInformation_SOURCE_DIR}/CMUMutualInformation.cpp
    ${MutualInformation_SOURCE_DIR}/CMUMutualInformation.h
)

# ---------- Setup output Directories -------------------------
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${EIMApps_BINARY_DIR}/Bin/plugins  )
#add_library(MutualInformation STATIC ${MutualInformation_PROJECT_SRCS})
add_executable(MITest ${MutualInformation_PROJECT_SRCS})
target_link_libraries(MITest ${QT_LIBRARIES} MXADataModel AIMLib)
#PluginProperties(MutualInformation "_debug")
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${EIMApps_BINARY_DIR}/Bin  )


# --------------------------------------------------------------------
#  Add an executable based on the ITK Mutual Information Classes
set (ITK_MI_TEST_SRCS  
    ${PROJECT_CODE_DIR}/AngReader/AngDirectoryPatterns.cpp
    ${PROJECT_CODE_DIR}/AngReader/AngFileHelper.cpp
    ${PROJECT_CODE_DIR}/AngReader/AngReader.cpp
    ${PROJECT_CODE_DIR}/AngReader/itkAngImageIO.cpp
    ${PROJECT_CODE_DIR}/AngReader/itkAngImageIOFactory.cpp
    ${MutualInformation_SOURCE_DIR}/itkMITest.cpp
)

set (ITK_MI_TEST_HDRS  
    ${PROJECT_CODE_DIR}/AngReader/AngConstants.h
    ${PROJECT_CODE_DIR}/AngReader/AngDirectoryPatterns.h
    ${PROJECT_CODE_DIR}/AngReader/AngFileHelper.h
    ${PROJECT_CODE_DIR}/AngReader/AngReader.h
    ${PROJECT_CODE_DIR}/AngReader/AngUtils.hpp
    ${PROJECT_CODE_DIR}/AngReader/OIMColoring.hpp
    ${PROJECT_CODE_DIR}/AngReader/itkAngImageIO.h
    ${PROJECT_CODE_DIR}/AngReader/itkAngImageIOFactory.h
)

add_executable(itkMITest ${ITK_MI_TEST_SRCS} ${ITK_MI_TEST_HDRS} )
target_link_libraries(itkMITest ${ITK_LIBRARIES} ${QT_QTCORE_LIBRARY})


set (ir9_SRCS 
    ${PROJECT_CODE_DIR}/AngReader/itkAngImageIO.cpp
    ${PROJECT_CODE_DIR}/AngReader/itkAngImageIOFactory.cpp
    ${MutualInformation_SOURCE_DIR}/ImageRegistration9.cxx
    )
set (ir9_HDRS
    ${PROJECT_CODE_DIR}/AngReader/itkAngImageIO.h
    ${PROJECT_CODE_DIR}/AngReader/itkAngImageIOFactory.h
)


add_executable(ir9  ${ir9_SRCS} ${ir9_HDRS})
target_link_libraries(ir9  ${ITK_LIBRARIES} AngReader)


