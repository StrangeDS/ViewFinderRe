// Copyright 2026, StrangeDS. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ViewFinderEditorTarget : TargetRules
{
	public ViewFinderEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V4;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;
		ExtraModuleNames.Add("ViewFinder");
	}
}
