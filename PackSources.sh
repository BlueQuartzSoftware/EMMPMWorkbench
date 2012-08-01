#!/bin/bash

cd /Users/mjackson/Workspace/EMMPMGui
version=`/usr/local/git/bin/git describe`
workspace=/Users/mjackson/Workspace
mkdir /tmp/Workspace

echo "Archiving Projects"
# Use Git to archive the latest copy of each of the projects
projects="EMMPMGui emmpm tiff CMP"
for project in $projects;
do
    cd $workspace/$project                                          
    echo "Archiving $project"
    mkdir -p /tmp/Workspace/$project
    /usr/local/git/bin/git archive master | gzip > /tmp/Workspace/$project/$project.tar.gz
done

for project in $projects;
do
    cd /tmp/Workspace/$project                                        
    tar -xvzf /tmp/Workspace/$project/$project.tar.gz
    rm /tmp/Workspace/$project/$project.tar.gz 
done

echo "EMMPMGui Version: "  $version >> /tmp/Workspace/Version.txt

cd /tmp

tar -cvzf EMMPMGui-$version.tar.gz Workspace

