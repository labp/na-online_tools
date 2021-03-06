#---------------------------------------------------------------------------
#
# Project: NA-Online ( http://www.labp.htwk-leipzig.de )
#
# Copyright 2010 Laboratory for Biosignal Processing, HTWK Leipzig, Germany
#
# This file is part of NA-Online.
#
# NA-Online is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# NA-Online is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with NA-Online. If not, see <http://www.gnu.org/licenses/>.
#
#---------------------------------------------------------------------------

# --------------------------------------------------------------------------------------------------------------------------------
#
# General CMake Setup
#
# --------------------------------------------------------------------------------------------------------------------------------

CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )
PROJECT( na-online_tools )

IF( NOT CMAKE_BUILD_TYPE )
    SET( CMAKE_BUILD_TYPE Release )
ENDIF( NOT CMAKE_BUILD_TYPE )

SET( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../tools/cmake" )

FIND_PACKAGE( Eigen3 3.1.0 REQUIRED )
FIND_PACKAGE( Qt5Core REQUIRED )
FIND_PACKAGE( MNE REQUIRED )

# Support old CMake versions
ADD_DEFINITIONS( ${Qt5Core_DEFINITIONS} )
SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}" )


# --------------------------------------------------------------------------------------------------------------------------------
# nao_eeg_sensor_generator
# --------------------------------------------------------------------------------------------------------------------------------

SET( TARGET nao_eeg_sensor_generator )
ADD_EXECUTABLE( ${TARGET} eeg_sensor_generator.cpp )

INCLUDE_DIRECTORIES( ${TARGET} ${Qt5_INCLUDE_DIRS} ${Qt5Core_INCLUDE_DIRS} )
TARGET_LINK_LIBRARIES( ${TARGET} ${Qt5Core_LIBRARIES} )

INCLUDE_DIRECTORIES( ${TARGET} ${EIGEN3_INCLUDE_DIR} )

INCLUDE_DIRECTORIES( ${TARGET} ${MNE_INCLUDE_DIR} )
TARGET_LINK_LIBRARIES( ${TARGET} ${MNE_FIFF_LIBRARY} ${MNE_LIBRARY} )


# --------------------------------------------------------------------------------------------------------------------------------
# nao_eeg_sensor_transformator
# --------------------------------------------------------------------------------------------------------------------------------

SET( TARGET nao_eeg_sensor_transformator )
ADD_EXECUTABLE( ${TARGET} eeg_sensor_transformator.cpp )

INCLUDE_DIRECTORIES( ${TARGET} ${Qt5_INCLUDE_DIRS} ${Qt5Core_INCLUDE_DIRS} )
TARGET_LINK_LIBRARIES( ${TARGET} ${Qt5Core_LIBRARIES} )

INCLUDE_DIRECTORIES( ${TARGET} ${EIGEN3_INCLUDE_DIR} )

INCLUDE_DIRECTORIES( ${TARGET} ${MNE_INCLUDE_DIR} )
TARGET_LINK_LIBRARIES( ${TARGET} ${MNE_FIFF_LIBRARY} )
