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

#ifndef otbImageFileReader_hxx
#define otbImageFileReader_hxx

#include "otbImageFileReader.h"
#include "otbConfigure.h"

#include "otbSystem.h"
#include <itksys/SystemTools.hxx>
#include <fstream>
#include <string>

#include "itkImageIOFactory.h"
#include "itkPixelTraits.h"
#include "itkVectorImage.h"
#include "itkMetaDataObject.h"

#include "otbConvertPixelBuffer.h"
#include "otbImageIOFactory.h"
#include "otbMetaDataKey.h"
#include "otbImageMetadata.h"
#include "otbImageMetadataInterfaceFactory.h"
#include "otbImageCommons.h"
#include "otbGeomMetadataSupplier.h"

#include "otbMacro.h"


namespace otb
{

static const char   DerivedSubdatasetPrefix[]     = "DERIVED_SUBDATASET:";
static const size_t DerivedSubdatasetPrefixLength = sizeof(DerivedSubdatasetPrefix);

template <class T>
bool PixelIsComplex(const std::complex<T>& /*dummy*/)
{
  return true;
}
template <class T>
bool PixelIsComplex(const T& /*dummy*/)
{
  return false;
}

template <class TOutputImage, class ConvertPixelTraits>
ImageFileReader<TOutputImage, ConvertPixelTraits>::ImageFileReader()
  : m_ImageIO(),
    m_UserSpecifiedImageIO(false),
    m_FileName(""),
    m_UseStreaming(true),
    m_ActualIORegion(),
    m_FilenameHelper(FNameHelperType::New()),
    m_AdditionalNumber(0),
    m_IOComponents(0)
{
}

template <class TOutputImage, class ConvertPixelTraits>
ImageFileReader<TOutputImage, ConvertPixelTraits>::~ImageFileReader()
{
}

template <class TOutputImage, class ConvertPixelTraits>
void ImageFileReader<TOutputImage, ConvertPixelTraits>::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  if (this->m_ImageIO)
  {
    os << indent << "ImageIO: \n";
    this->m_ImageIO->Print(os, indent.GetNextIndent());
  }
  else
  {
    os << indent << "ImageIO: (null)"
       << "\n";
  }

  os << indent << "UserSpecifiedImageIO flag: " << this->m_UserSpecifiedImageIO << "\n";
  os << indent << "m_FileName: " << this->m_FileName << "\n";
  os << indent << "m_UseStreaming flag: " << this->m_UseStreaming << "\n";
  os << indent << "m_ActualIORegion: " << this->m_ActualIORegion << "\n";
  os << indent << "m_AdditionalNumber: " << this->m_AdditionalNumber << "\n";
}

template <class TOutputImage, class ConvertPixelTraits>
void ImageFileReader<TOutputImage, ConvertPixelTraits>::SetImageIO(otb::ImageIOBase* imageIO)
{
  if (this->m_ImageIO != imageIO)
  {
    this->m_ImageIO = imageIO;
    this->Modified();
  }
  m_UserSpecifiedImageIO = true;
}

