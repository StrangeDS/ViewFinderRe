// Copyright 2026, StrangeDS. All Rights Reserved.

using UnrealBuildTool;

public class ViewFinderRe : ModuleRules
{
	public ViewFinderRe(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);


		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "CoreUObject",
                "Engine",
                "VFStepsRecorder",

				"VFCommon",
				"VFInteract",
				"ViewFinderCore",
			}
			);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"EnhancedInput",
				"UMG",
                "RenderCore",
                "VFGeometry",
                "GeometryFramework",

				"VFPhotoCommon",
				"VFPhotoCatcher",
			}
			);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
