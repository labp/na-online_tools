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

IF( NOT EXISTS "$ENV{MNE_CPP_ROOT}" )
    SET( MNE_CPP_ROOT ~/na-online_dependencies/mne-cpp )
    MESSAGE( "Using default MNE_CPP_ROOT, set it with the environment variable: MNE_CPP_ROOT" )
ELSEIF( NOT EXISTS "$ENV{MNE_CPP_ROOT}" )
    SET( MNE_CPP_ROOT ENV{MNE_CPP_ROOT} )
ENDIF( NOT EXISTS "$ENV{MNE_CPP_ROOT}" )
MESSAGE( "MNE_CPP_ROOT: ${MNE_CPP_ROOT}" )

# --------------------------------------------------------------------------------------------------------------------------------
# eeg_sensor_generator
# --------------------------------------------------------------------------------------------------------------------------------

SET( TARGET nao_eeg_sensor_generator )
ADD_EXECUTABLE( ${TARGET} eeg_sensor_generator.cpp )

FIND_PACKAGE( Qt5Core REQUIRED )
TARGET_INCLUDE_DIRECTORIES( ${TARGET} PRIVATE ${Qt5_INCLUDE_DIRS} ${Qt5Core_INCLUDE_DIRS} )
TARGET_LINK_LIBRARIES( ${TARGET} ${Qt5Core_LIBRARIES} )

FIND_PACKAGE( Eigen3 REQUIRED )
TARGET_INCLUDE_DIRECTORIES( ${TARGET} PRIVATE ${EIGEN3_INCLUDE_DIR} )

FIND_LIBRARY( MNE_FIFF MNE1Fiff ${MNE_CPP_ROOT}/lib )
FIND_LIBRARY( MNE_MNE MNE1Mne ${MNE_CPP_ROOT}/lib )
TARGET_INCLUDE_DIRECTORIES( ${TARGET} PRIVATE ${MNE_CPP_ROOT}/MNE )
TARGET_LINK_LIBRARIES( ${TARGET} ${MNE_FIFF} ${MNE_MNE} )


# --------------------------------------------------------------------------------------------------------------------------------
# eeg_sensor_transformator
# --------------------------------------------------------------------------------------------------------------------------------

SET( TARGET nao_eeg_sensor_transformator )
ADD_EXECUTABLE( ${TARGET} eeg_sensor_transformator.cpp )

FIND_PACKAGE( Qt5Core REQUIRED )
TARGET_INCLUDE_DIRECTORIES( ${TARGET} PRIVATE ${Qt5_INCLUDE_DIRS} ${Qt5Core_INCLUDE_DIRS} )
TARGET_LINK_LIBRARIES( ${TARGET} ${Qt5Core_LIBRARIES} )

FIND_PACKAGE( Eigen3 REQUIRED )
TARGET_INCLUDE_DIRECTORIES( ${TARGET} PRIVATE ${EIGEN3_INCLUDE_DIR} )

FIND_LIBRARY( MNE_FIFF MNE1Fiff ${MNE_CPP_ROOT}/lib )
TARGET_INCLUDE_DIRECTORIES( ${TARGET} PRIVATE ${MNE_CPP_ROOT}/MNE )
TARGET_LINK_LIBRARIES( ${TARGET} ${MNE_FIFF} )