template <class TOutputImage, class ConvertPixelTraits>
void ImageFileReader<TOutputImage, ConvertPixelTraits>::GenerateData()
{

  typename TOutputImage::Pointer output = this->GetOutput();

  // allocate the output buffer
  output->SetBufferedRegion(output->GetRequestedRegion());
  output->Allocate();

  // Raise an exception if the file could not be opened
  // i.e. if this->m_ImageIO is Null
  this->TestValidImageIO();

  // Tell the ImageIO to read the file
  OutputImagePixelType* buffer = output->GetPixelContainer()->GetBufferPointer();
  this->m_ImageIO->SetFileName(this->m_FileName);

  itk::ImageIORegion ioRegion(TOutputImage::ImageDimension);

  itk::ImageIORegion::SizeType  ioSize  = ioRegion.GetSize();
  itk::ImageIORegion::IndexType ioStart = ioRegion.GetIndex();

  /* Init IORegion with size or streaming size */
  SizeType dimSize;
  for (unsigned int i = 0; i < TOutputImage::ImageDimension; ++i)
  {
    if (i < this->m_ImageIO->GetNumberOfDimensions())
    {
      if (!this->m_ImageIO->CanStreamRead())
        dimSize[i] = this->m_ImageIO->GetDimensions(i);
      else
        dimSize[i] = output->GetRequestedRegion().GetSize()[i];
    }
    else
    {
      // Number of dimensions in the output is more than number of dimensions
      // in the ImageIO object (the file).  Use default values for the size,
      // spacing, and origin for the final (degenerate) dimensions.
      dimSize[i] = 1;
    }
  }

  for (unsigned int i = 0; i < dimSize.GetSizeDimension(); ++i)
  {
    ioSize[i] = dimSize[i];
  }

  IndexType start;
  if (!this->m_ImageIO->CanStreamRead())
    start.Fill(0);
  else
    start = output->GetRequestedRegion().GetIndex();
  for (unsigned int i = 0; i < start.GetIndexDimension(); ++i)
  {
    ioStart[i] = start[i];
  }

  ioRegion.SetSize(ioSize);
  ioRegion.SetIndex(ioStart);

  this->m_ImageIO->SetIORegion(ioRegion);

  typedef otb::DefaultConvertPixelTraits<typename TOutputImage::IOPixelType> ConvertIOPixelTraits;
  typedef otb::DefaultConvertPixelTraits<typename TOutputImage::PixelType>   ConvertOutputPixelTraits;

  if (this->m_ImageIO->GetComponentTypeInfo() == typeid(typename ConvertOutputPixelTraits::ComponentType) &&
      (this->m_ImageIO->GetNumberOfComponents() == ConvertIOPixelTraits::GetNumberOfComponents()) && !m_FilenameHelper->BandRangeIsSet())
  {
    // Have the ImageIO read directly into the allocated buffer
    this->m_ImageIO->Read(buffer);
    return;
  }
  else // a type conversion is necessary
  {
    // note: char is used here because the buffer is read in bytes
    // regardless of the actual type of the pixels.
    ImageRegionType region = output->GetBufferedRegion();

    // Adapt the image size with the region and take into account a potential
    // remapping of the components. m_BandList is empty if no band range is set
    std::streamoff nbBytes = (this->m_ImageIO->GetComponentSize() * std::max(this->m_ImageIO->GetNumberOfComponents(), (unsigned int)m_BandList.size())) *
                             static_cast<std::streamoff>(region.GetNumberOfPixels());

    char* loadBuffer = new char[nbBytes];

    this->m_ImageIO->Read(loadBuffer);

    if (m_FilenameHelper->BandRangeIsSet())
      this->m_ImageIO->DoMapBuffer(loadBuffer, region.GetNumberOfPixels(), this->m_BandList);

    this->DoConvertBuffer(loadBuffer, region.GetNumberOfPixels());

    delete[] loadBuffer;
  }
}

template <class TOutputImage, class ConvertPixelTraits>
void ImageFileReader<TOutputImage, ConvertPixelTraits>::EnlargeOutputRequestedRegion(itk::DataObject* output)
{
  typename TOutputImage::Pointer out = dynamic_cast<TOutputImage*>(output);

  // If the ImageIO object cannot stream, then set the RequestedRegion to the
  // LargestPossibleRegion
  if (!this->m_ImageIO->CanStreamRead())
  {
    if (out)
    {
      out->SetRequestedRegion(out->GetLargestPossibleRegion());
    }
    else
    {
      throw otb::ImageFileReaderException(__FILE__, __LINE__, "Invalid output object type");
    }
  }
}

