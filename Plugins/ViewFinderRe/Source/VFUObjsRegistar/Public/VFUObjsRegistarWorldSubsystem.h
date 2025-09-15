#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VFUObjsRegistarWorldSubsystem.generated.h"

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
一个用于UObject注册的, 通用的世界子系统.
内部实现为TMap<UClass*, TArray<UObject*>>,
注意: 不接管生命周期, 要求手动反注册.
不保证顺序， 使用RemoveSwap
*/
UCLASS(ClassGroup = (ViewFinder))
class VFUOBJSREGISTAR_API UVFUObjsRegistarWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	UVFUObjsRegistarWorldSubsystem();

public:
	virtual void Initialize(FSubsystemCollectionBase &Collection) override;

	virtual void Deinitialize() override;

public:
	UFUNCTION(BlueprintCallable, Category = "ViewFinder|UObjsRegistar")
	bool Register(UObject *Obj,
				  const FString &Channel = TEXT(""),
				  TSubclassOf<UObject> ObjClass = nullptr);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder|UObjsRegistar")
	bool Unregister(UObject *Obj,
					const FString &Channel = TEXT(""),
					TSubclassOf<UObject> ObjClass = nullptr);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder|UObjsRegistar")
	void ClearInvalidInChannel(const FString &Channel = TEXT(""),
							   bool bForceGarbage = true);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder|UObjsRegistar")
	void ClearAllInChannel(const FString &Channel = TEXT(""),
						   bool bForceGarbage = true);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder|UObjsRegistar")
	void ClearAll(bool bForceGarbage = true);

	UFUNCTION(BlueprintCallable, Category = "ViewFinder|UObjsRegistar",
			  meta = (DisplayName = "GetUObjs"))
	TArray<UObject *> K2_GetUObjs(const FString &Channel = TEXT(""),
								  TSubclassOf<UObject> ObjClass = nullptr);
	template <typename T>
	TArray<T *> GetUObjs(const FString &Channel = TEXT(""),
						 TSubclassOf<UObject> ObjClass = nullptr);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder|DMCompPool")
	TMap<FString, FVFRegisterChannel> ChannelsRegisted;
};

template <typename T>
inline TArray<T *> UVFUObjsRegistarWorldSubsystem::GetUObjs(
	const FString &Channel,
	TSubclassOf<UObject> ObjClass)
{
	TArray<T *> Objs;
	if (!ChannelsRegisted.Contains(Channel))
		return Objs;

	auto &Table = ChannelsRegisted[Channel].Table;
	if (!Table.Contains(ObjClass))
		return Objs;

	Objs.Reserve(Table[ObjClass].Objs.Num());
	for (auto &Obj : Table[ObjClass].Objs)
	{
		Objs.Emplace(Cast<T>(Obj));
	}
	return Objs;
}
