/*
The Original EM/MPM algorithm was developed by Mary L. Comer and is distributed
under the BSD License.
Copyright (c) <2010>, <Mary L. Comer>
All rights reserved.

[1] Comer, Mary L., and Delp, Edward J.,  ÒThe EM/MPM Algorithm for Segmentation
of Textured Images: Analysis and Further Experimental Results,Ó IEEE Transactions
on Image Processing, Vol. 9, No. 10, October 2000, pp. 1731-1744.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list
of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this
list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

Neither the name of <Mary L. Comer> nor the names of its contributors may be
used to endorse or promote products derived from this software without specific
prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Michael A. Jackson. BlueQuartz Software
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _EMMPM_TASK_H_
#define _EMMPM_TASK_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QString>
#include <QtGui/QImage>

#include "QtSupport/ProcessQueueTask.h"

#include "Random.h"

#define PI  3.14159265358979323846
#define MAX_CLASSES 15

#define MAXPRIME  2147483647       /*  MAXPRIME = (2^31)-1     */

#define UPDATE_PROGRESS(m, p)\
  emit progressTextChanged( (m) );\
  emit progressValueChanged( (p) );

/**
* @class EMMPMTask EMMPMTask.h EmMpm/GUI/EMMPMTask.h
* @brief This is the wrapper code for the code. This is called as a "worker" class from a separate thread
* of execution in order to not lock up the GUI.
* @author Michael A. Jackson for BlueQuartz Software
* @date Dec 20, 2009
* @version 1.0
*/
class EMMPMTask : public ProcessQueueTask
{

  Q_OBJECT;

  public:
    EMMPMTask(QObject* parent = 0);
    virtual ~EMMPMTask();

    void setBeta(float beta);
    void setGamma(float gamma);
    void setEmIterations(int emIterations);
    void setMpmIterations(int mpmIterations);
    void setNumberOfClasses(int numClasses);
    void useSimulatedAnnealing();

    /**
     *
     * @param imageData
     * @param outputData
     */
    template <class T, class S>
    void execute( T* imageData, S* outputData);

    /**
     *
     * @param imageData
     * @param segmentation
     * @param pr
     * @param yk
     * @param beta
     * @param gamma
     * @param mean_estimate
     * @param variance
     * @param idim
     * @param jdim
     * @param random
     */
    template <class T, class S>
    void mpm( T* imageData, S* segmentation,
                           double *pr[], double *yk[],
                           double beta, double *gamma,
                           double *mean_estimate, double *variance,
                           size_t idim, size_t jdim,
                           Random &random);


    virtual void run();

  private:

    float m_Beta;
    float m_Gamma;
    int m_EmIterations;
    int m_MpmIterations;
    int m_NumberOfClasses;
    bool m_UseSimulatedAnnealing;
    int m_TotalIterations;
    int m_CurrentIteration;

    AIMImage::Pointer m_OriginalImage;
    AIMImage::Pointer m_SegmentedImage;

    EMMPMTask(const EMMPMTask&); // Copy Constructor Not Implemented
    void operator=(const EMMPMTask&); // Operator '=' Not Implemented

};

