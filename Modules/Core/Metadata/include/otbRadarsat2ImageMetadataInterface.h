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

#ifndef otbRadarsat2ImageMetadataInterface_h
#define otbRadarsat2ImageMetadataInterface_h

#include "otbSarImageMetadataInterface.h"

namespace otb
{
/** \class Radarsat2ImageMetadataInterface
 *
 * \brief Creation of an "otb" Radarsat2ImageMetadataInterface that gets metadata.
 *
 *
 * \ingroup OTBMetadata
 */

class OTBMetadata_EXPORT Radarsat2ImageMetadataInterface : public SarImageMetadataInterface
{
public:
  typedef Radarsat2ImageMetadataInterface Self;
  typedef SarImageMetadataInterface       Superclass;
  typedef itk::SmartPointer<Self>         Pointer;
  typedef itk::SmartPointer<const Self>   ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(Radarsat2ImageMetadataInterface, SarImageMetadataInterface);

  typedef Superclass::ImageType                ImageType;
  typedef Superclass::MetaDataDictionaryType   MetaDataDictionaryType;
  typedef Superclass::VectorType               VectorType;
  typedef Superclass::VariableLengthVectorType VariableLengthVectorType;
  typedef Superclass::LookupDataPointerType    LookupDataPointerType;
  //  typedef Radarsat2CalibrationLookupData::Pointer          LookupDataPointerType;

  double GetCenterIncidenceAngle(const MetadataSupplierInterface&) const override;

  /*get lookup data for calculating backscatter */
  bool HasCalibrationLookupDataFlag(const MetadataSupplierInterface&) const override;
  bool CreateCalibrationLookupData(SARCalib&, const ImageMetadata&, const MetadataSupplierInterface&, const bool) const override;

  void ParseGdal(ImageMetadata &) override;

  void ParseGeom(ImageMetadata &) override;

  void Parse(ImageMetadata &) override;


protected:
  /* class constructor */
  Radarsat2ImageMetadataInterface();

  /* class destructor */
  ~Radarsat2ImageMetadataInterface() override = default;

private:
  Radarsat2ImageMetadataInterface(const Self&) = delete;
  void operator=(const Self&) = delete;
};


class Radarsat2CalibrationLookupData : public SarCalibrationLookupData
{

public:
  /** Standard typedefs */
  typedef Radarsat2CalibrationLookupData Self;
  typedef SarCalibrationLookupData       Superclass;
  typedef itk::SmartPointer<Self>        Pointer;
  typedef itk::SmartPointer<const Self>  ConstPointer;

  /** Creation through the object factory */
  itkNewMacro(Self);

  /** RTTI */
  itkTypeMacro(Radarsat2CalibrationLookupData, SarCalibrationLookupData);

  typedef itk::IndexValueType IndexValueType;

  typedef std::vector<float> GainListType;


  Radarsat2CalibrationLookupData() : m_Offset(0)
  {
  }

  ~Radarsat2CalibrationLookupData() override
  {
  }

  void InitParameters(short type, int offset, GainListType gains)
  {
    this->SetType(type);
    m_Offset = offset;
    m_Gains  = gains;
  }

  double GetValue(const IndexValueType x, const IndexValueType itkNotUsed(y)) const override
  {
    double lutVal = 1.0;

    const size_t pos = x + m_Offset;
    if (pos < m_Gains.size())
    {
      lutVal = m_Gains[pos];
    }
    else
    {
      // itkExceptionMacro( << "error: (pos < list.size() )" << pos << " < " << list.size())
    }
    return lutVal;
  }

  void PrintSelf(std::ostream& os, itk::Indent indent) const override
  {
    os << indent << " offset:'" << m_Offset << "'" << std::endl;
    os << " referenceNoiseLevel.gain: " << std::endl;
    std::vector<float>::const_iterator it = m_Gains.begin();
    while (it != m_Gains.end())
    {
      os << (*it) << " ";
      ++it;
    }
    os << std::endl;

    Superclass::PrintSelf(os, indent);
  }

private:
  Radarsat2CalibrationLookupData(const Self&) = delete;
  void operator=(const Self&) = delete;

  GainListType m_Gains;
  int          m_Offset;
};

} // end namespace otb

#endif