template <class TOutputImage, class ConvertPixelTraits>
void ImageFileReader<TOutputImage, ConvertPixelTraits>::GenerateOutputInformation(void)
{
  typename TOutputImage::Pointer output = this->GetOutput();

  // Check to see if we can read the file given the name or prefix
  if (this->m_FileName == "")
  {
    throw otb::ImageFileReaderException(__FILE__, __LINE__, "Filename must be specified.");
  }

  // Find real image file name
  // !!!!  Update FileName
  std::string lFileName;
  bool        found = GetGdalReadImageFileName(this->m_FileName, lFileName);
  if (found)
  {
    // Update FileName
    this->m_FileName = lFileName;
  }

  if (this->m_UserSpecifiedImageIO == false) // try creating via factory
  {
    this->m_ImageIO = ImageIOFactory::CreateImageIO(this->m_FileName.c_str(), otb::ImageIOFactory::ReadMode);
  }

  // Raise an exception if the file could not be opened
  // i.e. if this->m_ImageIO is Null
  this->TestValidImageIO();

  // Get the ImageIO MetaData Dictionary
  itk::MetaDataDictionary& dict = this->m_ImageIO->GetMetaDataDictionary();

  // Hint the IO whether the OTB image type takes complex pixels
  // this will determine the strategy to fill up a vector image
  OutputImagePixelType dummy;
  bool                 lVectorImage = false;
  if (strcmp(output->GetNameOfClass(), "VectorImage") == 0)
    lVectorImage = true;

  this->m_ImageIO->SetOutputImagePixelType(PixelIsComplex(dummy), lVectorImage);

  // Pass the dataset number (used for hdf files for example)
  if (m_FilenameHelper->SubDatasetIndexIsSet())
  {
    itk::EncapsulateMetaData<unsigned int>(dict, MetaDataKey::SubDatasetIndex, m_FilenameHelper->GetSubDatasetIndex());
  }
  else
  {
    itk::EncapsulateMetaData<unsigned int>(dict, MetaDataKey::SubDatasetIndex, m_AdditionalNumber);
  }


  if (m_FilenameHelper->ResolutionFactorIsSet())
  {
    itk::EncapsulateMetaData<unsigned int>(dict, MetaDataKey::ResolutionFactor, m_FilenameHelper->GetResolutionFactor());
  }
  else
  {
    itk::EncapsulateMetaData<unsigned int>(dict, MetaDataKey::ResolutionFactor, m_AdditionalNumber);
  }

  // Got to allocate space for the image. Determine the characteristics of
  // the image.
  //
  this->m_ImageIO->SetFileName(this->m_FileName);
  this->m_ImageIO->ReadImageInformation();
  // Initialize the number of component per pixel
  // THOMAS: This is not in ITK!
  // output->SetNumberOfComponentsPerPixel(this->m_ImageIO->GetNumberOfComponents());

  SizeType                             dimSize;
  double                               spacing[TOutputImage::ImageDimension];
  double                               origin[TOutputImage::ImageDimension];
  typename TOutputImage::DirectionType direction;
  std::vector<double>                  axis;
  int                                  spacing_sign(0);

  for (unsigned int i = 0; i < TOutputImage::ImageDimension; ++i)
  {
    if (i < this->m_ImageIO->GetNumberOfDimensions())
    {
      dimSize[i] = this->m_ImageIO->GetDimensions(i);
      if (this->m_ImageIO->GetSpacing(i) < 0)
        spacing_sign = -1;
      else
        spacing_sign = 1;
      spacing[i]     = spacing_sign * this->m_ImageIO->GetSpacing(i);
      origin[i]      = this->m_ImageIO->GetOrigin(i);
      // Please note: direction cosines are stored as columns of the
      // direction matrix
      axis = this->m_ImageIO->GetDirection(i);
      for (unsigned j = 0; j < TOutputImage::ImageDimension; ++j)
      {
        if (j < this->m_ImageIO->GetNumberOfDimensions())
        {
          direction[j][i] = spacing_sign * axis[j];
        }
        else
        {
          direction[j][i] = 0.0;
        }
      }
    }
    else
    {
      // Number of dimensions in the output is more than number of dimensions
      // in the ImageIO object (the file).  Use default values for the size,
      // spacing, origin and direction for the final (degenerate) dimensions.
      dimSize[i] = 1;
      spacing[i] = 1.0;
      origin[i]  = 0.5;
      for (unsigned j = 0; j < TOutputImage::ImageDimension; ++j)
      {
        if (i == j)
        {
          direction[j][i] = 1.0;
        }
        else
        {
          direction[j][i] = 0.0;
        }
      }
    }
  }

  if (m_FilenameHelper->GetSkipCarto())
  {
    for (unsigned int i = 0; i < TOutputImage::ImageDimension; ++i)
    {
      if (m_FilenameHelper->GetResolutionFactor() != 0)
      {
        spacing[i] = 1.0 * std::pow((double)2, (double)m_FilenameHelper->GetResolutionFactor());
      }
      else
      {
        spacing[i] = 1.0;
      }
      origin[i] = 0.5 * spacing[i];
      for (unsigned j = 0; j < TOutputImage::ImageDimension; ++j)
      {
        if (i == j)
        {
          direction[j][i] = 1.0;
        }
        else
        {
          direction[j][i] = 0.0;
        }
      }
    }
  }

  output->SetOrigin(origin);       // Set the image origin
  output->SetDirection(direction); // Set the image direction cosines
  output->SetSpacing(spacing);     // Set the image spacing

  // detect Image supporting new ImageMetadata
  ImageCommons* img_common = dynamic_cast<ImageCommons*>(this->GetOutput());
  
  // Get ImageMetadata from ImageIO
  ImageMetadata imd = m_ImageIO->GetImageMetadata();

  // Metadata Framework
  std::string DerivatedFileName = GetDerivedDatasetSourceFileName(m_FileName);
  std::string extension                 = itksys::SystemTools::GetFilenameLastExtension(DerivatedFileName);
  std::string attachedGeom              = DerivatedFileName.substr(0, DerivatedFileName.size() - extension.size()) + std::string(".geom");
  // Case 1: external geom supplied through extended filename
  if (!m_FilenameHelper->GetSkipGeom() && m_FilenameHelper->ExtGEOMFileNameIsSet())
  {
    GeomMetadataSupplier geomSupplier(m_FilenameHelper->GetExtGEOMFileName(), m_FileName);
    ImageMetadataInterfaceFactory::CreateIMI(imd, geomSupplier);
    geomSupplier.FetchRPC(imd);
    geomSupplier.FetchGCP(imd);
    otbLogMacro(Info, << "Loading metadata from external geom file " << m_FilenameHelper->GetExtGEOMFileName());
  }
  // Case 2: attached geom (if present)
  else if (!m_FilenameHelper->GetSkipGeom() && itksys::SystemTools::FileExists(attachedGeom))
  {
    GeomMetadataSupplier geomSupplier(attachedGeom, m_FileName);
    ImageMetadataInterfaceFactory::CreateIMI(imd, geomSupplier);
    geomSupplier.FetchRPC(imd);
    geomSupplier.FetchGCP(imd);
    otbLogMacro(Info, << "Loading metadata from attached geom file " << attachedGeom);
  }
  // Case 3: tags in file
  else
  {
    auto gdalMetadataSupplierPointer = dynamic_cast<MetadataSupplierInterface*>(m_ImageIO.GetPointer());
    if (gdalMetadataSupplierPointer)
    {
      ImageMetadataInterfaceFactory::CreateIMI(imd, *gdalMetadataSupplierPointer);
      otbLogMacro(Info, << "Loading metadata from official product");
    }
  }

  // If Skip ProjectionRef is activated, remove ProjRef from dict
  if (m_FilenameHelper->GetSkipCarto())
  {
    itk::EncapsulateMetaData<std::string>(dict, MetaDataKey::ProjectionRefKey, "");
    imd.RemoveProjectedGeometry();
  }

  // Copy MetaDataDictionary from instantiated reader to output image.
  // TODO: disable when Ossim removed
  if (!m_FilenameHelper->GetSkipGeom())
  {
    output->SetMetaDataDictionary(this->m_ImageIO->GetMetaDataDictionary());
    this->SetMetaDataDictionary(this->m_ImageIO->GetMetaDataDictionary());
  }
  else
  {
    itk::MetaDataDictionary dictLight;
    std::string             projRef;
    itk::ExposeMetaData(dict, MetaDataKey::ProjectionRefKey, projRef);
    itk::EncapsulateMetaData<std::string>(dictLight, MetaDataKey::ProjectionRefKey, projRef);
    output->SetMetaDataDictionary(dictLight);
    this->SetMetaDataDictionary(dictLight);
  }
  
  IndexType start;
  start.Fill(0);

  ImageRegionType region;
  region.SetSize(dimSize);
  region.SetIndex(start);

  // detect number of output components
  m_IOComponents = this->m_ImageIO->GetNumberOfComponents();
  m_BandList.clear();
  if (m_FilenameHelper->BandRangeIsSet())
  {
    bool ret = m_FilenameHelper->ResolveBandRange(m_FilenameHelper->GetBandRange(), m_IOComponents, m_BandList);
    if (ret == false || m_BandList.size() == 0)
    {
      // invalid range
      itkGenericExceptionMacro("The given band range is either empty or invalid for a " << m_IOComponents << " bands input image!");
    }
    // ImageIO returned the metadata from all bands of the input raster. It needs to be adapted to the layout of m_BandList
    ImageMetadata::ImageMetadataBandsType bandRangeMetadata;
    for (auto elem: m_BandList)
    {
      bandRangeMetadata.push_back(imd.Bands[elem]);
    }
    imd.Bands = bandRangeMetadata;
    m_IOComponents = m_BandList.size();
  }

  // Delete band metadata if the Conversion policy changed the number of bands, in the case of 
  // grayscale to RGB for example. Because we cannot know how the metadata should be mapped.
  // TODO: define proper behavior in this case.
  using ConvertIOPixelTraits = otb::DefaultConvertPixelTraits<typename TOutputImage::IOPixelType>;
  if (strcmp(output->GetNameOfClass(), "Image") == 0 && !(this->m_ImageIO->GetNumberOfComponents() == ConvertIOPixelTraits::GetNumberOfComponents()))
  {
    imd.Bands = ImageMetadata::ImageMetadataBandsType (ConvertIOPixelTraits::GetNumberOfComponents());
  }

  // THOMAS : ajout
  // If a VectorImage, this requires us to set the
  // VectorLength before allocate
  if (strcmp(output->GetNameOfClass(), "VectorImage") == 0)
  {
    typedef typename TOutputImage::AccessorFunctorType AccessorFunctorType;
    AccessorFunctorType::SetVectorLength(output, m_IOComponents);
  }

  if (img_common != nullptr)
    {
    img_common->SetImageMetadata(imd);
    }

  output->SetLargestPossibleRegion(region);
}

