/*
 * Copyright (C) 2005-2019 Centre National d'Etudes Spatiales (CNES)
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

#include "otbImageMetadata.h"
#include "otbSpatialReference.h"
#include "otbJoinContainer.h"

namespace otb
{
// ---------------------- [ImageMetadataBase] ------------------------------

ImageMetadataBase::ImageMetadataBase()
{
}

ImageMetadataBase::ImageMetadataBase(DictType<MDGeom, boost::any> geometryKeys,
                                     DictType<MDNum, double> numericKeys,
                                     DictType<MDStr, std::string> stringKeys,
                                     DictType<MDL1D, MetaData::LUT1D> lut1DKeys,
                                     DictType<MDL2D, MetaData::LUT2D> lut2DKeys,
                                     DictType<MDTime, MetaData::TimePoint> timeKeys,
                                     DictType<std::string, std::string> extraKeys)
  : GeometryKeys(std::move(geometryKeys)),
    NumericKeys(std::move(numericKeys)),
	StringKeys(std::move(stringKeys)),
	LUT1DKeys(std::move(lut1DKeys)),
	LUT2DKeys(std::move(lut2DKeys)),
	TimeKeys(std::move(timeKeys)),
	ExtraKeys(std::move(extraKeys))
{}

// -------------------- Geom utility function ----------------------------
const boost::any & ImageMetadataBase::operator[](const MDGeom& key) const
{
  return GeometryKeys.at(key);
}

const Projection::GCPParam & ImageMetadataBase::GetGCPParam() const
{
  return boost::any_cast<const Projection::GCPParam &>(GeometryKeys.at(MDGeom::GCP));
}

const Projection::RPCParam & ImageMetadataBase::GetRPCParam() const
{
  return boost::any_cast<const Projection::RPCParam &>(GeometryKeys.at(MDGeom::RPC));
}

const SARParam & ImageMetadataBase::GetSARParam() const
{
  return boost::any_cast<const SARParam &>(GeometryKeys.at(MDGeom::SAR));
}

std::string ImageMetadataBase::GetProjectedGeometry() const
{
  if (this->Has(MDGeom::ProjectionWKT))
  {
    // MDGeom::ProjectionWKT is a std::string stored as a boost::any
    return boost::any_cast<std::string>(GeometryKeys.at(MDGeom::ProjectionWKT));
  }
  else if (this->Has(MDGeom::ProjectionEPSG))
  {
    // MDGeom::ProjectionEPSG is an integer stored as a boost::any
    return std::to_string(boost::any_cast<int>(GeometryKeys.at(MDGeom::ProjectionEPSG)));
  }
  else if (this->Has(MDGeom::ProjectionProj))
  {
    // MDGeom::ProjectionProj is a std::string stored as a boost::any
    return boost::any_cast<std::string>(GeometryKeys.at(MDGeom::ProjectionProj));
  }
  else
    return "";
}

std::string ImageMetadataBase::GetProjectionWKT() const
{
  auto theProj = this->GetProjectedGeometry();
  if (theProj.empty())
    return "";
  else
    return SpatialReference::FromDescription(theProj).ToWkt();
}

std::string ImageMetadataBase::GetProjectionProj() const
{
  auto proj = GeometryKeys.find(MDGeom::ProjectionProj);
  if (proj !=  GeometryKeys.end())
  {
    return boost::any_cast<std::string>(proj->second);
  }
  else
  {
    return "";
  }
}

void ImageMetadataBase::Add(const MDGeom& key, const boost::any &value)
{
  GeometryKeys[key] = value;
}

size_t ImageMetadataBase::Remove(const MDGeom& key)
{
  return GeometryKeys.erase(key);
}

size_t ImageMetadataBase::RemoveSensorGeometry()
{
  return Remove(MDGeom::RPC) + Remove(MDGeom::SAR) + Remove(MDGeom::SensorGeometry);
}

size_t ImageMetadataBase::RemoveProjectedGeometry()
{
  return Remove(MDGeom::ProjectionWKT) + Remove(MDGeom::ProjectionEPSG) + Remove(MDGeom::ProjectionProj);
}

bool ImageMetadataBase::Has(const MDGeom& key) const
{
  return (GeometryKeys.find(key) != GeometryKeys.end());
}

bool ImageMetadataBase::HasSensorGeometry() const
{
  return Has(MDGeom::RPC) || Has(MDGeom::SAR) || Has(MDGeom::SensorGeometry);
}

bool ImageMetadataBase::HasProjectedGeometry() const
{
  return Has(MDGeom::ProjectionWKT) || Has(MDGeom::ProjectionEPSG) || Has(MDGeom::ProjectionProj);
}

// -------------------- Double utility function ----------------------------

const double & ImageMetadataBase::operator[](const MDNum& key) const
{
  return NumericKeys.at(key);
}

void ImageMetadataBase::Add(const MDNum& key, const double &value)
{
  NumericKeys[key] = value;
}

size_t ImageMetadataBase::Remove(const MDNum& key)
{
  return NumericKeys.erase(key);
}

bool ImageMetadataBase::Has(const MDNum& key) const
{
  return (NumericKeys.find(key) != NumericKeys.end());
}

std::string ImageMetadataBase::GetKeyListNum() const
{
  std::ostringstream oss;
  for (const auto& kv : MetaData::MDNumNames.left)
    oss << kv.second << " ";
  auto returnString = oss.str();
  returnString.pop_back();
  return returnString;
}

// -------------------- String utility function ----------------------------

const std::string & ImageMetadataBase::operator[](const MDStr& key) const
{
  return StringKeys.at(key);
}

void ImageMetadataBase::Add(const MDStr& key, const std::string &value)
{
  StringKeys[key] = value;
}

size_t ImageMetadataBase::Remove(const MDStr& key)
{
  return StringKeys.erase(key);
}

bool ImageMetadataBase::Has(const MDStr& key) const
{
  return (StringKeys.find(key) != StringKeys.end());
}

std::string ImageMetadataBase::GetKeyListStr() const
{
  std::ostringstream oss;
  JoinMap(oss, MetaData::MDStrNames.left, " ");
  return oss.str();
}

// -------------------- LUT1D utility function ----------------------------

const MetaData::LUT1D & ImageMetadataBase::operator[](const MDL1D& key) const
{
  return LUT1DKeys.at(key);
}

void ImageMetadataBase::Add(const MDL1D& key, const MetaData::LUT1D &value)
{
  LUT1DKeys[key] = value;
}

size_t ImageMetadataBase::Remove(const MDL1D& key)
{
  return LUT1DKeys.erase(key);
}

bool ImageMetadataBase::Has(const MDL1D& key) const
{
  return (LUT1DKeys.find(key) != LUT1DKeys.end());
}

std::string ImageMetadataBase::GetKeyListL1D() const
{
  std::ostringstream oss;
  for (const auto& kv : MetaData::MDL1DNames.left)
    oss << kv.second << " ";
  auto returnString = oss.str();
  returnString.pop_back();
  return returnString;
}

// -------------------- 2D LUT utility function ----------------------------

const MetaData::LUT2D & ImageMetadataBase::operator[](const MDL2D& key) const
{
  return LUT2DKeys.at(key);
}

void ImageMetadataBase::Add(const MDL2D& key, const MetaData::LUT2D &value)
{
  LUT2DKeys[key] = value;
}

size_t ImageMetadataBase::Remove(const MDL2D& key)
{
  return LUT2DKeys.erase(key);
}

bool ImageMetadataBase::Has(const MDL2D& key) const
{
  return (LUT2DKeys.find(key) != LUT2DKeys.end());
}

//std::string ImageMetadataBase::GetKeyListL2D() const
//{
//  std::ostringstream oss;
//  for (const auto& kv : MetaData::MDL2DNames.left)
//    oss << kv.second << " ";
//  auto returnString = oss.str();
//  returnString.pop_back();
//  return returnString;
//}

// -------------------- Time utility function ----------------------------

const MetaData::TimePoint & ImageMetadataBase::operator[](const MDTime& key) const
{
  return TimeKeys.at(key);
}

void ImageMetadataBase::Add(const MDTime& key, const MetaData::TimePoint &value)
{
  TimeKeys[key] = value;
}

size_t ImageMetadataBase::Remove(const MDTime& key)
{
  return TimeKeys.erase(key);
}

bool ImageMetadataBase::Has(const MDTime& key) const
{
  return (TimeKeys.find(key) != TimeKeys.end());
}

std::string ImageMetadataBase::GetKeyListTime() const
{
  std::ostringstream oss;
  for (const auto& kv : MetaData::MDTimeNames.left)
    oss << kv.second << " ";
  auto returnString = oss.str();
  returnString.pop_back();
  return returnString;
}

// -------------------- Extra keys utility function --------------------------

const std::string & ImageMetadataBase::operator[](const std::string & key) const
{
  return ExtraKeys.at(key);
}

void ImageMetadataBase::Add(const std::string& key, const std::string &value)
{
  ExtraKeys[key] = value;
}

size_t ImageMetadataBase::Remove(const std::string& key)
{
  return ExtraKeys.erase(key);
}

bool ImageMetadataBase::Has(const std::string& key) const
{
  return (ExtraKeys.find(key) != ExtraKeys.end());
}

void ImageMetadataBase::ToKeywordlist(Keywordlist& kwl) const
{
  kwl.clear();
  std::ostringstream oss;

  // Converting the GeomKeys
  for (const auto& kv : GeometryKeys)
  {
    oss.str("");
    if (kv.first == MDGeom::RPC)
    {
      // To be completed by ImageIO
      oss << std::string("<RPCParam>");
    }
    else if (kv.first == MDGeom::ProjectionEPSG)
    {
      oss << std::to_string(boost::any_cast<int>(kv.second));
    }
    else if (kv.first == MDGeom::GCP)
    {
      // To be completed by ImageIO
      oss << std::string("<GCPParam>");
    }
    else if (kv.first == MDGeom::SensorGeometry)
    {
    // MDGeom::SensorGeometry should be exported as "<typeinfo>" where typeinfo is boost::any::type().name()
      oss << kv.second.type().name();
    }
    else if (kv.first == MDGeom::SAR)
    {
      // To be completed by ImageIO
      oss << std::string("<SARParam>");
    }
    else if (kv.first == MDGeom::SARCalib)
    {
      // To be completed by ImageIO
      oss << std::string("<SARCalib>");
    }
    // TODO : MDGeom::Adjustment
    else
    {
      oss << boost::any_cast<std::string>(kv.second);
    }
    kwl.emplace(MetaData::MDGeomNames.left.at(kv.first), oss.str());
  }
  // Converting the StringKeys
  for (const auto& kv : StringKeys)
  {
    kwl.emplace(MetaData::MDStrNames.left.at(kv.first), kv.second);
  }
  // Converting the NumericKeys
  for (const auto& kv : NumericKeys)
  {
    oss.str("");
    oss << kv.second;
    kwl.emplace(MetaData::MDNumNames.left.at(kv.first), oss.str());
  }
  // Converting the LUT1DKeys
  for (const auto& kv : LUT1DKeys)
  {
    oss.str("");
    oss << kv.second.ToString();
    kwl.emplace(MetaData::MDL1DNames.left.at(kv.first), oss.str());
  }
  // Converting the LUT2DKeys
  for (const auto& kv : LUT2DKeys)
  {
    oss.str("");
    oss << kv.second.ToString();
    kwl.emplace(MetaData::MDL2DNames.left.at(kv.first), oss.str());
  }
  // Converting the TimeKeys
  for (const auto& kv : TimeKeys)
  {
    oss.str("");
    oss << kv.second;
    kwl.emplace(MetaData::MDTimeNames.left.at(kv.first), oss.str());
  }
  // Converting the ExtraKeys
  for (const auto& kv : ExtraKeys)
  {
    kwl.emplace(kv.first, kv.second);
  }
}

std::string ImageMetadataBase::ToJSON(bool multiline) const
{
  Keywordlist kwl;
  ToKeywordlist(kwl);
  std::ostringstream oss;
  std::string sep;
  if (multiline)
    {
    sep = "\n";
    }
  oss << "{";
  for (const auto& kv : kwl)
    {
    oss << "\""<< kv.first << "\": \"" << kv.second << "\"," << sep;
    }
  oss << "}";
  return oss.str();
}

bool ImageMetadataBase::FromKeywordlist(const Keywordlist& kwl)
{
  // search iterators
  for (const auto& kv : kwl)
  {
  // Converting the GeomKeys
    auto geomKey = MetaData::MDGeomNames.right.find(kv.first);
    if (geomKey != MetaData::MDGeomNames.right.end())
    {
      if (geomKey->second ==  MDGeom::ProjectionEPSG)
      {
        this->Add(geomKey->second, Utils::LexicalCast<int>(kv.second.c_str(), "Keywordlist.second.c_str()"));
      }
      // TODO : MDGeom::Adjustment
      else if (geomKey->second ==  MDGeom::ProjectionWKT ||geomKey->second ==  MDGeom:: ProjectionProj)
      {
        this->Add(geomKey->second, kv.second);
      }
      // skip MDGeom::SensorGeometry, MDGeom::RPC, MDGeom::GCP, MDGeom::SAR and MDGeom::SARCalib
      continue;
    }

    auto findGeomPrefix = [&kv](const MetaData::MDGeomBmType & bimap)
      {
        for (const auto & elem: bimap.right)
        {
          if (kv.first.rfind(elem.first + ".") != std::string::npos)
          {
            return true;
          }
        }
        return false;
      };

    if (findGeomPrefix(MetaData::MDGeomNames))
    {
      continue;
    }

  // Converting the StringKeys
    auto strKey = MetaData::MDStrNames.right.find(kv.first);
    if (strKey != MetaData::MDStrNames.right.end())
    {
      this->Add(strKey->second, kv.second);
      continue;
    }
  // Converting the NumericKeys
    auto numKey = MetaData::MDNumNames.right.find(kv.first);
    if (numKey != MetaData::MDNumNames.right.end())
    {
      this->Add(numKey->second, Utils::LexicalCast<double>(kv.second.c_str(), "Keywordlist.second.c_str()"));
      continue;
    }
  // Converting the LUT1DKeys
    auto lut1dKey = MetaData::MDL1DNames.right.find(kv.first);
    if (lut1dKey != MetaData::MDL1DNames.right.end())
    {
      MetaData::LUT1D lut;
      lut.FromString(kv.second);
      this->Add(lut1dKey->second, lut);
      continue;
    }
  // Converting the LUT2DKeys
    auto lut2dKey = MetaData::MDL2DNames.right.find(kv.first);
    if (lut2dKey != MetaData::MDL2DNames.right.end())
    {
      MetaData::LUT2D lut;
      lut.FromString(kv.second);
      this->Add(lut2dKey->second, lut);
      continue;
    }
  // Converting the TimeKeys
    auto timeKey = MetaData::MDTimeNames.right.find(kv.first);
    if (timeKey != MetaData::MDTimeNames.right.end())
    {
      MetaData::TimePoint time;
      std::istringstream (kv.second) >> time;
      this->Add(timeKey->second, time);
      continue;
    }
    otbLogMacro(Debug, << "The metadata named '" << kv.first << "' with value '" << kv.second << "' was added to ExtraKeys.");
    this->Add(kv.first, kv.second);
  }
  return true;
}


/** concatenate with an other ImageMetadata */
void ImageMetadataBase::Fuse(const ImageMetadataBase& imd)
{
  // Copy the keys
  this->GeometryKeys.insert(imd.GeometryKeys.begin(), imd.GeometryKeys.end());
  this->NumericKeys.insert(imd.NumericKeys.begin(), imd.NumericKeys.end());
  this->StringKeys.insert(imd.StringKeys.begin(), imd.StringKeys.end());
  this->LUT1DKeys.insert(imd.LUT1DKeys.begin(), imd.LUT1DKeys.end());
  this->LUT2DKeys.insert(imd.LUT2DKeys.begin(), imd.LUT2DKeys.end());
  this->TimeKeys.insert(imd.TimeKeys.begin(), imd.TimeKeys.end());
  this->ExtraKeys.insert(imd.ExtraKeys.begin(), imd.ExtraKeys.end());
}

