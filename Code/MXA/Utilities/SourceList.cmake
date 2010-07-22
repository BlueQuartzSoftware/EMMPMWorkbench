SET (MXA_Utilities_SRCS
  ${MXA_SOURCE_DIR}/MXA/Utilities/MXADir.cpp
  ${MXA_SOURCE_DIR}/MXA/Utilities/MXAFileInfo.cpp
  ${MXA_SOURCE_DIR}/MXA/Utilities/MXALogger.cpp
)

SET (MXA_Utilities_HEADERS
  ${MXA_SOURCE_DIR}/MXA/Utilities/StringUtils.h
  ${MXA_SOURCE_DIR}/MXA/Utilities/MXADir.h
  ${MXA_SOURCE_DIR}/MXA/Utilities/MXAFileInfo.h
  ${MXA_SOURCE_DIR}/MXA/Utilities/MXALogger.h
)

IF (MSVC)
    SET (MXA_Utilities_SRCS ${MXA_Utilities_SRCS} ${MXA_SOURCE_DIR}/MXA/Utilities/MXADirent.c )
    SET (MXA_Utilities_HEADERS ${MXA_Utilities_HEADERS} ${MXA_SOURCE_DIR}/MXA/Utilities/MXADirent.h )
endif()

if ( ${MXA_INSTALL_FILES} EQUAL 1 )
    INSTALL (FILES ${MXA_Utilities_HEADERS}
            DESTINATION include/MXA/Utilities
            COMPONENT Headers   )
endif()
IDE_SOURCE_PROPERTIES( "MXA/Utilities" "${MXA_Utilities_HEADERS}" "${MXA_Utilities_SRCS}")