template <class TOutputImage, class ConvertPixelTraits>
std::string ImageFileReader<TOutputImage, ConvertPixelTraits>::GetDerivedDatasetSourceFileName(const std::string& filename)
{

  const size_t dsds_pos = filename.find(otb::DerivedSubdatasetPrefix);

  if (dsds_pos != std::string::npos)
  {
    // Derived subdataset from gdal
    const size_t alg_pos = filename.find(":", dsds_pos + otb::DerivedSubdatasetPrefixLength);
    if (alg_pos != std::string::npos)
    {
      std::string sourceFilename = filename.substr(alg_pos + 1, filename.size() - alg_pos);
      return sourceFilename;
    }
  }
  return filename;
}

template <class TOutputImage, class ConvertPixelTraits>
void ImageFileReader<TOutputImage, ConvertPixelTraits>::TestValidImageIO()
{
  if (this->m_ImageIO.IsNull())
  {
    std::string fileToCheck = GetDerivedDatasetSourceFileName(m_FileName);

    // Test if the file exists.
    if (!itksys::SystemTools::FileExists(fileToCheck))
    {
      throw otb::ImageFileReaderException(__FILE__, __LINE__, std::string("Cannot open image ") + fileToCheck + std::string(". The file does not exist."),
                                          fileToCheck);
    }
    else
    {
      throw otb::ImageFileReaderException(__FILE__, __LINE__, std::string("Cannot open image ") + this->m_FileName +
                                                                  std::string(". Probably unsupported format or incorrect filename extension."),
                                          this->m_FileName);
    }
  }
}

