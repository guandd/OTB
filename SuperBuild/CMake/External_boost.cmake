#
# Copyright (C) 2005-2019 Centre National d'Etudes Spatiales (CNES)
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

INCLUDE_ONCE_MACRO(BOOST)

SETUP_SUPERBUILD(BOOST)

set(_SB_BOOST_LIBRARYDIR ${SB_INSTALL_PREFIX}/lib)

set(BOOST_SB_CONFIG)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
	set(BOOST_SB_CONFIG variant=debug)
else()
	set(BOOST_SB_CONFIG variant=release
	--libdir=${_SB_BOOST_LIBRARYDIR}
	--with-system
	--with-serialization
	--with-filesystem
	--with-test
	--with-date_time
	--with-program_options
	--with-thread)
endif()

if(OTB_TARGET_SYSTEM_ARCH_IS_X64)
  set(BOOST_SB_CONFIG ${BOOST_SB_CONFIG} address-model=64)
endif()

 if(NOT APPLE AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(BOOST_SB_CONFIG ${BOOST_SB_CONFIG} toolset=clang)
endif()

if(MSVC AND MSVC_VERSION GREATER 1900)
 set(BOOST_SB_CONFIG ${BOOST_SB_CONFIG} toolset=msvc-14.1)
endif()

set(BOOST_SB_CONFIG
  ${BOOST_SB_CONFIG}
  link=shared
  threading=multi
  runtime-link=shared
  --prefix=${SB_INSTALL_PREFIX}
  --includedir=${SB_INSTALL_PREFIX}/include
  )

set(BOOST_BOOTSTRAP_OPTIONS "")
if(UNIX)
  set(BOOST_BOOTSTRAP_FILE "./bootstrap.sh")
  set(BOOST_B2_EXE "./b2")
  if(NOT APPLE AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(BOOST_BOOTSTRAP_OPTIONS "--with-toolset=clang")
  endif()
else()
  set(BOOST_BOOTSTRAP_FILE "bootstrap.bat")
  set(BOOST_B2_EXE "b2.exe")
endif()

set(BOOST_CONFIGURE_COMMAND ${CMAKE_COMMAND}
  -E chdir ${BOOST_SB_SRC}
  ${BOOST_BOOTSTRAP_FILE} ${BOOST_BOOTSTRAP_OPTIONS}
  --prefix=${SB_INSTALL_PREFIX}
  )

set(BOOST_BUILD_COMMAND ${CMAKE_COMMAND}
  -E chdir ${BOOST_SB_SRC}
  ${BOOST_B2_EXE}
  ${BOOST_SB_CONFIG}
  install
  )

if(MSVC)
  configure_file(${CMAKE_SOURCE_DIR}/patches/BOOST/project-config.jam.in
    ${CMAKE_BINARY_DIR}/BOOST/src/BOOST-stamp/project-config.jam)
endif()

#NOTE: update _SB_Boost_INCLUDE_DIR below when you change version number
ExternalProject_Add(BOOST
  PREFIX BOOST
  URL "https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.tar.bz2"
  URL_MD5 65a840e1a0b13a558ff19eeb2c4f0cbe #TODO: needs update
  BINARY_DIR ${BOOST_SB_BUILD_DIR}
  INSTALL_DIR ${SB_INSTALL_PREFIX}
  DOWNLOAD_DIR ${DOWNLOAD_LOCATION}
  CONFIGURE_COMMAND ${BOOST_CONFIGURE_COMMAND}
  BUILD_COMMAND ${BOOST_BUILD_COMMAND}
  INSTALL_COMMAND ""
  LOG_DOWNLOAD 1
  LOG_CONFIGURE 1
  LOG_BUILD 1
  LOG_INSTALL 1  
)

if(MSVC)
  ExternalProject_Add_Step(BOOST add_project_jam_config
	COMMAND ${CMAKE_COMMAND} -E copy
	${CMAKE_BINARY_DIR}/BOOST/src/BOOST-stamp/project-config.jam
	${BOOST_SB_SRC}
	DEPENDEES patch
	DEPENDERS configure)
endif()

#HINT: avoid all uses of  _SB_* in External_<project>.cmake
# and depend on much saner CMAKE_PREFIX_PATH for cmake projects.
if(MSVC)
  set(_SB_Boost_INCLUDE_DIR ${SB_INSTALL_PREFIX}/include/boost-1_69)
else()
  set(_SB_Boost_INCLUDE_DIR ${SB_INSTALL_PREFIX}/include)
endif()
SUPERBUILD_PATCH_SOURCE(BOOST)
