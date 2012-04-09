#! /bin/bash
# For users of this script you will need to adjust the following variables:

# This is where all the projects are going to be checked out and built
workspace="/$HOME/Workspace"

# Git
# If any of those are missing then this script will fail horribly.

GIT=`type -P git`
if [[ $GIT == "" ]]
  then
  echo "Git is needed for this script. Please install it on your system and be sure it is on your path."
  exit 1
fi

archive_destination="/tmp/"
DATE=`date +%Y.%m.%d`

mkdir -p $archive_destination


project=EIM-EMMPMGui

cd $workspace/$project
version=`git describe`
cd $workspace/

tar czvf /$archive_destination/$project-$version.tar.gz --exclude .gitmodules --exclude .cproject --exclude .project --exclude .git --exclude Build --exclude zRel $project/

