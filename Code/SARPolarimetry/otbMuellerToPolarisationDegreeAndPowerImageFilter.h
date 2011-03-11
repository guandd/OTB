/*=========================================================================

  Program:   ORFEO Toolbox
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
  See OTBCopyright.txt for details.


     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __MuellerToPolarisationDegreeAndPowerImageFilter_h
#define __MuellerToPolarisationDegreeAndPowerImageFilter_h

#include "otbUnaryFunctorImageFilter.h"
#include "itkNumericTraits.h"
#include "itkMatrix.h"
#include "itkVector.h"
#include "otbMath.h"

namespace otb
 {

namespace Functor {

/** \class otbMuellerToPolarisationDegreeAndPowerFunctor
 * \brief Evaluate the  min and max polarisation degree and min and max power
 *   from the Mueller image
 *
 * \ingroup Functor
 * \ingroup SARPolarimetry
 *
 * \sa MuellerToCircularPolarisationImageFilter
 * \sa MuellerToMLCImageFilter
 *
 */
template< class TInput, class TOutput>
class MuellerToPolarisationDegreeAndPowerFunctor
{
public:
  typedef typename TOutput::ValueType               OutputValueType;
  typedef itk::Matrix<double, 4, 4>                 MuellerMatrixType;
  typedef itk::Vector<double, 4>                   StokesVectorType;

  inline TOutput operator()( const TInput & Mueller ) const
    {
    double P;
    double deg_pol;
    double tau;
    double psi;
    StokesVectorType Si;
    StokesVectorType Sr;

    double m_PowerMin(itk::NumericTraits<double>::max());
    double m_PowerMax(itk::NumericTraits<double>::min());
    double m_PolarisationDegreeMin(itk::NumericTraits<double>::max());
    double m_PolarisationDegreeMax(itk::NumericTraits<double>::min());

     TOutput result;
    result.SetSize(m_NumberOfComponentsPerPixel);

    MuellerMatrixType muellerMatrix;
    muellerMatrix[0][0] = Mueller[0];
    muellerMatrix[0][1] = Mueller[1];
    muellerMatrix[0][2] = Mueller[2];
    muellerMatrix[0][3] = Mueller[3];
    muellerMatrix[1][0] = Mueller[4];
    muellerMatrix[1][1] = Mueller[5];
    muellerMatrix[1][2] = Mueller[6];
    muellerMatrix[1][3] = Mueller[7];
    muellerMatrix[2][0] = Mueller[8];
    muellerMatrix[2][1] = Mueller[9];
    muellerMatrix[2][2] = Mueller[10];
    muellerMatrix[2][3] = Mueller[11];
    muellerMatrix[3][0] = Mueller[12];
    muellerMatrix[3][1] = Mueller[13];
    muellerMatrix[3][2] = Mueller[14];
    muellerMatrix[3][3] = Mueller[15];

    tau = -45.0;
    while (tau < 46.0)
      {
        psi = -90.0;
        while (psi < 91.0)
          {
            // Define the incident Stokes vector
            Si[0] = 1.0;
            Si[1] = cos(psi * m_PI_90) * cos(tau * m_PI_90);
            Si[2] = sin(psi * m_PI_90) * cos(tau * m_PI_90);
            Si[3] = sin(tau * m_PI_90);
            
            // Evaluate the received Stokes vector
            Sr = muellerMatrix * Si;
            
            //Evaluate Power and Polarisation degree
            P = Sr[0];
            
            if (P < m_Epsilon)
              {
                deg_pol = 0.;
              }
            else
              {
                deg_pol = vcl_sqrt(Sr[1] * Sr[1] + Sr[2] * Sr[2] + Sr[3] * Sr[3]) / Sr[0];
              }
            
            if (P > m_PowerMax)
              {
                m_PowerMax = P;
              }
            else
              {
                m_PowerMin = P;
              }
            
            if (deg_pol > m_PolarisationDegreeMax)
              {
                m_PolarisationDegreeMax = deg_pol;
              }
            else
              {
                m_PolarisationDegreeMin = deg_pol;
              }
            psi += 5.0;
          }
        tau += 5.0;
      }
    
    result[0] = m_PowerMin;
    result[1] = m_PowerMax;
    result[2] = m_PolarisationDegreeMin;
    result[3] = m_PolarisationDegreeMax;
    
    
    return result;
    }

  unsigned int GetOutputSize()
  {
    return m_NumberOfComponentsPerPixel;
  }

   /** Constructor */
   MuellerToPolarisationDegreeAndPowerFunctor() : m_NumberOfComponentsPerPixel(4), m_Epsilon(1e-6), m_PI_90(2*CONST_PI_180) {}

   /** Destructor */
   virtual ~MuellerToPolarisationDegreeAndPowerFunctor() {}

private:
    unsigned int m_NumberOfComponentsPerPixel;
    const double m_Epsilon;
    const double m_PI_90;
};
}


/** \class otbMuellerToPolarisationDegreeAndPowerImageFilter
 * \brief Compute the circular polarisation image (3 channels : LL, RR and LR)
 * from the Mueller image (16 real channels)
 */
template <class TInputImage, class TOutputImage, class TFunction = Functor::MuellerToPolarisationDegreeAndPowerFunctor<
    ITK_TYPENAME TInputImage::PixelType, ITK_TYPENAME TOutputImage::PixelType> >
class ITK_EXPORT MuellerToPolarisationDegreeAndPowerImageFilter :
   public UnaryFunctorImageFilter<TInputImage, TOutputImage, TFunction>
{
public:
   /** Standard class typedefs. */
   typedef MuellerToPolarisationDegreeAndPowerImageFilter  Self;
   typedef UnaryFunctorImageFilter<TInputImage, TOutputImage, TFunction> Superclass;
   typedef itk::SmartPointer<Self>        Pointer;
   typedef itk::SmartPointer<const Self>  ConstPointer;

   /** Method for creation through the object factory. */
   itkNewMacro(Self);

   /** Runtime information support. */
   itkTypeMacro(MuellerToPolarisationDegreeAndPowerImageFilter, UnaryFunctorImageFilter);


protected:
   MuellerToPolarisationDegreeAndPowerImageFilter() {}
  virtual ~MuellerToPolarisationDegreeAndPowerImageFilter() {}

private:
  MuellerToPolarisationDegreeAndPowerImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&);                                 //purposely not implemented
};
  
} // end namespace otb

#endif
