// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WorldMapWidget.generated.h"


class UImage;
class UCanvasPanel;
class UCanvasPanelSlot;
class UTexture2D; 

USTRUCT()
struct FWorldMapMarker
{
    GENERATED_BODY()

    UPROPERTY()
    UUserWidget* Widget;

    FVector WorldLocation;

    FWorldMapMarker() : Widget(nullptr), WorldLocation(FVector::ZeroVector) {}
    FWorldMapMarker(UUserWidget* InWidget, const FVector& InLocation)
        : Widget(InWidget), WorldLocation(InLocation) {
    }
};

/**
 * 
 */
UCLASS()
class KIWIJAM2025_API UWorldMapWidget : public UUserWidget
{
	GENERATED_BODY()

public:	
    // Sets the bounds of the playable area (for position normalization)
    UFUNCTION(BlueprintCallable, Category = "World Map")
    void SetWorldBounds(const FBox& InBounds);

    // Converts world position to UI map position
    UFUNCTION(BlueprintCallable, Category = "World Map")
    FVector2D WorldToMapPosition(const FVector& WorldLocation) const;

    
    UFUNCTION(BlueprintCallable)
    void AddMarkerPersistent(UUserWidget* MarkerWidget, const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable)
    void RemoveMarker(UUserWidget* MarkerWidget);

    // Set zoom level (1.0 = normal, >1 = zoom in)
    UFUNCTION(BlueprintCallable, Category = "World Map")
    void SetZoom(float NewZoom);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    // Widgets bound from UMG
    UPROPERTY(meta = (BindWidget))
    UImage* MapImage;

    UPROPERTY(meta = (BindWidget))
    UCanvasPanel* MarkerCanvas;

    UPROPERTY()
    TArray<FWorldMapMarker> PersistentMarkers;

    // Data
    FBox WorldBounds;
    float ZoomLevel = 1.0f;

    FVector2D CachedMapSize;

    void UpdateMapSize();
    void UpdateMarkerPositions();

};
