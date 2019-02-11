#pragma once

#include "FriendListStruct.generated.h"
 // use this class to store info from Player Friends list
USTRUCT(BlueprintType)
struct FriendListItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "State Manager")
		FString displayName;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "State Manager")
		FString id;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "State Manager")
		bool isActive = false;
};
