// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AzureKinectHelper.h"
#include "AzureKinectDevice.h"
#include "AzureKinectManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(AzureKinectLog, Log, All);

/**
 * A singleton class used in Blueprints to control and get info from the connected 
 * azure kinect device(s).
 */
UCLASS(BlueprintType, Category = "Azure Kinect")
class AZUREKINECTUNREAL_API UAzureKinectManager : public UObject
{
	GENERATED_BODY()
	
public:
	/** Ctor */
	UAzureKinectManager();
	/** Destructor */
	~UAzureKinectManager();

	/**
	 * Initializes the azure kinect device with the given details and if the initialization is 
	 * successful, then adds it to the TMap.
	 *
	 * @param DeviceId The device id of the Azure Kinect Device to initialize.
	 * @param DepthMode The default is set to NFOV_UNBINNED. For body tracking it should be NFOV_UNBINNED or WFOV_BINNED.
	 * @param TimeOutInMilliSecs Default is Zero (Non-blocking). Set it to -1 (K4A_WAIT_INFINITE) for Blocking call.
	 * @param GpuDeviceId Gpu device Id to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "Azure Kinect", meta = (DisplayName = "Init Azure Kinect"))
	static void InitDevice(int32 DeviceId = 0, EKinectDepthMode DepthMode = EKinectDepthMode::NFOV_UNBINNED, int32 TimeOutInMilliSecs = 0, int32 GpuDeviceId = 0);

	/** Shuts down the azure kinect device with the given device id. */
	UFUNCTION(BlueprintCallable, Category = "Azure Kinect", meta = (DisplayName = "Shutdown Azure Kinect"))
	static void ShutdownDevice(int32 DeviceId = 0);

	/** Check to see if the azure kinect device with the given device id is initialized. */
	UFUNCTION(BlueprintPure, Category = "Azure Kinect", meta = (DisplayName = "Is Azure Kinect Device Initialized"))
	static bool IsDeviceInitialized(int32 DeviceId = 0);

	static AzureKinectDevice* GetDevice(int32 DeviceId);

	/** Retrieves the bodies tracked and non-tracked by the azure kinect device with the given device id. */
	UFUNCTION(BlueprintPure, Category = "Azure Kinect", meta = (DisplayName = "Get Bodies for Device"))
	static TArray<UAzureKinectBody*> GetBodies(int32 DeviceId = 0);

	/** Retrieves the azure kinect body joint count. */
	UFUNCTION(BlueprintPure, Category = "Azure Kinect", meta = (DisplayName = "Get Kinect Body Joint Count"))
	static int32 GetJointCount();

	UFUNCTION(BlueprintCallable, Category = "Azure Kinect", meta = (DisplayName = "Toggle Kinect Device Logging"))
	static void ToggleDeviceLogging(int32 DeviceId, bool ShouldShowLogs, bool ShouldShowOnScreenMsgs);

private:
	/** A singleton instance. */
	static UAzureKinectManager *Instance;
	
	/**
	 * A map (key value pair) of connected azure kinect devices with their
	 * ids as keys.
	 *
	 * @note This is mainly useful when more than one azure kinect device is connected.
	 */
	TMap<int32, AzureKinectDevice*> KinectDevicesById;

	/** Iterates through all the devices and shuts them down. */
	void ShutdownAllDevices();
};
