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


#include "otbImageMetadataInterfaceFactory.h"

#include "otbDefaultImageMetadataInterface.h"

// Optical sensors
#include "otbIkonosImageMetadataInterfaceFactory.h"
#include "otbSpotImageMetadataInterfaceFactory.h"
#include "otbPleiadesImageMetadataInterfaceFactory.h"
#include "otbSpot6ImageMetadataInterfaceFactory.h"
#include "otbFormosatImageMetadataInterfaceFactory.h"
#include "otbQuickBirdImageMetadataInterfaceFactory.h"
#include "otbWorldView2ImageMetadataInterfaceFactory.h"

// SAR Sensors
#include "otbTerraSarXSarImageMetadataInterfaceFactory.h"
#include "otbSentinel1ImageMetadataInterfaceFactory.h"
#include "otbCosmoImageMetadataInterfaceFactory.h"
#include "otbRadarsat2ImageMetadataInterfaceFactory.h"

#include "itkMutexLock.h"
#include "itkMutexLockHolder.h"

#include <iostream>
#include <iterator>
#include <vector>

namespace otb
{
ImageMetadataInterfaceFactory::ImageMetadataInterfaceBasePointerType
ImageMetadataInterfaceFactory
::CreateIMI(ImageMetadata & imd, const MetadataSupplierInterface & mds)
{
  RegisterBuiltInFactories();

  auto                                       allOpticalObjects = itk::ObjectFactoryBase::CreateAllInstance("OpticalImageMetadataInterface");
  auto                                       allSarObjects     = itk::ObjectFactoryBase::CreateAllInstance("SarImageMetadataInterface");
  std::list<itk::LightObject::Pointer>       allObjects;

  std::copy(allOpticalObjects.begin(), allOpticalObjects.end(), std::back_inserter(allObjects));
  std::copy(allSarObjects.begin(), allSarObjects.end(), std::back_inserter(allObjects));

  for (auto i = allObjects.begin(); i != allObjects.end(); ++i)
  {
    ImageMetadataInterfaceBase* io = dynamic_cast<ImageMetadataInterfaceBase*>(i->GetPointer());
    if (io)
    {
      // the static part of ImageMetadata is already filled
      io->SetMetadataSupplierInterface(mds);
      try
      {
        io->Parse(imd);
        return io;
      }
      catch(MissingMetadataException& e)
      {
        // silent catch of MissingMetadataException
        // just means that this IMI can't parse the file
      }
    }
    else
    {
      itkGenericExceptionMacro(<< "Error ImageMetadataInterface factory did not return an ImageMetadataInterfaceBase: " << (*i)->GetNameOfClass());
    }
  }

  auto defaultIMI = DefaultImageMetadataInterface::New();
  defaultIMI->SetMetadataSupplierInterface(mds);
  return dynamic_cast<ImageMetadataInterfaceBase*>(static_cast<DefaultImageMetadataInterface*>(defaultIMI));
}

void ImageMetadataInterfaceFactory::RegisterBuiltInFactories()
{
  static bool firstTime = true;

  static itk::SimpleMutexLock mutex;
  {
    // This helper class makes sure the Mutex is unlocked
    // in the event an exception is thrown.
    itk::MutexLockHolder<itk::SimpleMutexLock> mutexHolder(mutex);
    if (firstTime)
    {
      itk::ObjectFactoryBase::RegisterFactory(IkonosImageMetadataInterfaceFactory::New());
      itk::ObjectFactoryBase::RegisterFactory(SpotImageMetadataInterfaceFactory::New());
      itk::ObjectFactoryBase::RegisterFactory(PleiadesImageMetadataInterfaceFactory::New());
      itk::ObjectFactoryBase::RegisterFactory(Spot6ImageMetadataInterfaceFactory::New());
      itk::ObjectFactoryBase::RegisterFactory(FormosatImageMetadataInterfaceFactory::New());
      itk::ObjectFactoryBase::RegisterFactory(QuickBirdImageMetadataInterfaceFactory::New());
      itk::ObjectFactoryBase::RegisterFactory(WorldView2ImageMetadataInterfaceFactory::New());
      itk::ObjectFactoryBase::RegisterFactory(TerraSarXSarImageMetadataInterfaceFactory::New());
      itk::ObjectFactoryBase::RegisterFactory(Sentinel1ImageMetadataInterfaceFactory::New());
      itk::ObjectFactoryBase::RegisterFactory(CosmoImageMetadataInterfaceFactory::New());
      itk::ObjectFactoryBase::RegisterFactory(Radarsat2ImageMetadataInterfaceFactory::New());
      firstTime = false;
    }
  }
}

} // end namespace otb