template <class TOutputImage, class ConvertPixelTraits>
bool ImageFileReader<TOutputImage, ConvertPixelTraits>::GetGdalReadImageFileName(const std::string& filename, std::string& GdalFileName)
{
  std::vector<std::string> listFileSearch;
  listFileSearch.push_back("DAT_01.001");
  listFileSearch.push_back("dat_01.001"); // RADARSAT or SAR_ERS2
  listFileSearch.push_back("IMAGERY.TIF");
  listFileSearch.push_back("imagery.tif"); // For format SPOT5TIF
  // Not recognized as a supported file format by GDAL.
  //        listFileSearch.push_back("IMAGERY.BIL"); listFileSearch.push_back("imagery.bil"); //For format SPOT5BIL
  listFileSearch.push_back("IMAG_01.DAT");
  listFileSearch.push_back("imag_01.dat"); // For format SPOT4

  std::string str_FileName;
  bool        fic_trouve(false);

  // If it's a directory, look at the content to see if it's RADARSAT, ERS
  std::vector<std::string> listFileFind;
  listFileFind = System::Readdir(filename);
  if (listFileFind.empty() == false)
  {
    unsigned int cpt(0);
    while ((cpt < listFileFind.size()) && (fic_trouve == false))
    {
      str_FileName = std::string(listFileFind[cpt]);
      for (unsigned int i = 0; i < listFileSearch.size(); ++i)
      {
        if (str_FileName.compare(listFileSearch[i]) == 0)
        {
          GdalFileName = std::string(filename) + str_FileName; // listFileSearch[i];
          fic_trouve   = true;
        }
      }
      ++cpt;
    }
  }
  else
  {
    std::string strFileName(filename);

    std::string extension = itksys::SystemTools::GetFilenameLastExtension(strFileName);
    if ((extension == ".HDR") || (extension == ".hdr"))
    {
      GdalFileName = System::GetRootName(strFileName);
    }
    else
    {
      // Else, the filename is the name of the file to open
      GdalFileName = std::string(filename);
    }
    fic_trouve = true;
  }

  return (fic_trouve);
}

