#pragma once

#include "CoreMinimal.h"
#include "VFPhotoCatcher_PickUp.h"
#include "VFPhotoCatcher_VirtualRender.generated.h"

class UMaterialParameterCollection;
class UMaterialParameterCollectionInstance;

/*
用于展示: 复刻星际拓荒太空站灯笼的后处理
向材质参数集更新参数
丢弃后加上后处理, 然后由Deactivate关闭
*/
UCLASS(Blueprintable, ClassGroup = (ViewFinder))
class VIEWFINDERRE_API AVFPhotoCatcher_VirtualRender : public AVFPhotoCatcher_PickUp
{
	GENERATED_BODY()

public:
	AVFPhotoCatcher_VirtualRender();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

public:
	virtual void DropDown_Implementation() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UMaterialParameterCollection> MPC;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	float RenderRadius = 1000.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	TObjectPtr<UMaterialParameterCollectionInstance> MPCI;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewFinder")
	bool bAddPPAfterDropDown = true;
};