// Fill out your copyright notice in the Description page of Project Settings.


#include "AzureKinectDevice.h"
#include "Containers/Ticker.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(AzureKinectDeviceLog);

AzureKinectDevice::AzureKinectDevice() :
	AzureKinectDevice(0, 0)
{
}

AzureKinectDevice::AzureKinectDevice(int32 Id, int32 TimeOut) :
	NativeKinectDevice(nullptr),
	DeviceId(Id),
	NativeBodyTracker(nullptr),
	TimeOutInMilliSecs(TimeOut),
	Thread(nullptr),
	bIsInitialized(false),
	bShowAllLogs(false),
	bShowOnScreenMsgs(false)
{
	TimeOutInMilliSecsConverted = std::chrono::milliseconds(TimeOutInMilliSecs);
}

AzureKinectDevice::~AzureKinectDevice()
{
	Shutdown();
}

bool AzureKinectDevice::Initialize(k4a_depth_mode_t DepthMode, int32_t GpuDeviceId)
{
	try
	{
		k4abt_tracker_configuration_t tracker_config = K4ABT_TRACKER_CONFIG_DEFAULT;
		tracker_config.processing_mode = K4ABT_TRACKER_PROCESSING_MODE_GPU;
		tracker_config.gpu_device_id = GpuDeviceId;

		// Open the Azure Kinect Device
		NativeKinectDevice = k4a::device::open(DeviceId);

		// Start the Camera and make sure the Depth Camera is Enabled
		k4a_device_configuration_t deviceConfig = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
		deviceConfig.depth_mode = DepthMode;
		deviceConfig.color_resolution = K4A_COLOR_RESOLUTION_OFF;

		NativeKinectDevice.start_cameras(&deviceConfig);

		// Get the device calibration
		k4a::calibration sensorCalibration = NativeKinectDevice.get_calibration(deviceConfig.depth_mode, deviceConfig.color_resolution);

		// Create the Body tracker using the calibration
		NativeBodyTracker = k4abt::tracker::create(sensorCalibration, tracker_config);
	}
	catch (k4a::error initError)
	{
		FString initErrorStr(initError.what());
		UE_LOG(AzureKinectDeviceLog, Error, TEXT("Kinect device (id : %d) - Initialization Failed with error : %s"), DeviceId, *initErrorStr);
		return false;
	}

	bIsInitialized = true;

	InitializeBodies();
	StartKinectThread();

	TickHandle = FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &AzureKinectDevice::OnTick), TickInterval);

	return true;
}

void AzureKinectDevice::Shutdown()
{
	bIsInitialized = false;

	if (Thread)
	{
		Thread->Shutdown();
		Thread = nullptr;
	}

	FTicker::GetCoreTicker().RemoveTicker(TickHandle);

	for (UAzureKinectBody *body : Bodies)
	{
		body->RemoveFromRoot();
	}
	Bodies.Empty();

	if (NativeBodyTracker)
	{
		NativeBodyTracker.shutdown();
		NativeBodyTracker.destroy();
		NativeBodyTracker = nullptr;
		UE_LOG(AzureKinectDeviceLog, Warning, TEXT("BodyTracker Shutdown and Destroyed"));
	}

	if (NativeKinectDevice)
	{
		NativeKinectDevice.stop_cameras();
		NativeKinectDevice.close();
		NativeKinectDevice = nullptr;
		UE_LOG(AzureKinectDeviceLog, Warning, TEXT("KinectDevice Camera is Stopped and Closed"));
	}
}

