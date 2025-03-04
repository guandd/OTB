/*
 * Copyright (C) 1999-2011 Insight Software Consortium
 * Copyright (C) 2005-2020 Centre National d'Etudes Spatiales (CNES)
 *
 * This file is part of Orfeo Toolbox
 *
 *     https://www.orfeo-toolbox.org/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef otbRadianceToImageImageFilter_h
#define otbRadianceToImageImageFilter_h

#include "otb_6S.h"
#include "otbUnaryImageFunctorWithVectorImageFilter.h"
#include "itkNumericTraits.h"
#include "otbMacro.h"


namespace otb
{
namespace Functor
{
/**
   * \class RadianceToImageImageFunctor
   * \brief Subtract beta to the Input and multiply by alpha.
   *
   * \sa RadianceToImageImageFilter
   * \ingroup Functor
   * \ingroup Radiometry
 *
 * \ingroup OTBOpticalCalibration
 */

template <class TInput, class TOutput>
class RadianceToImageImageFunctor
{
public:
  RadianceToImageImageFunctor() : m_Alpha(1.), m_Beta(0.)
  {
  }

  virtual ~RadianceToImageImageFunctor()
  {
  }

  void SetAlpha(double alpha)
  {
    m_Alpha = alpha;
  }
  void SetBeta(double beta)
  {
    m_Beta = beta;
  }
  double GetAlpha()
  {
    return m_Alpha;
  }
  double GetBeta()
  {
    return m_Beta;
  }

  inline TOutput operator()(const TInput& inPixel) const
  {
    TOutput outPixel;
    double  temp;
    temp     = (static_cast<double>(inPixel) - m_Beta) * m_Alpha;
    outPixel = static_cast<TOutput>(temp);
    return outPixel;
  }

private:
  double m_Alpha;
  double m_Beta;
};
}

/** \class RadianceToImageImageFilter
 *  \brief Convert a radiance value into raw image value
 *
 * Transform a radiance image into a classical image. For this it
 * uses the functor RadianceToImageImageFunctor calling for each component of each pixel.
 *
 *
 * For Spot image in the dimap format, the correction parameters are
 * retrieved automatically from the metadata
 *
 * \ingroup RadianceToImageImageFunctor
 * \ingroup Radiometry
 *
 * \example Radiometry/AtmosphericCorrectionSequencement.cxx
 *
 * \ingroup OTBOpticalCalibration
 */
template <class TInputImage, class TOutputImage>
class ITK_EXPORT RadianceToImageImageFilter
    : public UnaryImageFunctorWithVectorImageFilter<
          TInputImage, TOutputImage,
          typename Functor::RadianceToImageImageFunctor<typename TInputImage::InternalPixelType, typename TOutputImage::InternalPixelType>>
{
public:
  /**   Extract input and output images dimensions.*/
  itkStaticConstMacro(InputImageDimension, unsigned int, TInputImage::ImageDimension);
  itkStaticConstMacro(OutputImageDimension, unsigned int, TOutputImage::ImageDimension);

  /** "typedef" to simplify the variables definition and the declaration. */
  typedef TInputImage  InputImageType;
  typedef TOutputImage OutputImageType;
  typedef typename Functor::RadianceToImageImageFunctor<typename InputImageType::InternalPixelType, typename OutputImageType::InternalPixelType> FunctorType;

  /** "typedef" for standard classes. */
  typedef RadianceToImageImageFilter Self;
  typedef UnaryImageFunctorWithVectorImageFilter<InputImageType, OutputImageType, FunctorType> Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** object factory method. */
  itkNewMacro(Self);

  /** return class name. */
  itkTypeMacro(RadianceToImageImageFilter, UnaryImageFunctorWithVectorImageFiltermageFilter);

  /** Supported images definition. */
  typedef typename InputImageType::PixelType          InputPixelType;
  typedef typename InputImageType::InternalPixelType  InputInternalPixelType;
  typedef typename InputImageType::RegionType         InputImageRegionType;
  typedef typename OutputImageType::PixelType         OutputPixelType;
  typedef typename OutputImageType::InternalPixelType OutputInternalPixelType;
  typedef typename OutputImageType::RegionType        OutputImageRegionType;

  typedef typename itk::VariableLengthVector<double> VectorType;

  /** Image size "typedef" definition. */
  typedef typename InputImageType::SizeType SizeType;

  /** Set the absolute calibration gains. */
  itkSetMacro(Alpha, VectorType);

  /** Give the absolute calibration gains. */
  itkGetConstReferenceMacro(Alpha, VectorType);

  /** Set the absolute calibration bias. */
  itkSetMacro(Beta, VectorType);
  /** Give the absolute calibration bias. */
  itkGetConstReferenceMacro(Beta, VectorType);

protected:
  /** Constructor */
  RadianceToImageImageFilter()
  {
    m_Alpha.SetSize(0);
    m_Beta.SetSize(0);
  };

  /** Destructor */
  ~RadianceToImageImageFilter() override
  {
  }

  /** Update the functor list and input parameters */
  void BeforeThreadedGenerateData(void) override
  {
    const auto & metadata = this->GetInput()->GetImageMetadata();

    if (m_Alpha.GetSize() == 0 && metadata.HasBandMetadata(MDNum::PhysicalGain))
    {
      m_Alpha = metadata.GetAsVector(MDNum::PhysicalGain);
    }

    if (m_Beta.GetSize() == 0 && metadata.HasBandMetadata(MDNum::PhysicalBias))
    {
      m_Beta = metadata.GetAsVector(MDNum::PhysicalBias);
    }

    otbMsgDevMacro(<< "Dimension: ");
    otbMsgDevMacro(<< "m_Alpha.GetSize(): " << m_Alpha.GetSize());
    otbMsgDevMacro(<< "m_Beta.GetSize() : " << m_Beta.GetSize());
    otbMsgDevMacro(<< "this->GetInput()->GetNumberOfComponentsPerPixel() : " << this->GetInput()->GetNumberOfComponentsPerPixel());

    if ((m_Alpha.GetSize() != this->GetInput()->GetNumberOfComponentsPerPixel()) || (m_Beta.GetSize() != this->GetInput()->GetNumberOfComponentsPerPixel()))
    {
      itkExceptionMacro(<< "Alpha and Beta parameters should have the same size as the number of bands");
    }

    otbMsgDevMacro(<< "Using correction parameters: ");
    otbMsgDevMacro(<< "Alpha (gain): " << m_Alpha);
    otbMsgDevMacro(<< "Beta (bias):  " << m_Beta);

    this->GetFunctorVector().clear();
    for (unsigned int i = 0; i < this->GetInput()->GetNumberOfComponentsPerPixel(); ++i)
    {
      FunctorType functor;
      functor.SetAlpha(m_Alpha[i]);
      functor.SetBeta(m_Beta[i]);
      this->GetFunctorVector().push_back(functor);
    }
  }

private:
  /** Ponderation declaration*/
  VectorType m_Alpha;
  VectorType m_Beta;
};

} // end namespace otb

#endif
