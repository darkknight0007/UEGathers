// Fill out your copyright notice in the Description page of Project Settings.

#include "GrabSystemComponent.h"
#include "DrawDebugHelpers.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values for this component's properties
UGrabSystemComponent::UGrabSystemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	PhysicsHandle = nullptr;
	PickedUp = false;
	GrabArmLength = 0;
}

// Called when the game starts
void UGrabSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	auto existPhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (existPhysicsHandle)
		PhysicsHandle = existPhysicsHandle;
	else
	{
		if (!PhysicsHandle)
		{
			PhysicsHandle = NewObject<UPhysicsHandleComponent>(GetOwner(),UPhysicsHandleComponent::StaticClass(),TEXT("PhysicsHandle"));
			check(PhysicsHandle);
			PhysicsHandle->OnComponentCreated();
			PhysicsHandle->RegisterComponent();
			GetOwner()->AddInstanceComponent(PhysicsHandle);
		}
	}
}

// Called every frame
void UGrabSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (PickedUp && PhysicsHandle)
	{
		AActor *actor = GetOwner();
		FVector lineStart = actor->GetActorLocation();
		FVector forwardVector = actor->GetActorForwardVector();
		FVector lineEnd = lineStart + forwardVector * GrabArmLength * 0.5;

		PhysicsHandle->SetTargetLocation(lineEnd);

		UWorld *world = actor->GetWorld();
		DrawDebugLine(world, lineStart, lineEnd, FColor(255, 255, 255), false, 5, 0, 5);
	}
}

void UGrabSystemComponent::Grab()
{
	if (PickedUp)
	{
		ReleaseGrab();
		return;
	}

	AActor *actor = GetOwner();
	FVector lineStart = actor->GetActorLocation();
	FVector forwardVector = actor->GetActorForwardVector();
	FVector lineEnd = lineStart + forwardVector * GrabArmLength;

	UWorld *world = actor->GetWorld();
	DrawDebugLine(world, lineStart, lineEnd, FColor(255, 0, 0), false, 5, 0, 5);

	FCollisionQueryParams trace_params;
	trace_params.AddIgnoredActor(actor);

	FHitResult hitResult;
	bool hit = world->LineTraceSingleByChannel(hitResult, lineStart, lineEnd, ECC_Visibility, trace_params);
	if (hit)
	{
		UPrimitiveComponent *hitComponent = hitResult.GetComponent();
		if (hitComponent->ComponentHasTag(FName(TEXT("Grabable"))))
		{
			if (PhysicsHandle)
			{
				PickedUp = true;
				PhysicsHandle->GrabComponentAtLocationWithRotation(hitComponent, FName(), hitComponent->GetComponentLocation(), FRotator(0, 0, 0));
			}
		}
	}
}

void UGrabSystemComponent::ReleaseGrab()
{
	if (PickedUp && PhysicsHandle)
	{
		PhysicsHandle->ReleaseComponent();
		PickedUp = false;
	}
}