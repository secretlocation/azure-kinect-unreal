// Fill out your copyright notice in the Description page of Project Settings.


#include "AzureKinectManager.h"

DEFINE_LOG_CATEGORY(AzureKinectLog);

UAzureKinectManager *UAzureKinectManager::Instance = nullptr;

UAzureKinectManager::UAzureKinectManager()
{
}

UAzureKinectManager::~UAzureKinectManager()
{
	ShutdownAllDevices();

	RemoveFromRoot();
	Instance = nullptr;
}

void UAzureKinectManager::InitDevice(int32 DeviceId, EKinectDepthMode DepthMode, int32 TimeOutInMilliSecs, int32 GpuDeviceId)
{
	if (!Instance)
	{
		Instance = NewObject<UAzureKinectManager>();
		if (!Instance)
		{
			UE_LOG(AzureKinectLog, Error, TEXT("Could not create an instance for UAzureKinectManager"));
			return;
		}

		Instance->AddToRoot();
	}

	if (Instance->KinectDevicesById.Contains(DeviceId))
	{
		UE_LOG(AzureKinectLog, Error, TEXT("Kinect device (id : %d) already initialized"), DeviceId);
		return;
	}

	AzureKinectDevice *KinectDevice = new AzureKinectDevice(DeviceId, TimeOutInMilliSecs);
	bool bIsInitialized = KinectDevice->Initialize((k4a_depth_mode_t)DepthMode, GpuDeviceId);
	if (bIsInitialized)
	{
		Instance->KinectDevicesById.Add(DeviceId, KinectDevice);
		UE_LOG(AzureKinectLog, Warning, TEXT("Kinect device (id : %d) added to TMap : Count : %d"), DeviceId, Instance->KinectDevicesById.Num());
	}
	else
	{
		UE_LOG(AzureKinectLog, Error, TEXT("Kinect device (id : %d) initialization Failed, so cleaning up."), DeviceId);
		KinectDevice->Shutdown();
	}
}

void UAzureKinectManager::ShutdownDevice(int32 DeviceId)
{
	if (!Instance)
	{
		UE_LOG(AzureKinectLog, Error, TEXT("UAzureKinectManager Instance is nullptr"));
		return;
	}

	AzureKinectDevice **Device = Instance->KinectDevicesById.Find(DeviceId);
	if (!Device || !(*Device))
	{
		UE_LOG(AzureKinectLog, Error, TEXT("Kinect device (id : %d) was not initialized to be shutdown"), DeviceId);
		return;
	}

	(*Device)->Shutdown();
	Instance->KinectDevicesById.Remove(DeviceId);
	UE_LOG(AzureKinectLog, Warning, TEXT("Kinect device (id : %d) was shutdown. TMap count : %d"), DeviceId, Instance->KinectDevicesById.Num());
}

bool UAzureKinectManager::IsDeviceInitialized(int32 DeviceId)
{
	AzureKinectDevice *Device = GetDevice(DeviceId);
	if (!Device)
	{
		return false;
	}

	return Device->IsInitialized();
}

void UAzureKinectManager::ShutdownAllDevices()
{
	for (TPair<int32, AzureKinectDevice*> kvp : KinectDevicesById)
	{
		if (kvp.Value)
		{
			kvp.Value->Shutdown();
		}
	}

	KinectDevicesById.Empty();
}

AzureKinectDevice *UAzureKinectManager::GetDevice(int32 DeviceId)
{
	if (!Instance)
	{
		UE_LOG(AzureKinectLog, Error, TEXT("UAzureKinectManager Instance is nullptr"));
		return nullptr;
	}

	AzureKinectDevice **Device = Instance->KinectDevicesById.Find(DeviceId);
	if (!Device || !(*Device))
	{
		UE_LOG(AzureKinectLog, Error, TEXT("Kinect device (id : %d) was not found"), DeviceId);
		return nullptr;
	}

	return (*Device);
}

TArray<UAzureKinectBody*> UAzureKinectManager::GetBodies(int32 DeviceId)
{
	AzureKinectDevice *Device = GetDevice(DeviceId);
	if (!Device)
	{
		UE_LOG(AzureKinectLog, Warning, TEXT("Returning empty TArray of UAzureKinectBody"));
		return TArray<UAzureKinectBody*>();
	}

	return Device->GetBodies();
}

int32 UAzureKinectManager::GetJointCount()
{
	return static_cast<int32>(JointCount);
}

void UAzureKinectManager::ToggleDeviceLogging(int32 DeviceId, bool ShouldShowLogs, bool ShouldShowOnScreenMsgs)
{
	AzureKinectDevice *Device = GetDevice(DeviceId);
	if (Device)
	{
		Device->ToggleShowLogsAndOnScreenMsgs(ShouldShowLogs, ShouldShowOnScreenMsgs);
	}
}
