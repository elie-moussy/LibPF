# Copyright (C) LAAS-CNRS
# Author : Elie MOUSSY
#
# This file is part of LibPF.
# LibPF is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# Requires at least CMake 2.6 to configure the package.
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

INCLUDE(cmake/base.cmake)
INCLUDE(cmake/cpack.cmake)

SET(PROJECT_NAME LibPF)
SET(PROJECT_URL "http://github.com/elie-moussy/LibPF")
SET(PROJECT_VERSION 1.0)

SET(CXX_DISABLE_WERROR True)
SET(CMAKE_VERBOSE_MAKEFILE ON)

SETUP_PROJECT()

ADD_REQUIRED_DEPENDENCY("opencv >= 2.4.2")
INCLUDE_DIRECTORIES(${OPENCV_INCLUDE_DIRS})
#INCLUDE_DIRECTORIES(/usr/include/)

PKG_CONFIG_APPEND_LIBS("LibPF")

ADD_SUBDIRECTORY(include)
ADD_SUBDIRECTORY(src)

SETUP_PROJECT_FINALIZE()
SETUP_PROJECT_CPACK()