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
#ifndef otbPCAModel_h
#define otbPCAModel_h

#include "otbMachineLearningModelTraits.h"
#include "otbMachineLearningModel.h"

// Quiet a deprecation warning
#define BOOST_BIND_GLOBAL_PLACEHOLDERS

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push

#if (defined (__GNUC__) && (__GNUC__ >= 9)) || (defined (__clang__) && (__clang_major__ >= 10))
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#endif

#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wheader-guard"

#if defined(__apple_build_version__)
/* Need AppleClang >= 9.0.0 to support -Wexpansion-to-defined */
#if __apple_build_version__ >= 9000000
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#endif
#elif __clang_major__ > 3
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#endif

#else
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
#endif
#include "otb_shark.h"
#include <shark/Algorithms/Trainers/PCA.h>
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

namespace otb
{

/** \class PCAModel
 *
 * This class wraps a PCA model implemented by Shark, in a otb::MachineLearningModel
 *
 * \ingroup OTBDimensionalityReductionLearning
 */
template <class TInputValue>
class ITK_EXPORT PCAModel : public MachineLearningModel<itk::VariableLengthVector<TInputValue>, itk::VariableLengthVector<TInputValue>>
{
public:
  typedef PCAModel                                                                                             Self;
  typedef MachineLearningModel<itk::VariableLengthVector<TInputValue>, itk::VariableLengthVector<TInputValue>> Superclass;
  typedef itk::SmartPointer<Self>                                                                              Pointer;
  typedef itk::SmartPointer<const Self>                                                                        ConstPointer;

  typedef typename Superclass::InputValueType       InputValueType;
  typedef typename Superclass::InputSampleType      InputSampleType;
  typedef typename Superclass::InputListSampleType  InputListSampleType;
  typedef typename InputListSampleType::Pointer     ListSamplePointerType;
  typedef typename Superclass::TargetValueType      TargetValueType;
  typedef typename Superclass::TargetSampleType     TargetSampleType;
  typedef typename Superclass::TargetListSampleType TargetListSampleType;

  // Confidence map related typedefs
  typedef typename Superclass::ConfidenceValueType      ConfidenceValueType;
  typedef typename Superclass::ConfidenceSampleType     ConfidenceSampleType;
  typedef typename Superclass::ConfidenceListSampleType ConfidenceListSampleType;

  typedef typename Superclass::ProbaSampleType     ProbaSampleType;
  typedef typename Superclass::ProbaListSampleType ProbaListSampleType;

  itkNewMacro(Self);
  itkTypeMacro(PCAModel, DimensionalityReductionModel);

  itkSetMacro(DoResizeFlag, bool);

  itkSetMacro(WriteEigenvectors, bool);
  itkGetMacro(WriteEigenvectors, bool);

  bool CanReadFile(const std::string& filename) override;
  bool CanWriteFile(const std::string& filename) override;

  void Save(const std::string& filename, const std::string& name = "") override;
  void Load(const std::string& filename, const std::string& name = "") override;

  void Train() override;

protected:
  PCAModel();
  ~PCAModel() override;

  virtual TargetSampleType DoPredict(const InputSampleType& input, ConfidenceValueType* quality = nullptr, ProbaSampleType* proba = nullptr) const override;

  virtual void DoPredictBatch(const InputListSampleType*, const unsigned int& startIndex, const unsigned int& size, TargetListSampleType*,
                              ConfidenceListSampleType* quality = nullptr, ProbaListSampleType* proba = nullptr) const override;

private:
  shark::LinearModel<> m_Encoder;
  shark::LinearModel<> m_Decoder;
  shark::PCA           m_PCA;
  bool                 m_DoResizeFlag;
  bool                 m_WriteEigenvectors;
};
} // end namespace otb


#ifndef OTB_MANUAL_INSTANTIATION
#include "otbPCAModel.hxx"
#endif


#endif
