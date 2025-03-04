#
# Copyright (C) 2005-2020 Centre National d'Etudes Spatiales (CNES)
# Copyright (C) 2019 IRSTEA
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

set(DOCUMENTATION "This module provide components to enable the 
generation of images mosaics, including some color/radiometric
harmonization based on statistics of overlapping pixels.")

otb_module(OTBMosaic
  DEPENDS
    OTBCommon
    OTBConversion
    OTBFunctor

  TEST_DEPENDS

  DESCRIPTION
    "${DOCUMENTATION}"
)
