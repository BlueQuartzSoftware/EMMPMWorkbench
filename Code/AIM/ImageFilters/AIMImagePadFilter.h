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

#ifndef AIMIMAGEPADFILTER_H_
#define AIMIMAGEPADFILTER_H_

#include "AIM/Common/AIMImage.h"
#include "AIM/ImageFilters/AIMImageFilter.h"
#include "AIM/Common/AIMLibExport.h"

/**
* @class AIMImagePadFilter AIMImagePadFilter.h AIM/ImageFilters/AIMImagePadFilter.h
* @brief This class will pad an existing image by the amount given by the "pixelPad" value in
* the constructor.
* @author Michael A. Jackson for BlueQuartz Software
* @date July 06, 2010
* @version 1.0
*/
class AIMLib_EXPORT AIMImagePadFilter : public AIMImageFilter
{
  public:
    /**
     *
     * @param imageData The AIMImage to pad
     * @param pixelPad The number of pixels to add to the left, right, top, bottom of the image
     * @param padValue The value of the pixel
     * @return
     */
    AIMImagePadFilter(AIMImage::Pointer imageData, int32_t pixelPad[2], int32_t padValue);
    virtual ~AIMImagePadFilter();

    /**
     * @brief Crops the input image
     * @return Error code
     */
    int run();

    /**
     * @brief Returns the output of this filter. The contained pointer may be NULL if there was
     * an error during the filtering process.
     * @return Smart Pointer to the cropped image
     */
    AIMImage::Pointer getOutputImage();

  protected:


  private:
    AIMImage::Pointer m_InData;
    int32_t m_PixelPad[4];
    int32_t m_PadValue;
    AIMImage::Pointer m_OutData;

    AIMImagePadFilter(const AIMImagePadFilter&); // Copy Constructor Not Implemented
    void operator=(const AIMImagePadFilter&); // Operator '=' Not Implemented

};

#endif /* AIMIMAGEPADFILTER_H_ */
