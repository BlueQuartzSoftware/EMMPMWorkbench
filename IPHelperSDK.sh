#!/bin/bash

# This script will download and install the following projects and compile and install them into
# the location determined by the current working directory.
# The packages downloaded are:
# CMake - in order to build other projects
# MXABoost - subset of Boost 1.44
# ITK 3.20.0 - Image Processing Toolkit
# Qwt - 2D Plotting Widgets for the Qt toolkit
# You need to have the following programs already on your system:
#  curl or wget
#  git version 1.7 or greater
#  Qt version 4.5 or greater
# The script will create 2 directories as subdirectories of the current working dir: sources and Toolkits
# All projects are downloaded and compiled into sources and then installed into Toolkits.
# A bash style shell file can be called from your .bashrc or .bash_profile called 'initvars.sh' which
# will export a number of paths so that building projects from the IPHelper project goes smoother.
# You can simply 'source "initvars.sh" any time to get the exports.

# If you want to force another directory then change where "SDK_ROOT" points to.
SDK_ROOT=`pwd`
SDK_SOURCE=$SDK_ROOT/Workspace
SDK_INSTALL=$SDK_ROOT/Toolkits

# If you are on a Multicore system and want to build faster set the variable to the number
# of cores/CPUs available to you.
PARALLEL_BUILD=2

HOST_SYSTEM=`uname`


GIT=`type -P git`
if [ $GIT == "" ]
  then
  echo "Git is needed for this script. Please install it on your system and be sure it is on your path."
  exit 1
fi

QMAKE=`type -P qmake`
if [ $QMAKE == "" ]
  then
  echo "An installation of Qt is required. Please install a version of Qt of at least 4.6 or greater."
  exit 1
fi

WGET=`type -P wget`
CURL=`type -P curl`

if [ "$WGET" == "" ]
   then
  if [ "$CURL" == "" ]
     then
    echo "wget and curl are NOT present on your machine. One of them is needed to download sources from the internet."
    exit 1
  fi
fi

# Create some directories for our use
mkdir $SDK_SOURCE
mkdir $SDK_INSTALL
cd $SDK_SOURCE

DOWNLOAD_PROG=""
DOWNLOAD_ARGS=""

if [ "$WGET" != "" ]; then
  DOWNLOAD_PROG=$WGET
fi

if [ "$CURL" != "" ]; then
  DOWNLOAD_PROG=$CURL
  DOWNLOAD_ARGS="-o cmake-2.8.3.tar.gz"
fi

#Download and Compile CMake
$DOWNLOAD_PROG "http://www.cmake.org/files/v2.8/cmake-2.8.3.tar.gz" $DOWNLOAD_ARGS
tar -xvzf cmake-2.8.3.tar.gz
cd cmake-2.8.3
./configure --prefix=$SDK_INSTALL/cmake-2.8.3
make -j $PARALLEL_BUILD install

# Export these variables for our use and then echo them into a file that people can use
# to setup their environment
export CMAKE_INSTALL=$SDK_INSTALL/cmake-2.8.3
export PATH=$CMAKE_INSTALL/bin:$PATH

echo "export CMAKE_INSTALL=$SDK_INSTALL/cmake-2.8.3" >  $SDK_INSTALL/initvars.sh
echo "export PATH=\$CMAKE_INSTALL/bin:\$PATH" >>  $SDK_INSTALL/initvars.sh

#------------------------------------------------------------------------------
# We now need MXABoost on the system
cd $SDK_SOURCE
# Remove any previous MXABoost
rm -rf MXABoost
git clone git://scm.bluequartz.net/support-libraries/mxaboost.git MXABoost
cd MXABoost
git checkout boost-1.44
mkdir Build
cd Build
cmake -DBOOST_INCLUDE_INSTALL_DIR=include/boost-1_44 -DBOOST_LIB_INSTALL_DIR=lib -DCMAKE_INSTALL_PREFIX=$SDK_INSTALL/MXABoost-1.44 -DENABLE_SHARED=OFF -DWINMANGLE_LIBNAMES=ON ../
make -j $PARALLEL_BUILD install
export BOOST_ROOT=$SDK_INSTALL/MXABoost-1.44
echo "export BOOST_ROOT=$SDK_INSTALL/MXABoost-1.44" >> $SDK_INSTALL/initvars.sh


#------------------------------------------------------------------------------
cd $SDK_SOURCE
# Remove any previous Qwt
rm -rf Qwt
git clone --recursive git://scm.bluequartz.net/support-libraries/qwt.git Qwt
cd Qwt
mkdir Build
cd Build
# On OS X we need to set the "install_name" correctly on Libraries that will get used which is
# what the variable will do.
if ["$HOST_SYSTEM" == "Darwin" ]
  then
  ADDITIONAL_ARGS="-DCMP_BUILD_WITH_INSTALL_NAME=ON"
