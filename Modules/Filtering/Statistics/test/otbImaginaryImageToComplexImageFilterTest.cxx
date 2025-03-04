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


#include "otbImaginaryImageToComplexImageFilter.h"
#include "itkMacro.h"
#include "otbImageFileReader.h"
#include "otbImage.h"

#include "itkComplexToImaginaryImageFilter.h"
#include "otbStreamingCompareImageFilter.h"

int otbImaginaryImageToComplexImageFilterTest(int itkNotUsed(argc), char* argv[])
{
  const char* inputFilename = argv[1];

  typedef double                  PixelType;
  typedef std::complex<PixelType> ComplexPixelType;
  const unsigned int              Dimension = 2;

  typedef otb::Image<PixelType, Dimension>        ImageType;
  typedef otb::Image<ComplexPixelType, Dimension> ComplexImageType;

  typedef otb::ImageFileReader<ImageType> ReaderType;

  typedef otb::ImaginaryImageToComplexImageFilter<ImageType, ComplexImageType> FilterType;

  FilterType::Pointer filter = FilterType::New();

  ReaderType::Pointer reader = ReaderType::New();

  reader->SetFileName(inputFilename);
  reader->UpdateOutputInformation();

  filter->SetInput(reader->GetOutput());
  filter->Update();

  typedef itk::ComplexToImaginaryImageFilter<ComplexImageType, ImageType> ImageFilterType;

  ImageFilterType::Pointer filterImaginaryPart = ImageFilterType::New();
  filterImaginaryPart->SetInput(filter->GetOutput());

  typedef otb::StreamingCompareImageFilter<ImageType> CompareImageFilterType;
  CompareImageFilterType::Pointer                     compareImaginaryImageFilter = CompareImageFilterType::New();

  compareImaginaryImageFilter->SetInput1(filterImaginaryPart->GetOutput());
  compareImaginaryImageFilter->SetInput2(reader->GetOutput());
  compareImaginaryImageFilter->Update();

  if (compareImaginaryImageFilter->GetMSE() > 0.0000001)
  {
    std::cout << " Invalid ImaginaryPart ! MSE: " << compareImaginaryImageFilter->GetMSE() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
