# KinectBlenderTool

There are 3 folders in this repository,
- KinectV1, containing the code for KinectSDK v1.8, with OpenFrameworks v0.8.0
- KinectV2, containing the code for KinectSDK v2, with OpenFrameworks v0.8.4
- BlenderKinect, containing the Blender files used for our simulations

[KinectV1]
The KinectV1 code was built in Windows 7, with VisualStudio 2012. The project used is in KinectV1>apps>myApps>kinectExample folder. To have an easier time with the graphical aspect of the code, we used OpenFrameworks with the KinectCommonBridge add-on helping getting values from Kinect. After installing the KinectSDK, in order for Kinect to function properly, add the following directories to the VS project:
- C/C++ - General - Additional Include Directories -> $(KINECTSDK10_DIR)\inc
- Linker - General - Additional Library Directories -> $(KINECTSDK10_DIR)\lib\x86
- Linker - Input - Additional Dependencies -> Kinect10.lib

[KinectV2]
Please note that KinectSDK v2 only works with Windows 8, also being built with VisualStudio 2012. The project used is in KinectV1>apps>myApps>kinectBlender2 folder. To have an easier time with the graphical aspect of the code, we used OpenFrameworks with the KinectCommonBridge add-on helping getting values from Kinect. After installing the KinectSDK, in order for Kinect to function properly, add the following directories to the VS project:
- C/C++ - General - Additional Include Directories -> $(KINECTSDK20_DIR)\inc
- Linker - General - Additional Library Directories -> $(KINECTSDK20_DIR)\lib\x86
- Linker - Input - Additional Dependencies -> Kinect20.lib

[BlenderKinect]
The files in the BlenderKinect folder use Blender v2.68 as an simulation environment for the Kinect captured movement, using python scripts as support for the communication between the two tools. Run the "base_skele_rig_tpose_final.exe" file to start the simulation.