std::vector<unsigned int> ImageMetadataBase::GetDefaultDisplay() const
{
  unsigned int redChannel = Has(MDNum::RedDisplayChannel) ? NumericKeys.at(MDNum::RedDisplayChannel) : 0;
  unsigned int greenChannel = Has(MDNum::GreenDisplayChannel) ? NumericKeys.at(MDNum::GreenDisplayChannel) : 1;
  unsigned int blueChannel = Has(MDNum::BlueDisplayChannel) ? NumericKeys.at(MDNum::BlueDisplayChannel) : 2;

  return {redChannel, greenChannel, blueChannel};
}

int ImageMetadataBase::GetSize() const
{
  return GeometryKeys.size() + NumericKeys.size() + StringKeys.size() + LUT1DKeys.size()
      + LUT2DKeys.size() + TimeKeys.size() + ExtraKeys.size();
}

// ----------------------- [ImageMetadata] ------------------------------


ImageMetadata::ImageMetadata(DictType<MDGeom, boost::any> geometryKeys,
                             DictType<MDNum, double> numericKeys,
                             DictType<MDStr, std::string> stringKeys,
                             DictType<MDL1D, MetaData::LUT1D> lut1DKeys,
                             DictType<MDL2D, MetaData::LUT2D> lut2DKeys,
                             DictType<MDTime, MetaData::TimePoint> timeKeys,
                             DictType<std::string, std::string> extraKeys,
                             ImageMetadataBandsType bands)
  : ImageMetadataBase(geometryKeys, numericKeys, stringKeys, lut1DKeys, lut2DKeys, timeKeys, extraKeys),
    Bands(std::move(bands))
{}

