# Azure Kinect Unreal Plugin

This Unreal (4.24) project contains the Azure Kinect Plugin and sample map to test the Plugin. The Plugin integrates the **Azure Kinect Sensor SDK (K4A)** and **Azure Kinect Body Tracking SDK (K4ABT)**. It captures and processes the Body Tracking data and maps it onto the Unreal Mannequin skeletal mesh. The plugin supports tracking up to a maximum of 10 bodies.

## SDKs

- [Azure Kinect SDK 1.4.0](https://docs.microsoft.com/en-us/azure/kinect-dk/sensor-sdk-download)
- [Azure Kinect Body Tracking SDK 1.0.1](https://docs.microsoft.com/en-us/azure/kinect-dk/body-sdk-download)

## To get up and running

 - Add the `<Azure Kinect Body Tracking SDK installation>/tools` folder to the `Path` variable in `Environment Variables` for User or System.

The [Notes](README.md#notes) section below has more FYI.


## Plugin Files

 - [AzureKinectManager](Plugins/AzureKinectUnreal/Source/AzureKinectUnreal/Public/AzureKinectManager.h) : A singleton class used in Blueprints to control and get info from the connected Azure Kinect Device(s).
 - [AzureKinectDevice](Plugins/AzureKinectUnreal/Source/AzureKinectUnreal/Public/AzureKinectDevice.h) : A representation of the Native Azure Kinect Device that is used to call the connected device's (K4A & K4ABT) API for starting & stopping camera sensors, creating & destroying the body tracker, capturing the body frame etc.
 - [AzureKinectThread](Plugins/AzureKinectUnreal/Source/AzureKinectUnreal/Public/AzureKinectThread.h) : A thread that is used to capture the Azure Kinect device's body frame data.
 - [AzureKinectBody](Plugins/AzureKinectUnreal/Source/AzureKinectUnreal/Public/AzureKinectBody.h) : A representation of the Native Azure Kinect Body that contains Joints.
	- [AzureKinectJoint](Plugins/AzureKinectUnreal/Source/AzureKinectUnreal/Public/AzureKinectBody.h#L18) : A representation of the Native Azure Kinect Body Skeleton joint. This stores the kinect skeleton joint position and orientation data by converting them from Kinect camera co-ordinate system to Unreal co-ordinate system and maps them to the Unreal mannequin.
 - [AzureKinectHelper](Plugins/AzureKinectUnreal/Source/AzureKinectUnreal/Public/AzureKinectHelper.h) : A helper file that contains the duplicated Azure Kinect enums for Blueprint access and constant values for maximum bodies tracked and the joints count. 

### Sample Files for Testing

Blueprints/
 - BP_AnimMannequin : The animation blueprint that applies the Orientation of the joints from the plugin to the UE4 mannequin.
 - BP_MannequinActor : A test blueprint actor with 1 UE4 mannequin skeletal mesh that uses the above animation blueprint.
 - BP_TenMannequinActors : A test blueprint actor with 10 UE4 mannequin skeletal meshes that use the above animation blueprint.
 - BP_TestActor : A test blueprint actor with 26 spheres where each one represents a joint in Azure kinect body skeleton.The spheres use the Position data from the plugin for their corresponding joints that they each represent.

Maps/
 - Sample : This map contains the BP_TestActor and BP_MannequinActor already placed in the level for testing.

## Notes

The following dlls and their dependency files are needed for the Azure Kinect API to work.
Since Unreal will search in `Engine/Binaries/Win64`, `Project/Binaries/Win64`, `Engine/Plugins/.../Binaries/Win64` and `Project/Plugins/.../Binaries/Win64` folders to load the dependent dll files, the following are put in the plugin binaries folder. This will ensure that the project will compile and open in UE4 editor.
```
Plugins/AzureKinectUnreal/Binaries/Win64/
 - k4a.dll
 - k4abt.dll
 - depthengine_2_0.dll
 - dnn_model_2_0.onnx
```
_**Important Note 1:** In order to run the project in Unreal Editor and call the Azure Kinect API without issues, one of the following can be done._
_[For Reference check Azure Kinect SDK Issue 709](https://github.com/microsoft/Azure-Kinect-Sensor-SDK/issues/709#issuecomment-545132613)_

 - _1. Add the `<Azure Kinect Body Tracking SDK installation>/tools` folder to the `Path` variable in `Environment Variables` for User or System._

_(OR)_

 - _2. The below mentioned dependent files can be copied to the `Unreal installation folder` where `UE4Editor.exe` resides. Eg: `C:\Program Files\Epic Games\UE_4.24\Engine\Binaries\Win64`. The dependent dll files can be found in `<Azure Kinect Body Tracking SDK installation>/tools` folder._
```
<unreal_installation_folder>/Engine/Binaries/Win64/
 - onnxruntime.dll
 - cudnn64_7.dll
 - cudart64_100.dll
 - cublas64_100.dll
```

_**Important Note 2:** It is **No** longer required to place the `dnn_model_2_0.onnx` file in the `Unreal Installation folder`. This file is already placed in the `Plugins/AzureKinectUnreal/Binaries/Win64` and Unreal checks this folder and loads the file. This file is needed for the Body Tracking to work._


## Disclaimer

 - The plugin supports tracking with one connected Azure Kinect device for tracking one body.
 
 - The plugin has **not** been thoroughly tested with one connected Azure Kinect device for tracking multiple bodies although the plugin supports it.

 - The plugin has **not at all been tested** with multiple Azure Kinect Devices connected to one PC.


## Troubleshooting

Kindly check the logs (Project, k4a.log, k4abt.log) to see if there is anything regarding the issue you are having trouble with and raise a [Github Issue](https://github.com/secretlocation/azure-kinect-unreal/issues) with the information found in logs.

#### The Project doesn't open.

Delete the `Project/Binaries` and only the `UE4` dlls in the `Plugin/AzureKinectUnreal/Binaries` folder and try again. If it still doesn't open, try building the Project in Visual Studio and check if there are any issues.

Kindly check the project log file to see what the issue is. It is located in `<project>/Saved/Logs/` folder.

Also, check if `Plugin/AzureKinectUnreal/Binaries` contains the dependent dll files `k4a.dll`, `k4abt.dll` and `depthengine_2_0.dll`. 

#### Unreal crashes when PIE (Play in Editor).

Kindly check the following.
 - `Path` Environment variable has the `<Azure Kinect Body Tracking SDK installation>/tools` folder added.
 - The `Plugins/AzureKinectUnreal/Binaries/Win64` contains the `k4a.dll`, `k4abt.dll`, `depthengine_2_0.dll` and `dnn_model_2_0.onnx` files.
 - The version of Azure Kinect Sensor SDK and Azure Kinect Body Tracking SDK installed in your PC is the same as mentioned above under [SDKs](README.md#sdks) section.

#### Pressing play doesn't show anything.

Kindly check the `Output Log` window and if the issue is something related to the `Device` then check the `k4a.log` and if it is related to the `Body Tracking` then check the `k4abt.log`. These log files can be found in `<unreal_installation_folder>/Engine/Binaries/Win64/`.

If these log files are not present then kindly check [here](https://docs.microsoft.com/en-us/azure/kinect-dk/troubleshooting#collecting-logs) in order to enable the logs.
