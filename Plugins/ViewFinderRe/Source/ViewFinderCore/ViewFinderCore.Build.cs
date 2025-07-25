// Copyright StrangeDS. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class ViewFinderCore : ModuleRules
{
	public ViewFinderCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModuleDirectory, "Public"),
				Path.Combine(ModuleDirectory, "Public/Components"),
				Path.Combine(ModuleDirectory, "Public/Functions"),
				Path.Combine(ModuleDirectory, "Public/Interfaces"),
				Path.Combine(ModuleDirectory, "Public/Subsystems"),
				Path.Combine(ModuleDirectory, "Public/Generators"),
				Path.Combine(ModuleDirectory, "Public/Config"),
			}
			);


		PrivateIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModuleDirectory, "Private"),
				Path.Combine(ModuleDirectory, "Private/Components"),
				Path.Combine(ModuleDirectory, "Private/Functions"),
				Path.Combine(ModuleDirectory, "Private/Interfaces"),
				Path.Combine(ModuleDirectory, "Private/Subsystems"),
				Path.Combine(ModuleDirectory, "Private/Config"),
			}
			);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"DynamicMesh",
				// "GeometryScriptingCore",
			}
			);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"MeshConversionEngineTypes",
				"ModelingComponents",
				"GeometryCore",
				"GeometryFramework",
				"PhysicsCore",
				"DeveloperSettings",
				"Projects",
			}
			);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		if (Target.bBuildEditor == true)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"UnrealEd",
					"AssetTools"
				}
			);
		}
	}
}