ImageMetadata ImageMetadata::slice(int start, int end) const
{
  assert(start <= end);

  auto first = this->Bands.cbegin() + start;
  auto last = this->Bands.cbegin() + end + 1;
  ImageMetadata imd(this->GeometryKeys, this->NumericKeys, this->StringKeys, this->LUT1DKeys,
                    this->LUT2DKeys, this->TimeKeys, this->ExtraKeys, ImageMetadataBandsType(first, last));
  return imd;
}

/** concatenate with an other ImageMetadata */
void ImageMetadata::append(const ImageMetadata& imd)
{
  ImageMetadataBase::Fuse(imd);
  
  // Copy the bands
  this->Bands.insert(this->Bands.end(), imd.Bands.begin(), imd.Bands.end());
}

void ImageMetadata::Merge(const ImageMetadata& imd)
{
  ImageMetadataBase::Fuse(imd);
  
  for (unsigned int i = 0; i < std::min(Bands.size(), imd.Bands.size()); ++i)
  {
    Bands[i].Fuse(imd.Bands[i]);
  }
}
/** if all bands share the same value of a key, put it at top level */
void ImageMetadata::compact()
{
  if (this->Bands.size() < 2)
    return;
  bool compactVal;
  // TODO Compact the GeometryKeys when comparisons exists
  // Compact NumericKeys
  auto numKeys = this->Bands.front().NumericKeys;
  for (const auto& kv : numKeys)
  {
    compactVal = true;
    auto bandIt = this->Bands.cbegin();
    ++bandIt;
    for ( ; bandIt != this->Bands.cend() ; ++bandIt)
    {
      auto otherKey = bandIt->NumericKeys.find(kv.first);
      if ((otherKey == bandIt->NumericKeys.end())
       || !itk::Math::AlmostEquals(otherKey->second, kv.second))
      {
        compactVal = false;
        break;
      }
    }
    if (compactVal)
    {
      this->Add(kv.first, kv.second);
      for (auto& band : this->Bands)
      {
        band.NumericKeys.erase(kv.first);
      }
    }
  }
  // Compact StringKeys
  auto strKeys = this->Bands.front().StringKeys;
  for (const auto& kv : strKeys)
  {
    compactVal = true;
    auto bandIt = this->Bands.cbegin();
    ++bandIt;
    for ( ; bandIt != this->Bands.cend() ; ++bandIt)
    {
      auto otherKey = bandIt->StringKeys.find(kv.first);
      if ((otherKey == bandIt->StringKeys.end())
       || !(otherKey->second == kv.second))
      {
        compactVal = false;
        break;
      }
    }
    if (compactVal)
    {
      this->Add(kv.first, kv.second);
      for (auto& band : this->Bands)
      {
        band.StringKeys.erase(kv.first);
      }
    }
  }
  // TODO Compact LUT1DKeys when comparisons exists
  // TODO Compact LUT2DKeys when comparisons exists
  // Compact TimeKeys
  auto timeKeys = this->Bands.front().TimeKeys;
  for (const auto& kv : timeKeys)
  {
    compactVal = true;
    auto bandIt = this->Bands.cbegin();
    ++bandIt;
    for ( ; bandIt != this->Bands.cend() ; ++bandIt)
    {
      auto otherKey = bandIt->TimeKeys.find(kv.first);
      if ((otherKey == bandIt->TimeKeys.end())
       || !itk::Math::AlmostEquals(otherKey->second.GetJulianDay(), kv.second.GetJulianDay()))
      {
        compactVal = false;
        break;
      }
    }
    if (compactVal)
    {
      this->Add(kv.first, kv.second);
      for (auto& band : this->Bands)
      {
        band.TimeKeys.erase(kv.first);
      }
    }
  }
  // Compact ExtraKeys
  auto extraKeys = this->Bands.front().ExtraKeys;
  for (const auto& kv : extraKeys)
  {
    compactVal = true;
    auto bandIt = this->Bands.cbegin();
    ++bandIt;
    for ( ; bandIt != this->Bands.cend() ; ++bandIt)
    {
      auto otherKey = bandIt->ExtraKeys.find(kv.first);
      if ((otherKey == bandIt->ExtraKeys.end())
       || !(otherKey->second == kv.second))
      {
        compactVal = false;
        break;
      }
    }
    if (compactVal)
    {
      this->Add(kv.first, kv.second);
      for (auto& band : this->Bands)
      {
        band.ExtraKeys.erase(kv.first);
      }
    }
  }
}

