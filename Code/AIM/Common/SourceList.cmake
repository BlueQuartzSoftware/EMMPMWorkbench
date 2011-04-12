#--////////////////////////////////////////////////////////////////////////////
#-- Copyright (c) 2009, Michael A. Jackson. BlueQuartz Software
#-- All rights reserved.
#-- BSD License: http://www.opensource.org/licenses/bsd-license.html
#-- This code was partly written under US Air Force Contract FA8650-07-D-5800
#--////////////////////////////////////////////////////////////////////////////
set (AIM_Common_SRCS
    ${PROJECT_CODE_DIR}/AIM/Common/AIMImage.cpp
)
set (AIM_Common_HDRS
    ${PROJECT_CODE_DIR}/AIM/Common/AIMArray.hpp
    ${PROJECT_CODE_DIR}/AIM/Common/AIMImage.h   )
cmp_IDE_SOURCE_PROPERTIES("AIM/Common" "${AIM_Common_HDRS}" "${AIM_Common_SRCS}" "0")
