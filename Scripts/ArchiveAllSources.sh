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


project=EMMPMGui

cd $workspace/$project
version=`git describe`
cd $workspace/

cd $workspace/CMP
cmpvers=`git describe`

cd $workspace/MXADataModel
mxavers=`git describe`

cd $workspace/emmpm
emmpmvers=`git describe`

cd $workspace/EMMPMGui
guivers=`git describe`


cd $workspace/

echo "CMP Version: " $cmpvers > Versions.txt
echo "MXADataModel Version: " $mxavers >> Versions.txt
echo "EMMPM Lib Version: " $emmpmvers >> Versions.txt
echo "EMMPM Gui Version: " $guivers >> Versions.txt


tar czvf /$archive_destination/$project-Source.tar.gz --exclude .gitmodules --exclude .cproject --exclude .project --exclude .git --exclude Build --exclude zRel EMMPMGui/ emmpm/ MXADataModel/ CMP/  Versions.txt

scp /$archive_destination/$project-Source.tar.gz mjackson@scm.bluequartz.net:/var/www/www.bluequartz.net/binaries/to81/.

rm  Versions.txt