void ImageMetadata::AppendToKeywordlists(KeywordlistVector& kwlVect) const
{
  Keywordlist kwl;
  this->ToKeywordlist(kwl);
  kwlVect.push_back(kwl);
  this->AppendToBandKeywordlists(kwlVect);
}

void ImageMetadata::AppendToBandKeywordlists(KeywordlistVector& kwlVect) const
{
  Keywordlist kwl;
  for (const auto& band: this->Bands)
  {
    band.ToKeywordlist(kwl);
    kwlVect.push_back(kwl);
  }
}

bool ImageMetadata::FromKeywordlists(const KeywordlistVector& kwlVect)
{
  bool all_parsed = true;
  auto kwlIt = kwlVect.cbegin();
  all_parsed = this->FromKeywordlist(*kwlIt) && all_parsed;
  ++kwlIt;
  for ( ; kwlIt != kwlVect.cend() ; ++kwlIt)
  {
    ImageMetadataBase imb;
    all_parsed = imb.FromKeywordlist(*kwlIt) && all_parsed;
    this->Bands.push_back(imb);
  }
  return all_parsed;
}

void ImageMetadata::Add(const MDNum& key, const MetaDataKey::VariableLengthVectorType vlv)
{
  assert(this->Bands.size() == vlv.Size());
  const double* vlvIt = vlv.GetDataPointer();
  for(auto band = this->Bands.begin() ; band != this->Bands.end() ; ++band, ++vlvIt)
  {
    band->Add(key, *vlvIt);
  }
}


