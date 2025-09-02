// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using System.Numerics;
using UnrealBuildTool;

public class SimplePF : ModuleRules
{
    public SimplePF(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG" });

        PublicIncludePaths.Add(ModuleDirectory);
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Actor"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Character"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Character/AnimInstance"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Character/Component"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Character/DataAsset"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Character/Weapon"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Framework/Player"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Manager"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Table"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "UI"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Unique/GameInstance"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Unique/GameMode"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Util"));
    }
}
