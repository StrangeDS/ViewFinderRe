#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VFStandInInterface.generated.h"

UINTERFACE(MinimalAPI)
class UVFStandInInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 在ViewFinder中, Pawn被拍到会显示扭曲身影.
 * 目前实现是: Helper中定义bReplacedWithStandIn, 并给定用于替换的(实现接口的)的替身AActor.
 * 拍照时生成替身, Pawn则被忽略, 不参与后续的处理.
 * 后续的操作都应用在替身上.
 * 例子可见VFPawnStandIn.
 */
class VIEWFINDERCORE_API IVFStandInInterface
{
	GENERATED_BODY()

public:
	// 此函数最好在SpawnActorDeferred中处理, 以便能在BeginPlay中访问GetOriginalActor()
	// 参照UVFFunctions::ReplaceWithStandIn()
	// 使用SetOriginalActor_Implementation()去触发相关操作也是可行的
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	void SetOriginalActor(AActor *Original);
	virtual void SetOriginalActor_Implementation(AActor *Original);

	// BeginPlay中可能访问可能为空值, 原因是没有使用SpawnActorDeferred()去调用SetOriginalActor()
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	AActor *GetOriginalActor();
	virtual AActor *GetOriginalActor_Implementation();

	// 建议返回(一个无实际网格的)VFDMComp, 这样在后续处理中就不会制成对应的VFDMComp.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewFinder")
	UPrimitiveComponent *GetPrimitiveComp();
	virtual UPrimitiveComponent *GetPrimitiveComp_Implementation();

public:
	// 被替换的Actor, 通过GetOriginalActor()访问
	TObjectPtr<AActor> OriginalActor;
};
