// Copyright 2026, StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VFUObjsRegistrarWorldSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FVFUObjs
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<UObject>> Objs;
};

USTRUCT(BlueprintType)
struct FVFRegisterChannel
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<TSubclassOf<UObject>, FVFUObjs> Table;
};

/*
A generic World Subsystem for UObject registration.
Internally implemented as TMap<UClass*, TArray<UObject*>>.
Note: Does not manage lifecycle; manual unregistration is required.
Order is not guaranteed; uses RemoveSwap.
*/
UCLASS(ClassGroup = (ViewFinder))
class VFUOBJSREGISTRAR_API UVFUObjsRegistrarWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	UVFUObjsRegistrarWorldSubsystem();

public:
	virtual void Initialize(FSubsystemCollectionBase &Collection) override;

	virtual void Deinitialize() override;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder|UObjsRegistrar",
			  meta = (DefaultToSelf = "Obj"))
	bool Register(UObject *Obj,
				  const FString &Channel = TEXT(""),
				  TSubclassOf<UObject> ObjClass = nullptr);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder|UObjsRegistrar",
			  meta = (DefaultToSelf = "Obj"))
	bool Unregister(UObject *Obj,
					const FString &Channel = TEXT(""),
					TSubclassOf<UObject> ObjClass = nullptr);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder|UObjsRegistrar")
	void ClearInvalidInChannel(const FString &Channel = TEXT(""),
							   bool bForceGarbage = true);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder|UObjsRegistrar")
	void ClearAllInChannel(const FString &Channel = TEXT(""),
						   bool bForceGarbage = true);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder|UObjsRegistrar")
	void ClearAll(bool bForceGarbage = true);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder|UObjsRegistrar",
			  meta = (DisplayName = "GetUObjs"))
	TArray<UObject *> K2_GetUObjs(const FString &Channel = TEXT(""),
								  TSubclassOf<UObject> ObjClass = nullptr);
	template <typename T>
	TArray<T *> GetUObjs(const FString &Channel = TEXT(""),
						 TSubclassOf<UObject> ObjClass = nullptr);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder|DMCompPool")
	TMap<FString, FVFRegisterChannel> ChannelsRegistered;
};

template <typename T>
inline TArray<T *> UVFUObjsRegistrarWorldSubsystem::GetUObjs(
	const FString &Channel,
	TSubclassOf<UObject> ObjClass)
{
	TArray<T *> Objs;
	if (!ChannelsRegistered.Contains(Channel))
		return Objs;

	auto &Table = ChannelsRegistered[Channel].Table;
	if (!Table.Contains(ObjClass))
		return Objs;

	Objs.Reserve(Table[ObjClass].Objs.Num());
	for (auto &Obj : Table[ObjClass].Objs)
	{
		Objs.Emplace(Cast<T>(Obj));
	}
	return Objs;
}