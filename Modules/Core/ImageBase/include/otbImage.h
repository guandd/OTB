/*
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

#ifndef otbImage_h
#define otbImage_h

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "itkImage.h"
#pragma GCC diagnostic pop
#else
#include "itkImage.h"
#endif

#include "otbImageCommons.h"
#include "OTBImageBaseExport.h"

namespace otb
{
namespace internal
{
template <class ImageType>
typename ImageType::SpacingType GetSignedSpacing(const ImageType* input)
{
  typename ImageType::SpacingType   spacing   = input->GetSpacing();
  typename ImageType::DirectionType direction = input->GetDirection();
  for (unsigned int i = 0; i < ImageType::ImageDimension; i++)
  {
    spacing[i] *= direction[i][i];
  }
  return spacing;
}

template <class InputImage, typename SpacingType>
void SetSignedSpacing(InputImage* input, SpacingType spacing)
{
  // TODO check for spacing size ==> error
  typename InputImage::DirectionType direction = input->GetDirection();
  for (unsigned int i = 0; i < InputImage::ImageDimension; i++)
  {
    // TODO check if spacing[i] = 0 ==> error
    if (spacing[i] < 0)
    {
      if (direction[i][i] > 0)
      {
        for (unsigned int j = 0; j < InputImage::ImageDimension; j++)
        {
          direction[j][i] = -direction[j][i];
        }
      }
      spacing[i] = -spacing[i];
    }
  }
  input->SetDirection(direction);
  input->SetSpacing(spacing);
}
}
}


namespace otb
{
/** \class Image
 * \brief Creation of an "otb" image which contains metadata.
 *
 *
 * \ingroup OTBImageBase
 */

