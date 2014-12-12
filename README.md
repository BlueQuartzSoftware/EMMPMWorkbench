EMMPMWorkbench
==============

GUI Front end for EMMPM Segmentation algorithm

# Introduction to compiling #

There are several dependent repositories and libraries that you will need

+ CMP https://github.com/BlueQuartzSoftware/CMP.git
+ emmpm https://github.com/BlueQuartzSoftware/emmpm.git
+ MXADataModel https://github.com/BlueQuartzSoftware/MXADataModel.git

Clone the master branch of each repository into a directory so that each directory is at the same level.

The projects are CMake based so you will need CMake installed on your syste. In addition you will also need the following:

+ Boost
+ Qt 4.8.x
+ Qwt 5.2.x
+ HDF5 1.8.x

When you have everything installed then you can create a build directory and use CMake to configure the EMMPMWorkbench project. CMake should be able to find all the other projects.


