#pragma once

#include "CoreMinimal.h"

VIEWFINDERCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogViewFinderRe, Log, All);

#define VF_LOG(Verbosity, Format, ...) UE_LOG(LogViewFinderRe, Verbosity,  Format, ##__VA_ARGS__)