template <class T, class S>
void EMMPMTask::mpm( T* imageData, S* segmentation,
                       double *pr[], double *yk[],
                       double beta, double *gamma,
                       double *mean_estimate, double *variance,
                       size_t idim, size_t jdim,
                       Random &random)
{
  double con[MAX_CLASSES], d[MAX_CLASSES], post[MAX_CLASSES];
  size_t prior[MAX_CLASSES];
  size_t ijdim = idim*jdim;
  size_t extent_ijdim = ijdim;
  double sqrt2pi = ::sqrt(PI * 2);
  int components = 1;

  uint32_t NumberClasses = m_NumberOfClasses;
  uint32_t MPMIterations = m_MpmIterations;
  for (uint32_t l = 0; l < NumberClasses; ++l)
  {
    con[l] = -::log(sqrt2pi * ::sqrt(variance[l]));
    d[l] = -2.0 * variance[l];
  }

  int index = 0;

  for (size_t i = 0; i < ijdim; ++i)
  {
    for (uint32_t l = 0; l < NumberClasses; ++l)
    {
      pr[l][index] = 0;
      yk[l][index] = con[l] + ((imageData[i] - mean_estimate[l]) * (imageData[i] - mean_estimate[l])/d[l]);
    }
    ++index;
  }


  for (uint32_t k = 0; k < MPMIterations; ++k)
  {
    if (isCanceled() == true) { break; }
    ++m_CurrentIteration;
    int progress = (float)m_CurrentIteration/(float)m_TotalIterations * 100.0f;
    emit progressValueChanged( progress );
    emit progressTextChanged(QString::number(progress));
    int extent_index = 0;
    for (size_t j = 0; j < jdim; ++j)
    {
      for (size_t i = 0; i < idim; ++i)
      {
        size_t index = j*idim*components+i*components;

        double sum = 0;
        for (uint32_t l = 0; l < NumberClasses; ++l)
        {
          prior[l] = 0;
          // 8 point neighborhood
          if (i > 0) // left
            if (segmentation[index-components] != l)
              prior[l]++;
          if (i < idim-1) // right
            if (segmentation[index+components] != l)
              prior[l]++;
          if (j > 0) // bottom
            if (segmentation[index-idim*components] != l)
              prior[l]++;
          if (j < jdim-1) // top
            if (segmentation[index+idim*components] != l)
              prior[l]++;
          if (i > 0 && j > 0) // bottom left
            if (segmentation[index-idim*components-components] != l)
              prior[l]++;
          if (i > 0 && j < jdim-1) // top left
            if (segmentation[index+idim*components-components] != l)
              prior[l]++;
          if (i < idim-1 && j > 0) // bottom right
            if (segmentation[index-idim*components+components] != l)
              prior[l]++;
          if (i < idim-1 && j < jdim-1) // bottom left
            if (segmentation[index+idim*components+components] != l)
              prior[l]++;

          post[l] = exp(yk[l][extent_index] - beta * (double)(prior[l]) - gamma[l]);
          sum += post[l];
        }
        double x = random.next();
        double current = 0;
        for (uint32_t l = 0; l < NumberClasses; ++l)
        {
          if (x >= current && x <= current+post[l]/sum)
          {
            segmentation[index] = l;//index] = l;
            pr[l][extent_index] += 1.0;
          }
          current += post[l]/sum;
        }
        ++extent_index;
      }
    }
  }
  if (isCanceled() == false)
  {
    // normalize probabilities
    for (uint32_t l = 0; l < NumberClasses; ++l)
    {
      for (size_t i = 0; i < extent_ijdim; ++i)
      {
        pr[l][i] = pr[l][i] / (double)MPMIterations;
      }
    }
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
template <class T, class S>
void EMMPMTask::execute( T* imageData, S* outputData)
{
  // data dimensions
  int32_t width, height;
  m_OriginalImage->getImagePixelDimension(width, height);
  size_t idim = width;
  size_t jdim = height;
  size_t ijdim = idim*jdim;
  size_t extent_ijdim = idim*jdim;

  // output pixels
  S *xt = outputData;
  Random random;
 // int components = 1;


  // ==========================================================================
  // Compute the Mean and Variance in one loop using a Maximum Liklehood Algo
  uint8_t t;
  double local_mu = 0.0;
  double local_sigma = 0.0;
  int32_t s0;
  int64_t s1, s2 = 0;
  s0 = 0;
  s1 = 0;
  for (size_t i = 0; i < ijdim; ++i)
  {
    t = imageData[i];
    s0 += t != 0; // Count number of non-zero elements.
    s1 += t;    // Accumulate sum.
    s2 += t*t;    // Accumulate sum of squares.
  }

  local_mu = (double) s1 / s0;
  local_sigma = (s2 - s1*local_mu)/s0;
  local_sigma = sqrt(local_sigma);
  // printf("mu=%f sigma=%f\n",local_mu,local_sigma);

  // initialize variables on main thread
  double local_mean_estimate[MAX_CLASSES];
  double local_variance[MAX_CLASSES];
  int NumberClasses = m_NumberOfClasses;
  if (true)//threadId == 0)
  {
    //fprintf(stderr, "mu=%f sigma=%f\n", local_mu, local_sigma);
    // initialize mean estimate
    if (NumberClasses % 2 == 0)
    {
      for (int k = 0; k < NumberClasses/2; ++k)
      {
        local_mean_estimate[NumberClasses/2 + k] = local_mu + (k+1)*local_sigma/2;
        local_mean_estimate[NumberClasses/2 - 1 - k] = local_mu - (k+1)*local_sigma/2;
      }
    }
    else
    {
      //shared_mean_estimate[NumberClasses/2] = local_mu;
      local_mean_estimate[NumberClasses/2] = local_mu;
      for (int k = 0; k < NumberClasses/2; ++k)
      {
        local_mean_estimate[NumberClasses/2 + 1 + k] = local_mu + (k+1)*local_sigma/2;
        local_mean_estimate[NumberClasses/2 - 1 - k] = local_mu - (k+1)*local_sigma/2;
      }
    }
    // initialize variance
    for (int k = 0; k < NumberClasses; ++k)
      local_variance[k] = 20;
      //shared_variance[k] = 20;
  }

  double Gamma = m_Gamma;

  // allocate needed data
  double *local_probs[MAX_CLASSES];
  double *local_yk[MAX_CLASSES];
  for (int k = 0; k < NumberClasses; ++k)
  {
    local_probs[k] = new double[extent_ijdim];
    local_yk[k] = new double[extent_ijdim];
  }


  int local_class_count[MAX_CLASSES];
  double gamma[MAX_CLASSES];
  for (int i = 0; i < NumberClasses; ++i)
  {
    local_class_count[i] = 0;
    gamma[i] = Gamma;
  }

  // initialize classifications of each pixel randomly with a uniform distribution
  for (size_t i = 0; i < ijdim; ++i)
  {
    if (outputData[i] != 0)
    {
      outputData[i] = static_cast<S>(random.next(0, 1)*(NumberClasses + 1));
    }
  }

  uint32_t EMIterations = m_EmIterations;
  double Beta = m_Beta;

  // create beta ramp for simulated annealing
  // ramps beta continuously from min to 10*min
  double* bt = new double[EMIterations];
  for (uint32_t i = 0; i < EMIterations; ++i)
  {
    bt[i] = Beta;
    if (m_UseSimulatedAnnealing == true)
    {
      bt[i] = bt[i] + ::pow(i/(EMIterations-1.0), 8) * (10.0*Beta - Beta);
    }
  }

  // random access for mpm
 // int *index = new int[extent_ijdim];
 // int temp_index = 0;
  m_TotalIterations = m_EmIterations * m_MpmIterations;
  m_CurrentIteration = 0;

//  printf("Beta: %f\n", m_Beta);
//  printf("Gamma: %f\n", m_Gamma);

  // perform EM
  for (uint32_t k = 0; k < EMIterations; ++k)
  {
    if (isCanceled() == true) { break; }
    double local_N[MAX_CLASSES];

    // perform MPM
    mpm<T>( imageData, xt, local_probs, local_yk, bt[k], gamma, local_mean_estimate, local_variance, idim, jdim, random);
    // reset model parameters to zero
    if (isCanceled() == true) { break; }

    /* Reset model parameters to zero */
    for (int l = 0; l < NumberClasses; ++l)
    {
      local_mean_estimate[l] = local_variance[l] = local_N[l] = 0.0;
    }

    /* Update estimates for mean of each class */
    for (int l = 0; l < NumberClasses; ++l)
    {
      int temp_index = 0;
      for (size_t i = 0; i < ijdim; ++i)
      {
        if (imageData[i] != 0)
        {
          local_N[l] += local_probs[l][temp_index];
          local_mean_estimate[l] += imageData[i] * local_probs[l][temp_index];
          ++temp_index;
        }
      }
    }

    // combine results
    for (int l = 0; l < NumberClasses; ++l) {
      if (local_N[l] != 0) {
        local_mean_estimate[l] = local_mean_estimate[l] / local_N[l];
//        std::cout << "local_mean_estimate[" << l << "]: " << local_mean_estimate[l] << std::endl;
//        std::cout << "local_N[" << l << "]: " << local_N[l] << std::endl;
      }
    }

    /* Update estimates of variance of each class */
    for (int l = 0; l < NumberClasses; ++l)
    {
      int temp_index = 0;

      for (size_t i = 0; i < ijdim; ++i)
      {
        if (imageData[i] != 0)
        {
          local_variance[l] += (imageData[i] - local_mean_estimate[l]) * (imageData[i] - local_mean_estimate[l])
              * local_probs[l][temp_index];
          ++temp_index;
        }
      }
    }

    // combine results
    for (int l = 0; l < NumberClasses; ++l)
    {
      if (local_N[l] != 0) {
        local_variance[l] = local_variance[l] / local_N[l];
  //      std::cout << "local_variance[" << l << "]: " << local_variance[l] << std::endl;
      }
      else
      {
       // printf("local_N[%d] = %f\n", l, local_N[l]);
      }
    }

    /* Monitor estimates of mean and variance */
    setDebug(true);
    if (getDebug())
    {
      if (EMIterations < 10 || (k + 1) % (EMIterations / 10) == 0)
      {
        for (int l = 0; l < NumberClasses ; l++) {
          printf("%d %.3f %.3f\n", l, local_mean_estimate[l], local_variance[l]);
        }
      }
    }


  } /* End of EM Loop */

  if (isCanceled() == false) {
    // scale range of classes to fit pixel depth
    for (size_t i = 0; i < ijdim; ++i)
    {
      if (outputData[i] != 0)
      {
        outputData[i] = static_cast<S>(outputData[i] * 255 / (NumberClasses - 1 ) );
        //outputData[i] = 175 + outputData[i];
      }
    }
  }
  //Clean up all the memory that got allocated:
  for (int k = 0; k < NumberClasses; ++k)
  {
    delete[] local_probs[k];
    delete[] local_yk[k];
  }
  delete[] bt;
}








#endif /* _EMMPM_TASK_H_ */
