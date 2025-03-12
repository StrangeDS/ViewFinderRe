// Copyright StrangeDS. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ViewFinderTarget : TargetRules
{
	public ViewFinderTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V4;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;
		ExtraModuleNames.Add("ViewFinder");
	}
}
