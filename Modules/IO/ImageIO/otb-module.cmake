#
# Copyright (C) 2005-2020 Centre National d'Etudes Spatiales (CNES)
#
# This file is part of Orfeo Toolbox
#
#     https://www.orfeo-toolbox.org/
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

set(DOCUMENTATION "This module contains classes related to the reading and the
writing of remote sensing images.")

otb_module(OTBImageIO
  ENABLE_SHARED
  DEPENDS
    OTBBoostAdapters
    OTBCommon
    OTBExtendedFilename
    OTBIOBSQ
    OTBIOGDAL
    OTBIOLUM
    OTBIOMSTAR
    OTBIOONERA
    OTBIORAD
    OTBITK
    OTBImageBase
    OTBMetadata
    OTBObjectList
    OTBStreaming

  TEST_DEPENDS
    OTBStatistics
    OTBTestKernel

  DESCRIPTION
    "${DOCUMENTATION}"
)
