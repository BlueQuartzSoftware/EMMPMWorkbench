/* ============================================================================
 * Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of Michael A. Jackson nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#ifndef AIMIMAGEFILTER_H_
#define AIMIMAGEFILTER_H_

#include <itkImage.h>
#include <itkImportImageFilter.h>
#include <itkExtractImageFilter.h>

#include "AIM/Common/AIMLibExport.h"

namespace AIM
{
namespace ImageFilters
{
const unsigned int Dimension = 2;
}
}
typedef unsigned char PixelType;
typedef itk::ImportImageFilter<PixelType, AIM::ImageFilters::Dimension> ImportFilterType;
typedef itk::Image<PixelType, AIM::ImageFilters::Dimension> InputImageType;
typedef itk::Image<PixelType, AIM::ImageFilters::Dimension> OutputImageType;


/**
* @class AIMImageFilter AIMImageFilter.h AIM/ImageFilters/AIMImageFilter.h
* @brief Base class for other filters
* @author Michael A. Jackson for BlueQuartz Software
* @date Mar 19, 2009
* @version 1.0
*/
class AIMLib_EXPORT AIMImageFilter
{
  public:
    AIMImageFilter();
    virtual ~AIMImageFilter();

    /**
     * @brief Runs the filter
     * @return Non Zero on Success
     */
    virtual int run() = 0;

    /**
     * @brief Initializes an ITK import filter with a raw array of data
     * @param importFilter The import filter to initialize
     * @param imageData Pointer to the raw data
     * @param width The width of the image
     * @param height The height of the image
     */
    virtual void initializeImportFilter(ImportFilterType::Pointer importFilter,
                                                          PixelType* imageData,
                                                          int width, int height);


  private:
    AIMImageFilter(const AIMImageFilter&);    // Copy Constructor Not Implemented
    void operator=(const AIMImageFilter&);  // Operator '=' Not Implemented
};

#endif /* AIMIMAGEFILTER_H_ */