template <class TPixel, unsigned int VImageDimension = 2>
class OTBImageBase_EXPORT_TEMPLATE Image
  : public itk::Image<TPixel, VImageDimension>
  , public ImageCommons
{
public:
  /** Standard class typedefs. */
  using Self = Image;
  using Superclass = itk::Image<TPixel, VImageDimension>;
  using Pointer = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;
  using ConstWeakPointer = itk::WeakPointer<const Self>;

  using VectorType = ImageMetadataInterfaceBase::VectorType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(Image, itk::Image);

  /** Pixel typedef support. Used to declare pixel type in filters
   * or other operations. */
  typedef typename Superclass::PixelType PixelType;

  /** Typedef alias for PixelType */
  typedef typename Superclass::ValueType ValueType;

  /** Internal Pixel representation. Used to maintain a uniform API
   * with Image Adaptors and allow keeping a particular internal
   * representation of data while showing a different external
   * representation. */
  typedef typename Superclass::InternalPixelType InternalPixelType;

  typedef typename Superclass::IOPixelType IOPixelType;

  /** Accessor type that convert data between internal and external
   *  representations.  */
  // typedef itk::DefaultPixelAccessor< PixelType > AccessorType;
  // typedef itk::DefaultPixelAccessorFunctor< Self > AccessorFunctorType;

  /** Tyepdef for the functor used to access a neighborhood of pixel pointers.*/
  typedef itk::NeighborhoodAccessorFunctor<Self> NeighborhoodAccessorFunctorType;

  /** Dimension of the image.  This constant is used by functions that are
   * templated over image type (as opposed to being templated over pixel type
   * and dimension) when they need compile time access to the dimension of
   * the image. */
  itkStaticConstMacro(ImageDimension, unsigned int, VImageDimension);

  /** Container used to store pixels in the image. */
  typedef typename Superclass::PixelContainer PixelContainer;

  /** Index typedef support. An index is used to access pixel values. */
  typedef typename Superclass::IndexType IndexType;

  /** Offset typedef support. An offset is used to access pixel values. */
  typedef typename Superclass::OffsetType OffsetType;

  /** Size typedef support. A size is used to define region bounds. */
  typedef typename Superclass::SizeType SizeType;

  /** Direction typedef support. A matrix of direction cosines. */
  typedef typename Superclass::DirectionType DirectionType;

  /** Region typedef support. A region is used to specify a subset of an image. */
  typedef typename Superclass::RegionType RegionType;

  /** Spacing typedef support.  Spacing holds the size of a pixel.  The
   * spacing is the geometric distance between image samples. */
  typedef typename Superclass::SpacingType SpacingType;

  /** Origin typedef support.  The origin is the geometric coordinates
   * of the index (0, 0). */
  typedef typename Superclass::PointType PointType;

  /** A pointer to the pixel container. */
  typedef typename PixelContainer::Pointer      PixelContainerPointer;
  typedef typename PixelContainer::ConstPointer PixelContainerConstPointer;

  /** Offset typedef (relative position between indices) */
  typedef typename Superclass::OffsetValueType OffsetValueType;

  /** Return the Pixel Accessor object */
  //   AccessorType GetPixelAccessor( void )
  //   {
  //     return AccessorType();
  //   }

  //   /** Return the Pixel Accessor object */
  //   const AccessorType GetPixelAccessor( void ) const
  //   {
  //     return AccessorType();
  //   }

  /** Return the NeighborhoodAccessor functor */
  NeighborhoodAccessorFunctorType GetNeighborhoodAccessor()
  {
    return NeighborhoodAccessorFunctorType();
  }

  /** Return the NeighborhoodAccessor functor */
  const NeighborhoodAccessorFunctorType GetNeighborhoodAccessor() const
  {
    return NeighborhoodAccessorFunctorType();
  }


  /** Get the six coefficients of affine geoTtransform. */
  virtual VectorType GetGeoTransform(void) const;

  /** Get image corners. */
  // TODO: GenericRSTransform should be instanciated to translate from physical
  // space to EPSG:4328 ?
  VectorType GetUpperLeftCorner(void) const;
  VectorType GetUpperRightCorner(void) const;
  VectorType GetLowerLeftCorner(void) const;
  VectorType GetLowerRightCorner(void) const;

  /** Get signed spacing */
  SpacingType GetSignedSpacing() const;

  // SpacingType GetSpacing() const
  //   {
  //     PixelType a;
  //     a.toto();
  //     SpacingType t = this->GetSignedSpacing();
  //     return t ;
  //   };

  /** Set signed spacing */
  virtual void SetSignedSpacing(SpacingType spacing);
  virtual void SetSignedSpacing(double spacing[VImageDimension]);

  void PrintSelf(std::ostream& os, itk::Indent indent) const override;

  /// Copy metadata from a DataObject
  void CopyInformation(const itk::DataObject*) override;

protected:
  Image();
  ~Image() override
  {
  }

private:
  Image(const Self&) = delete;
  void operator=(const Self&) = delete;
};

} // end namespace otb

#ifndef OTB_MANUAL_INSTANTIATION
#include "otbImage.hxx"
#endif

#include <complex>

namespace otb
{

// Prevent implicit instantiation of common types to improve build performance
// Explicit instantiations are provided in the .cxx
extern template class OTBImageBase_EXPORT_TEMPLATE Image<unsigned int, 2>;
extern template class OTBImageBase_EXPORT_TEMPLATE Image<int, 2>;
extern template class OTBImageBase_EXPORT_TEMPLATE Image<unsigned char, 2>;
extern template class OTBImageBase_EXPORT_TEMPLATE Image<char, 2>;
extern template class OTBImageBase_EXPORT_TEMPLATE Image<unsigned short, 2>;
extern template class OTBImageBase_EXPORT_TEMPLATE Image<short, 2>;
extern template class OTBImageBase_EXPORT_TEMPLATE Image<float, 2>;
extern template class OTBImageBase_EXPORT_TEMPLATE Image<double, 2>;
extern template class OTBImageBase_EXPORT_TEMPLATE Image<std::complex<int>, 2>;
extern template class OTBImageBase_EXPORT_TEMPLATE Image<std::complex<short>, 2>;
extern template class OTBImageBase_EXPORT_TEMPLATE Image<std::complex<float>, 2>;
extern template class OTBImageBase_EXPORT_TEMPLATE Image<std::complex<double>, 2>;
}

#endif
