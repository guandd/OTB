/*
 * Copyright (C) 2005-2017 Centre National d'Etudes Spatiales (CNES)
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

#ifndef otbFunctorImageFilter_h
#define otbFunctorImageFilter_h

#include "itkImageToImageFilter.h"
#include "otbVariadicInputsImageFilter.h"
#include "otbImage.h"
#include "otbVectorImage.h"
#include "itkConstNeighborhoodIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkProcessObject.h"
#include <type_traits>
#include "otbFunctionTraits.h"
#include "itkDefaultConvertPixelTraits.h"

namespace otb
{

namespace functor_filter_details
{
// Variadic creation of iterator tuple
template <class T> auto MakeIterator(itk::SmartPointer<T> img)
{
  itk::ImageRegionConstIterator<T> it(img,img->GetLargestPossibleRegion());
  return it;
}

template<class... T>  auto MakeIterators(itk::SmartPointer<T>... args)
  {
    return std::make_tuple(MakeIterator(args)...);
  }

// Variadic call of operator from iterator tuple
template <class Tuple, class Oper, size_t...Is> auto CallOperatorImpl(Tuple& t, const Oper & oper,std::index_sequence<Is...>)
{
  return oper(std::get<Is>(t).Get()...);
}

template <class Oper, typename ... Args> auto CallOperator(const Oper& oper, std::tuple<Args...> & t)
{
  return CallOperatorImpl(t,oper,std::make_index_sequence<sizeof...(Args)>{});
}

// Variadic move of iterators
template<class Tuple,size_t...Is> auto MoveIteratorsImpl(Tuple & t, std::index_sequence<Is...>)
{
  return std::make_tuple(++(std::get<Is>(t) )...);
}

template<typename ... Args> void MoveIterators(std::tuple<Args...> & t)
{
  MoveIteratorsImpl(t,std::make_index_sequence<sizeof...(Args)>{});
}

} // end namespace functor_filter_details

template<typename T>
using FTraits = typename FunctionTraits::function_traits<T>;

template<typename T>
using FResultType = typename FTraits<T>::result_type;

template<typename T, size_t i>
using ArgType = typename FTraits<T>::template arg<i>::type;

template <class T> struct IsNeighborhood
{
  using ValueType = std::false_type;
  static constexpr bool value = false;
  using PixelType = T;
};

  template <class T> struct IsNeighborhood<itk::Neighborhood<T>>
{
  using ValueType = std::true_type;
  static constexpr bool value = true;
  using PixelType = T;
};

template<typename T>
struct TInputImage
{
  using ArgumentType = T; //ArgType<T,N>;
  using PixelType = typename IsNeighborhood<typename std::remove_cv<typename std::remove_reference<ArgumentType>::type >::type>::PixelType;
  using ScalarType = typename itk::DefaultConvertPixelTraits<PixelType>::ComponentType;
  using ImageType = typename std::conditional<std::is_scalar<PixelType>::value,
                                     otb::Image< ScalarType >,
                                     otb::VectorImage< ScalarType > >::type;
};


template<typename T>
struct TOutputImage
{
  using ResultType = T; //FResultType<T>;
  using ScalarType = typename itk::DefaultConvertPixelTraits<ResultType>::ComponentType;
  using ImageType =  typename std::conditional<std::is_scalar<ResultType>::value,
                                     typename otb::Image<ScalarType>,
                                     typename otb::VectorImage<ScalarType> >::type;
};



template <typename T> struct FunctorFilterSuperclassHelper : public FunctorFilterSuperclassHelper<decltype(&T::operator())> {};


template <typename R, typename... T> struct FunctorFilterSuperclassHelper<R(*)(T...)>
{
  using OutputImageType = typename TOutputImage<R>::ImageType;
  using FilterType = VariadicInputsImageFilter<OutputImageType,typename TInputImage<T>::ImageType...>;

};

template <typename C, typename R, typename... T> struct FunctorFilterSuperclassHelper<R(C::*)(T...) const>
{
  using OutputImageType = typename TOutputImage<R>::ImageType;
  using FilterType = VariadicInputsImageFilter<OutputImageType,typename TInputImage<T>::ImageType...>;
};

template <typename C, typename R, typename... T> struct FunctorFilterSuperclassHelper<R(C::*)(T...)>
{
  using OutputImageType = typename TOutputImage<R>::ImageType;
  using FilterType = VariadicInputsImageFilter<OutputImageType,typename TInputImage<T>::ImageType...>;
};


/** \class FunctorImageFilter
 * \brief Implements 
 *
 * This class is 
 *
 * \ingroup IntensityImageFilters   Multithreaded
 *
 * \ingroup OTBImageManipulation
 */