fi
cmake $ADDITIONAL_ARGS -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$SDK_INSTALL/Qwt -DBUILD_SHARED_LIBS=ON ../
make -j $PARALLEL_BUILD
make install
cd ../
mkdir zRel
cd zRel
cmake $ADDITIONAL_ARGS -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$SDK_INSTALL/Qwt -DBUILD_SHARED_LIBS=ON ../
make -j $PARALLEL_BUILD
make install
export QWT_INSTALL=$SDK_INSTALL/Qwt
echo "export QWT_INSTALL=$SDK_INSTALL/Qwt" >> $SDK_INSTALL/initvars.sh

#------------------------------------------------------------------------------
# Pull Down ITK and compile/Install it
cd $SDK_SOURCE
DOWNLOAD_ARGS="-o InsightToolkit-3.20.0.tar.gz"
$DOWNLOAD_PROG "http://voxel.dl.sourceforge.net/project/itk/itk/3.20/InsightToolkit-3.20.0.tar.gz" $DOWNLOAD_ARGS
tar -xvzf InsightToolkit-3.20.0.tar.gz
cd InsightToolkit-3.20.0
mkdir Build
cd Build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$SDK_INSTALL/ITK-3.20.0 -DBUILD_SHARED_LIBS=OFF -DBUILD_TESTING=OFF -DBUILD_EXAMPLES=OFF ../
make -j $PARALLEL_BUILD install
export ITK_DIR=$SDK_INSTALL/ITK-3.20.0/lib/InsightToolkit
echo "export ITK_DIR=$SDK_INSTALL/ITK-3.20.0/lib/InsightToolkit" >> $SDK_INSTALL/initvars.sh

#------------------------------------------------------------------------------
# Compile libTiff
cd $SDK_SOURCE
# Remove any previous tiff
rm -rf tiff
git clone --recursive git://scm.bluequartz.net/support-libraries/tiff.git tiff
cd tiff
mkdir Build
cd Build
# On OS X we need to set the "install_name" correctly on Libraries that will get used which is
# what the variable will do.
if ["$HOST_SYSTEM" == "Darwin" ]
  then
  ADDITIONAL_ARGS="-DCMP_BUILD_WITH_INSTALL_NAME=ON"
fi
cmake $ADDITIONAL_ARGS -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$SDK_INSTALL/tiff -DBUILD_SHARED_LIBS=ON ../
make -j $PARALLEL_BUILD
make install
cd ../
mkdir zRel
cd zRel
cmake $ADDITIONAL_ARGS -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$SDK_INSTALL/tiff -DBUILD_SHARED_LIBS=ON ../
make -j $PARALLEL_BUILD
make install
export TIFF_INSTALL=$SDK_INSTALL/tiff
echo "export TIFF_INSTALL=$SDK_INSTALL/tiff" >> $SDK_INSTALL/initvars.sh


#------------------------------------------------------------------------------
# Compile the emmpm Library
cd $SDK_SOURCE
# Remove any previous emmpm
rm -rf emmpm
git clone --recursive git://scm.bluequartz.net/eim/emmpm.git emmpm
cd emmpme
mkdir Build
cd Build
# On OS X we need to set the "install_name" correctly on Libraries that will get used which is
# what the variable will do.
if ["$HOST_SYSTEM" == "Darwin" ]
  then
  ADDITIONAL_ARGS="-DCMP_BUILD_WITH_INSTALL_NAME=ON"
fi
cmake $ADDITIONAL_ARGS -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$SDK_INSTALL/emmpm -DBUILD_SHARED_LIBS=ON ../
make -j $PARALLEL_BUILD
make install
cd ../
mkdir zRel
cd zRel
cmake $ADDITIONAL_ARGS -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$SDK_INSTALL/emmpm -DBUILD_SHARED_LIBS=ON ../
make -j $PARALLEL_BUILD
make install
export EMMPM_INSTALL=$SDK_INSTALL/emmpm
echo "export EMMPM_INSTALL=$SDK_INSTALL/emmpm" >> $SDK_INSTALL/initvars.sh

#------------------------------------------------------------------------------
# Compile the IPHelper
# Pull Down the IPHelper Source and compile it
cd $SDK_SOURCE
# Remove any previous IPHelper
rm -rf IPHelper
git clone git://scm.bluequartz.net/eim/iphelper.git IPHelper
cd IPHelper
git checkout user_init
mkdir Build
cd Build
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$SDK_INSTALL/IPHelper  ../
make -j $PARALLEL_BUILD 