itk::VariableLengthVector<double> ImageMetadata::GetAsVector(const MDNum& key) const
{
  itk::VariableLengthVector<double> output(Bands.size());
  int i = 0;
  for (const auto & band : Bands)
  {
    output[i] = band[key];
    i++;
  }

  return output;
}



bool ImageMetadata::HasBandMetadata(const MDNum & key) const
{
  return std::all_of(Bands.begin(), Bands.end(),
                      [key](ImageMetadataBase band){return band.Has(key);});
}

bool ImageMetadata::HasBandMetadata(const MDL1D & key) const
{
  return std::all_of(Bands.begin(), Bands.end(),
                      [key](ImageMetadataBase band){return band.Has(key);});
}

std::vector<std::string> GetBandInfo(MDStr key, const ImageMetadata & imd)
{
  std::vector<std::string> output;
  for (const auto & band : imd.Bands)
  {
    if (band.Has(key))
    {
      output.push_back(band[key]);
    }
    else
    {
      return {};
    }
  }
  return output;
}

std::vector<std::string> ImageMetadata::GetBandNames() const
{
  return GetBandInfo(MDStr::BandName, *this);
}

std::vector<std::string> ImageMetadata::GetEnhancedBandNames() const
{
  return GetBandInfo(MDStr::EnhancedBandName, *this);
}