template <class TOutputImage, class ConvertPixelTraits>
void ImageFileReader<TOutputImage, ConvertPixelTraits>::SetFileName(const std::string& extendedFileName)
{
  const std::string skip_geom_key = "skipgeom";
  const std::string geom_key      = "geom";

  // First, see if the simple filename has changed
  typename FNameHelperType::Pointer helper = FNameHelperType::New();

  helper->SetExtendedFileName(extendedFileName);
  std::string simpleFileName = helper->GetSimpleFileName();

  if (simpleFileName == this->m_FileName)
  {
    // Then, see if the option map changed
    const typename ExtendedFilenameHelper::OptionMapType& newMap = helper->GetOptionMap();
    const typename ExtendedFilenameHelper::OptionMapType& oldMap = m_FilenameHelper->GetOptionMap();

    // Both maps are not completely the same
    if (oldMap.size() != newMap.size() || !std::equal(oldMap.begin(), oldMap.end(), newMap.begin()))
    {
      this->Modified();
    }
  }
  else
  {
    this->m_FileName      = simpleFileName;
    this->Modified();
  }

  m_FilenameHelper = helper;
}

template <class TOutputImage, class ConvertPixelTraits>
const char* ImageFileReader<TOutputImage, ConvertPixelTraits>::GetFileName() const
{
  return this->m_FilenameHelper->GetSimpleFileName();
}

template <class TOutputImage, class ConvertPixelTraits>
unsigned int ImageFileReader<TOutputImage, ConvertPixelTraits>::GetOverviewsCount()
{
  this->UpdateOutputInformation();

  return this->m_ImageIO->GetOverviewsCount();
}


template <class TOutputImage, class ConvertPixelTraits>
std::vector<std::string> ImageFileReader<TOutputImage, ConvertPixelTraits>::GetOverviewsInfo()
{
  this->UpdateOutputInformation();

  return this->m_ImageIO->GetOverviewsInfo();
}

