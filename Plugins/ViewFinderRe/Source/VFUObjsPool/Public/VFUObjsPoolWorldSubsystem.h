// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VFUObjsPoolWorldSubsystem.generated.h"

class IVFPoolableInterface;

USTRUCT(BlueprintType)
struct FVFPoolableUObjs
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TScriptInterface<IVFPoolableInterface>> Objs;
};

// requires implementation of VFPoolableInterface
UCLASS(ClassGroup = (ViewFinder))
class VFUOBJSPOOL_API UVFUObjsPoolWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	UVFUObjsPoolWorldSubsystem();

public:
	virtual void Initialize(FSubsystemCollectionBase &Collection) override;

	virtual void Deinitialize() override;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder|DMCompPool")
	UObject *GetOrCreateAsUObject(
		UObject *Outer,
		const TSubclassOf<UObject> &ObjClass);

	template <typename T>
	T *GetOrCreate(
		UObject *Outer,
		const TSubclassOf<UObject> &ObjClass);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder|DMCompPool")
	bool Return(UObject *Obj);

	// Nums of various classes for Pre-generated
	UFUNCTION(BlueprintCallable, Category = "ViewFinder|DMCompPool")
	void PreparePools(const TMap<TSubclassOf<UObject>, int> &PrepareNum);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder|DMCompPool")
	void ClearPools(bool bForceGarbage = true);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder|DMCompPool")
	TMap<TSubclassOf<UObject>, FVFPoolableUObjs> PoolsOfAvailable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder|DMCompPool")
	TMap<TSubclassOf<UObject>, FVFPoolableUObjs> PoolsOfAll;
};

template <typename T>
inline T *UVFUObjsPoolWorldSubsystem::GetOrCreate(UObject *Outer, const TSubclassOf<UObject> &ObjClass)
{
	auto Obj = GetOrCreateAsUObject(Outer, ObjClass);
	return Cast<T>(Obj);
}