int ImageMetadata::GetSize() const
{
  int size = this->ImageMetadataBase::GetSize();
  for (const auto &band : this->Bands)
    size += band.GetSize();
  return size;
}

// printing
std::ostream& operator<<(std::ostream& os, const otb::ImageMetadataBase& imd)
{
  os << imd.ToJSON(true);
  return os;
}

std::ostream& operator<<(std::ostream& os, const otb::ImageMetadata& imd)
{
  os << "{";
  os << imd.ToJSON(true);
  os << "Bands[";
  for (auto &band : imd.Bands)
    {
    os << band.ToJSON(true) << ",";
    }
  os << "]\n";
  os << "}";

  return os;
}


bool HasOpticalSensorMetadata(const ImageMetadata & imd)
{
  auto hasBandMetadataNum = [&imd](MDNum key)
                        {return std::all_of(imd.Bands.begin(),
                                            imd.Bands.end(),
                                            [key](ImageMetadataBase band){return band.Has(key);});};

  auto hasBandMetadataLut = [&imd](MDL1D key)
                        {return std::all_of(imd.Bands.begin(),
                                            imd.Bands.end(),
                                            [key](ImageMetadataBase band){return band.Has(key);});};

  return imd.Has(MDStr::SensorID)
      && imd.Has(MDNum::SunElevation)
      && imd.Has(MDNum::SunAzimuth)
      && imd.Has(MDNum::SatElevation)
      && imd.Has(MDNum::SatAzimuth)
      && imd.Has(MDTime::AcquisitionDate)
      && imd.Has(MDTime::ProductionDate)
      && hasBandMetadataNum(MDNum::PhysicalBias)
      && hasBandMetadataNum(MDNum::PhysicalGain)
      && hasBandMetadataNum(MDNum::SolarIrradiance)
      && hasBandMetadataLut(MDL1D::SpectralSensitivity);
}