template <class TOutputImage, class ConvertPixelTraits>
void ImageFileReader<TOutputImage, ConvertPixelTraits>::DoConvertBuffer(void* inputData, size_t numberOfPixels)
{
  // get the pointer to the destination buffer
  OutputImagePixelType* outputData = this->GetOutput()->GetPixelContainer()->GetBufferPointer();

// TODO:
// Pass down the PixelType (RGB, VECTOR, etc.) so that any vector to
// scalar conversion be type specific. i.e. RGB to scalar would use
// a formula to convert to radiance, VECTOR to scalar would use
// vector magnitude.


// Create a macro as this code is a bit lengthy and repetitive
// if the ImageIO pixel type is typeid(type) then use the ConvertPixelBuffer
// class to convert the data block to TOutputImage's pixel type
// see DefaultConvertPixelTraits and ConvertPixelBuffer

// The first else if block applies only to images of type itk::VectorImage
// VectorImage needs to copy out the buffer differently.. The buffer is of
// type InternalPixelType, but each pixel is really 'k' consecutive pixels.

#define OTB_CONVERT_BUFFER_IF_BLOCK(type)                                                                                                                     \
  else if (m_ImageIO->GetComponentTypeInfo() == typeid(type))                                                                                                 \
  {                                                                                                                                                           \
    if (strcmp(this->GetOutput()->GetNameOfClass(), "VectorImage") == 0)                                                                                      \
    {                                                                                                                                                         \
      ConvertPixelBuffer<type, OutputImagePixelType, ConvertPixelTraits>::ConvertVectorImage(static_cast<type*>(inputData), m_IOComponents, outputData,       \
                                                                                             numberOfPixels);                                                 \
    }                                                                                                                                                         \
    else                                                                                                                                                      \
    {                                                                                                                                                         \
      ConvertPixelBuffer<type, OutputImagePixelType, ConvertPixelTraits>::Convert(static_cast<type*>(inputData), m_IOComponents, outputData, numberOfPixels); \
    }                                                                                                                                                         \
  }
#define OTB_CONVERT_CBUFFER_IF_BLOCK(type)                                                                                                                \
  else if (m_ImageIO->GetComponentTypeInfo() == typeid(type))                                                                                             \
  {                                                                                                                                                       \
    if (strcmp(this->GetOutput()->GetNameOfClass(), "VectorImage") == 0)                                                                                  \
    {                                                                                                                                                     \
      if ((typeid(OutputImagePixelType) == typeid(std::complex<double>)) || (typeid(OutputImagePixelType) == typeid(std::complex<float>)) ||              \
          (typeid(OutputImagePixelType) == typeid(std::complex<int>)) || (typeid(OutputImagePixelType) == typeid(std::complex<short>)))                   \
      {                                                                                                                                                   \
        ConvertPixelBuffer<type::value_type, OutputImagePixelType, ConvertPixelTraits>::ConvertComplexVectorImageToVectorImageComplex(                    \
            static_cast<type*>(inputData), m_IOComponents, outputData, numberOfPixels);                                                                   \
      }                                                                                                                                                   \
      else                                                                                                                                                \
      {                                                                                                                                                   \
        ConvertPixelBuffer<type::value_type, OutputImagePixelType, ConvertPixelTraits>::ConvertComplexVectorImageToVectorImage(                           \
            static_cast<type*>(inputData), m_IOComponents, outputData, numberOfPixels);                                                                   \
      }                                                                                                                                                   \
    }                                                                                                                                                     \
    else                                                                                                                                                  \
    {                                                                                                                                                     \
      ConvertPixelBuffer<type::value_type, OutputImagePixelType, ConvertPixelTraits>::ConvertComplexToGray(static_cast<type*>(inputData), m_IOComponents, \
                                                                                                           outputData, numberOfPixels);                   \
    }                                                                                                                                                     \
  }

  if (0)
  {
  }
  OTB_CONVERT_BUFFER_IF_BLOCK(unsigned char)
  OTB_CONVERT_BUFFER_IF_BLOCK(char)
  OTB_CONVERT_BUFFER_IF_BLOCK(unsigned short)
  OTB_CONVERT_BUFFER_IF_BLOCK(short)
  OTB_CONVERT_BUFFER_IF_BLOCK(unsigned int)
  OTB_CONVERT_BUFFER_IF_BLOCK(int)
  OTB_CONVERT_BUFFER_IF_BLOCK(unsigned long)
  OTB_CONVERT_BUFFER_IF_BLOCK(long)
  OTB_CONVERT_BUFFER_IF_BLOCK(float)
  OTB_CONVERT_BUFFER_IF_BLOCK(double)
  OTB_CONVERT_CBUFFER_IF_BLOCK(std::complex<short>)
  OTB_CONVERT_CBUFFER_IF_BLOCK(std::complex<int>)
  OTB_CONVERT_CBUFFER_IF_BLOCK(std::complex<float>)
  OTB_CONVERT_CBUFFER_IF_BLOCK(std::complex<double>)
  else
  {
    otb::ImageFileReaderException e(__FILE__, __LINE__);
    std::ostringstream            msg;
    msg << "Couldn't convert component type: " << std::endl
        << "    " << ImageIOBase::GetComponentTypeAsString(m_ImageIO->GetComponentType()) << std::endl
        << "to one of: " << std::endl
        << "    " << typeid(unsigned char).name() << std::endl
        << "    " << typeid(char).name() << std::endl
        << "    " << typeid(unsigned short).name() << std::endl
        << "    " << typeid(short).name() << std::endl
        << "    " << typeid(unsigned int).name() << std::endl
        << "    " << typeid(int).name() << std::endl
        << "    " << typeid(unsigned long).name() << std::endl
        << "    " << typeid(long).name() << std::endl
        << "    " << typeid(float).name() << std::endl
        << "    " << typeid(double).name() << std::endl;
    e.SetDescription(msg.str());
    e.SetLocation(ITK_LOCATION);
    throw e;
    return;
  }
#undef OTB_CONVERT_BUFFER_IF_BLOCK
#undef OTB_CONVERT_CBUFFER_IF_BLOCK
}


} // namespace otb

#endif
