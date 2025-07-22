#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EVFHelperGetting : uint8
{
	ByVFHelperInterface,
	ByGetComponentByClass,
};