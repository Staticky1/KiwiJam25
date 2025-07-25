// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class KiwiJam2025 : ModuleRules
{
	public KiwiJam2025(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
