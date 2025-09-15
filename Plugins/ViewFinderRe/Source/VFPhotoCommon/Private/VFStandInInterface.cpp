#include "VFStandInInterface.h"

#include "VFHelperComponent.h"

void IVFStandInInterface::SetOriginalActor_Implementation(AActor *Original)
{
    auto Actor = Cast<AActor>(_getUObject());
    auto Helper = Actor->GetComponentByClass<UVFHelperComponent>();
    Helper->ActorStandInFor = Original;
}

AActor *IVFStandInInterface::GetOriginalActor_Implementation()
{
    auto Actor = Cast<AActor>(_getUObject());
    auto Helper = Actor->GetComponentByClass<UVFHelperComponent>();
    return Helper->ActorStandInFor;
}

UPrimitiveComponent *IVFStandInInterface::GetPrimitiveComp_Implementation()
{
    // 需要被重写. 不能返回空指针.
    // 照片中应当有对应的显示, 无显示那为什么不直接设置bCanBeTakenInPhoto为false?
    unimplemented();
    return nullptr;
}