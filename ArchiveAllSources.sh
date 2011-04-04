#! /bin/bash
# For users of this script you will need to adjust the following variables:

# This is where all the projects are going to be checked out and built
workspace="/$HOME/Workspace"
# Set this to where you want to install all the libraries and executables. Each 
# project will be installed into its own sub-directory under that directory
CMAKE_INSTALL_PREFIX="/Users/Shared/Toolkits"
# The number of concurrent compiles:
parallel=2

# to make this work with your own setup. You will need the following projects in your path:
#
# CMake
# Git
# Qt
# If any of those are missing then this script will fail horribly.

GIT=`type -P git`
if [[ $GIT == "" ]]
  then
  echo "Git is needed for this script. Please install it on your system and be sure it is on your path."
  exit 1
fi

QMAKE=`type -P qmake`
if [[ $QMAKE == "" ]]
  then
  echo "An installation of Qt is required. Please install a version of Qt of at least 4.6 or greater."
  exit 1
fi

CMAKE=`type -P cmake`
if [[ $CMAKE == "" ]]
  then
  echo "An installation of CMake is required. Please install a version of CMake of at least 2.8.3 or greater."
  exit 1
fi

# Export the location of CMake into our path.
export PATH=$PATH:/Users/Shared/Toolkits/CMake-2.8.3/bin


# We are building from the command line so Makefiles work best. I have not tried it 
# with any other CMake generator
CMAKE_GENERATOR="Unix Makefiles"

archive_destination="/Volumes/Misc/TO81-Deliverables"
DATE=`date +%Y.%m.%d`

mkdir -p $archive_destination/4.1
mkdir -p $archive_destination/4.2
mkdir -p $archive_destination/4.3
mkdir -p $archive_destination/4.4
mkdir -p $archive_destination/4.5
mkdir -p $archive_destination/4.5/OS_X
mkdir -p $archive_destination/4.5/Source
mkdir -p $archive_destination/4.5/Windows
mkdir -p $archive_destination/4.6

echo "4.1: Copying Word Documents"
cd $workspace/EIM-Guides
projects="SCMPractices SoftwareDevelopment SoftwareDocumentation EmMpmGui"
for project in $projects
do
  cp $workspace/EIM-Guides/$project/$project.docx $archive_destination/4.1/.
done

echo "4.2: Copying Presentation Materials - NOT DONE YET"

echo "4.3: Copying Word Files"
project="SCMService"
cp $workspace/EIM-Guides/$project/$project.docx $archive_destination/4.3/.

echo "4.4: Copying Datasets MNML-3 and MNML-5"
cp /Users/Shared/Data/MNML-3_769_780_Aligned.zip $archive_destination/4.4/.
cp /Users/Shared/Data/MNML-5_610_640_Aligned.zip $archive_destination/4.4/.

echo "4.5: Assembling All Software source codes"
# Move into the archive_destination/DATE/Source directory
mkdir -p $archive_destination
cd $archive_destination/4.5
rm -rf Source
mkdir Source
cd Source

# clone all the projects
# git clone --recursive git://scm.bluequartz.net/support-libraries/tiff.git tiff
# git clone --recursive git://scm.bluequartz.net/support-libraries/mxaboost.git MXABoost
# git clone --recursive git://scm.bluequartz.net/eim/emmpm.git emmpm
# git clone --recursive git://scm.bluequartz.net/eim/emmpmgui.git emmpmgui
# git clone --recursive git://scm.bluequartz.net/eim/side.git side

# Use Git to archive the latest copy of each of the projects
projects="tiff MXABoost EIM-emmpm EIM-side EIM-EMMPMGui EIM-Guides Qwt"
for project in $projects;
do
  echo "4.5:  Archiving $project"
  cd $workspace/$project
  git archive --format=tar HEAD | gzip >$archive_destination/4.5/Source/$project.tar.gz
done

echo "4.5:  Copying Qt source code"
cp /Volumes/Backup/Warehouse/Qt-4.7.2/qt-everywhere-opensource-src-4.7.2.zip $archive_destination/4.5/Source/.

echo "4.6: Copying Final Report"
cp /Users/mjackson/BlueQuartz/Contracts_Current/AFRL-TO81/Final_Report_S587-081.docx $archive_destination/4.6

echo "You still need the following items:"
echo "  Precompiled Binaries for OS X and Windows"
echo "  CMake installers for Windows and OS X"