bool HasSARSensorMetadata(const ImageMetadata & imd)
{
  auto hasBandMetadata = [&imd](auto key) {
    return std::all_of(imd.Bands.begin(),
		       imd.Bands.end(),
		       [key](ImageMetadataBase band){return band.Has(key);});
  };

  return imd.Has(MDStr::SensorID)
    && imd.Has(MDStr::Mission)
    && imd.Has(MDStr::ProductType)
//    && (imd.Has(MDNum::RadarFrequency) || hasBandMetadata(MDNum::RadarFrequency))
//    && (imd.Has(MDNum::PRF) || hasBandMetadata(MDNum::PRF))
    && imd.Has(MDTime::AcquisitionStartTime)
    && imd.Has(MDStr::OrbitDirection)
    && (hasBandMetadata(MDStr::Polarization) || imd.Has(MDStr::Polarization));
}

void WriteImageMetadataToGeomFile(const ImageMetadata & imd, const std::string & filename)
{
  std::ofstream myfile(filename);
  auto writeKwl = [&myfile](const MetaData::Keywordlist & kwl, const std::string prefix ="")
  {
    for (const auto & kv: kwl)
    {
      myfile << prefix << kv.first << ":" << kv.second << std::endl;
    }
  };

  MetaData::Keywordlist kwl;
  imd.ToKeywordlist(kwl);
  writeKwl(kwl);

  // Band are indexed starting at 1
  int bIdx = 1;
  for (const auto& band : imd.Bands)
  {
    band.ToKeywordlist(kwl);
    writeKwl(kwl, "Band_" + std::to_string(bIdx) + ".");
    ++bIdx;
  }
}

bool HasSameRPCModel(const ImageMetadataBase& a, const ImageMetadataBase& b)
{
  bool aHasRpc = a.Has(MDGeom::RPC);
  bool bHasRpc = b.Has(MDGeom::RPC);

  if (aHasRpc && bHasRpc)
    // Both have a model
    return a.GetRPCParam() == b.GetRPCParam();

  else if (aHasRpc != bHasRpc)
    // One has a model, not the other
    return false;

  // Both don't have a model
  return true;
}

bool HasSameSARModel(const ImageMetadataBase& a, const ImageMetadataBase& b)
{
  bool aHasSar = a.Has(MDGeom::SAR);
  bool bHasSar = b.Has(MDGeom::SAR);

  if (aHasSar && bHasSar)
    // Both have a model
    return a.GetSARParam() == b.GetSARParam();

  else if (aHasSar != bHasSar)
    // One has a model, not the other
    return false;

  // Both don't have a model
  return true;
}

bool HasSameSensorModel(const ImageMetadataBase& a, const ImageMetadataBase& b)
{
  return HasSameRPCModel(a, b) && HasSameSARModel(a, b);
}

}