template <class TFunction>
    class ITK_EXPORT FunctorImageFilter
  : public FunctorFilterSuperclassHelper<TFunction>::FilterType
{

public:
  using Self = FunctorImageFilter;
  using FunctorType = TFunction;
  using Pointer = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;

  // using InputImageType = typename TInputImage<TFunction,0>::ImageType;
  // using InputImagePointer = typename InputImageType::ConstPointer;
  // using InputImageRegionType = typename InputImageType::RegionType;
  // using InputImagePixelType = typename InputImageType::PixelType;
  // using InputImageSizeType = typename InputImageType::SizeType;
  // using InputImageIndexType = typename InputImageType::IndexType;

  // using OutputImageType = typename TOutputImage<TFunction>::ImageType;
  // using OutputImagePointer = typename OutputImageType::Pointer;
  // using OutputImageRegionType = typename OutputImageType::RegionType;
  // using OutputImagePixelType = typename OutputImageType::PixelType;
  using Superclass = typename FunctorFilterSuperclassHelper<TFunction>::FilterType;
  using OutputImageType = typename Superclass::OutputImageType;
  using OutputImageRegionType = typename OutputImageType::RegionType;
  
  using ProcessObjectType = itk::ProcessObject;

 /** Method for creation by passing the filter type. */


  static Pointer New(const FunctorType& f) ;

/** Run-time type information (and related methods). */
  itkTypeMacro(FunctorImageFilter, ImageToImageFilter);

 
/** Get the functor object.  The functor is returned by reference.
   * (Functors do not have to derive from itk::LightObject, so they do
   * not necessarily have a reference count. So we cannot return a
   * SmartPointer.) */
  FunctorType& GetFunctor()
  {
    this->Modified();
    return m_Functor;
  }

/** Get the functor object.  The functor is returned by reference.
   * (Functors do not have to derive from itk::LightObject, so they do
   * not necessarily have a reference count. So we cannot return a
   * SmartPointer.) */
  const FunctorType& GetFunctor() const
  {
    return m_Functor;
  }

/** Set the functor object.  This replaces the current Functor with a
   * copy of the specified Functor. This allows the user to specify a
   * functor that has ivars set differently than the default functor.
   * This method requires an operator!=() be defined on the functor
   * (or the compiler's default implementation of operator!=() being
   * appropriate). */
void SetFunctor(const FunctorType& functor)
{
  m_Functor = functor;
  this->Modified();
}

private:
FunctorImageFilter();
FunctorImageFilter(const FunctorType& f) : m_Functor(f) {};
FunctorImageFilter(const Self &) ;
void operator =(const Self&) ;
~FunctorImageFilter() {}

protected:

/** FunctorImageFilter can be implemented as a multithreaded filter.
   * Therefore, this implementation provides a ThreadedGenerateData() routine
   * which is called for each processing thread. The output image data is
   * allocated automatically by the superclass prior to calling
   * ThreadedGenerateData().  ThreadedGenerateData can only write to the
   * portion of the output image specified by the parameter
   * "outputRegionForThread"
   *
   * \sa ImageToImageFilter::ThreadedGenerateData(),
   *     ImageToImageFilter::GenerateData()  */
    virtual void ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, itk::ThreadIdType threadId) override;

  /**
   * Pad the input requested region by radius
   */
  virtual void GenerateInputRequestedRegion(void) override;


  FunctorType m_Functor;
};

}// namespace otb

#ifndef OTB_MANUAL_INSTANTIATION
#include "otbFunctorImageFilter.hxx"
#endif

#endif
