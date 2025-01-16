#include "VFStandInInterface.h"

#include "VFHelperComponent.h"

void IVFStandInInterface::SetSourceActor_Implementation(AActor *Source)
{
    StandInSourceActor = Source;
}

UPrimitiveComponent *IVFStandInInterface::GetPrimitiveComp_Implementation()
{
    // 需要被重写. 不能返回空指针.
    // 照片中应当有对应的显示, 无显示那为什么不直接设置bCanBeTakenInPhoto为false?
    check(0);
    return nullptr;
}