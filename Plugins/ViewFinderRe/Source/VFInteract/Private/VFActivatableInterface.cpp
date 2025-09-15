#include "VFActivatableInterface.h"

void IVFActivatableInterface::Activate_Implementation()
{
    unimplemented();
}

void IVFActivatableInterface::Deactivate_Implementation()
{
    unimplemented();
}

bool IVFActivatableInterface::TryActivate_Implementation()
{
    auto Object = Cast<UObject>(this);
    Execute_Activate(Object);
    return Execute_IsActive(Object);
}

bool IVFActivatableInterface::TryDeactivate_Implementation()
{
    auto Object = Cast<UObject>(this);
    Execute_Deactivate(Object);
    return !Execute_IsActive(Object);
}

bool IVFActivatableInterface::CanActivate_Implementation()
{
    unimplemented();
    return false;
}

bool IVFActivatableInterface::IsActive_Implementation()
{
    unimplemented();
    return false;
}