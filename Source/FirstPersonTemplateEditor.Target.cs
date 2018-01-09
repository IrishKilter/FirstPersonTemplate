// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class FirstPersonTemplateEditorTarget : TargetRules
{
    //public FirstPersonTemplateEditorTarget(TargetInfo Target)
    public FirstPersonTemplateEditorTarget(TargetInfo Target) : base (Target)
    {
		Type = TargetType.Editor;
        ExtraModuleNames.Add("FirstPersonTemplate");
	}

	//
	// TargetRules interface.
	//

	/*public override void SetupBinaries(
		TargetInfo Target,
		ref List<UEBuildBinaryConfiguration> OutBuildBinaryConfigurations,
		ref List<string> OutExtraModuleNames
		)
	{
		OutExtraModuleNames.AddRange( new string[] { "FirstPersonTemplate" } );
	}*/
}