void AzureKinectDevice::CaptureBodyTrackingFrame()
{
	if (!bIsInitialized)
	{
		UE_LOG(AzureKinectDeviceLog, Error, TEXT("Kinect device for capturing body tracking frame is Not Initialized!"));
		return;
	}

	if (!NativeKinectDevice)
	{
		UE_LOG(AzureKinectDeviceLog, Error, TEXT("Kinect device for capturing body tracking frame is Invalid!"));
		return;
	}

	if (!NativeBodyTracker)
	{
		UE_LOG(AzureKinectDeviceLog, Error, TEXT("Body Tracker for capturing body tracking frame is Invalid!"));
		return;
	}

	uint32_t i = 0;
	try
	{
		// Capture a depth frame
		k4a::capture sensorCapture = nullptr;
		if (!NativeKinectDevice.get_capture(&sensorCapture, TimeOutInMilliSecsConverted))
		{
			UE_CLOG(bShowAllLogs, AzureKinectDeviceLog, Error, TEXT("Kinect device get capture Timed Out!"));
			return;
		}

		// Enqueue the capture
		if (!NativeBodyTracker.enqueue_capture(sensorCapture, TimeOutInMilliSecsConverted))
		{
			UE_CLOG(bShowAllLogs, AzureKinectDeviceLog, Error, TEXT("Adding capture to the Tracker process queue Timed Out!"));
			return;
		}

		k4abt::frame bodyFrame = nullptr;
		if (!NativeBodyTracker.pop_result(&bodyFrame, TimeOutInMilliSecsConverted))
		{
			UE_CLOG(bShowAllLogs, AzureKinectDeviceLog, Error, TEXT("Tracker pop body frame result Timed Out!"));
			return;
		}

		// Successfully popped the body tracking result
		uint32_t numBodies = bodyFrame.get_num_bodies();
		if (GEngine && bShowOnScreenMsgs)
		{
			GEngine->AddOnScreenDebugMessage(0, 5.0f, (numBodies > 0 ? FColor::Cyan : FColor::Red), FString::Printf(TEXT("%zu bodies are detected"), numBodies));
		}

		// Get the skeleton data for the tracked bodies
		for (; i < FMath::Min(numBodies, MaxBodies); i++)
		{
			k4abt_body_t body;
			try
			{
				bodyFrame.get_body_skeleton(i, body.skeleton);
			}
			catch (k4a::error skeletonError)
			{
				UE_CLOG(bShowAllLogs, AzureKinectDeviceLog, Error, TEXT("Get Body Skeleton Failed!"));
				Bodies[i]->bIsTracked = false;
				continue;
			}

			body.id = bodyFrame.get_body_id(i);

			if (GEngine && bShowOnScreenMsgs)
			{
				GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Cyan, FString::Printf(TEXT("  Body Id : %d"), body.id));
			}

			Bodies[i]->NativeBody = body;
			Bodies[i]->bIsTracked = true;
		}
	}
	catch (k4a::error captureError)
	{
		FString captureErrorStr(captureError.what());
		UE_CLOG(bShowAllLogs, AzureKinectDeviceLog, Error, TEXT("Kinect device body tracking capture Failed with error : %s"), *captureErrorStr);
	}

	// Set all the remaining Bodies to be Not tracked
	for (uint32 j = i; j < MaxBodies; j++)
	{
		Bodies[j]->bIsTracked = false;
	}
}

int32 AzureKinectDevice::GetTimeOutInMilliSecs() const
{
	return TimeOutInMilliSecs;
}

TArray<UAzureKinectBody*> AzureKinectDevice::GetBodies() const
{
	return Bodies;
}

bool AzureKinectDevice::IsInitialized() const
{
	return bIsInitialized;
}

void AzureKinectDevice::InitializeBodies()
{
	// reset any allocated memory
	Bodies.Empty(MaxBodies);
	Bodies.SetNum(MaxBodies);
	Bodies.Shrink();

	for (uint32 i = 0; i < MaxBodies; i++)
	{
		Bodies[i] = NewObject<UAzureKinectBody>();
		Bodies[i]->AddToRoot();
	}
}

void AzureKinectDevice::StartKinectThread()
{
	Thread = FAzureKinectThread::InitPolling(this);
}

bool AzureKinectDevice::OnTick(float DeltaTime)
{
	if (bIsInitialized)
	{
		for (UAzureKinectBody *Body : Bodies)
		{
			Body->UpdateBodyWithKinectInfo();
		}
	}

	return true;
}

void AzureKinectDevice::ToggleShowLogsAndOnScreenMsgs(bool ShouldShowLogs, bool ShouldShowOnScreenMsgs)
{
	bShowAllLogs = ShouldShowLogs;
	bShowOnScreenMsgs = ShouldShowOnScreenMsgs;
}
