// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WorldMapWidget.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

void UWorldMapWidget::SetWorldBounds(const FBox& InBounds)
{
	WorldBounds = InBounds;
}

FVector2D UWorldMapWidget::WorldToMapPosition(const FVector& WorldLocation) const
{
    if (!MarkerCanvas) return FVector2D::ZeroVector;

    FVector2D MapSize = MarkerCanvas->GetCachedGeometry().GetLocalSize();

    float NormalizedX = (WorldLocation.X - WorldBounds.Min.X) / (WorldBounds.Max.X - WorldBounds.Min.X);
    float NormalizedY = (WorldLocation.Y - WorldBounds.Min.Y) / (WorldBounds.Max.Y - WorldBounds.Min.Y);

    // Flip Y because UI Y grows downward
    //NormalizedY = 1.f - NormalizedY;

    float MapPosX = NormalizedX * MapSize.X;
    float MapPosY = (NormalizedY) * MapSize.Y;

    return FVector2D(MapPosX, MapPosY);
}

void UWorldMapWidget::AddMarkerPersistent(UUserWidget* MarkerWidget, const FVector& WorldLocation)
{
    if (!MarkerWidget) return;

    PersistentMarkers.Add(FWorldMapMarker(MarkerWidget, WorldLocation));
    UE_LOG(LogTemp, Warning, TEXT("Added Marker to array at %s"), *WorldLocation.ToString());
    // If map is open, add immediately
    if (MarkerCanvas && !MarkerCanvas->HasChild(MarkerWidget))
    {
        MarkerCanvas->AddChild(MarkerWidget);
        UE_LOG(LogTemp, Warning, TEXT("Added Marker at %s"), *WorldLocation.ToString());
    }
}

void UWorldMapWidget::RemoveMarker(UUserWidget* MarkerWidget)
{
    if (!MarkerWidget) return;

    // Remove from UI
    if (MarkerCanvas && MarkerCanvas->HasChild(MarkerWidget))
    {
        MarkerCanvas->RemoveChild(MarkerWidget);
    }

    // Remove from persistent array
    PersistentMarkers.RemoveAll([MarkerWidget](const FWorldMapMarker& Marker)
        {
            return Marker.Widget == MarkerWidget;
        });
}

void UWorldMapWidget::SetZoom(float NewZoom)
{
    ZoomLevel = FMath::Clamp(NewZoom, 0.5f, 4.0f); // Limit zoom range
    if (MapImage)
    {
        UCanvasPanelSlot* MapSlot = Cast<UCanvasPanelSlot>(MapImage->Slot);
        if (MapSlot)
        {
            MapSlot->SetSize(CachedMapSize * ZoomLevel);
        }
    }
}

void UWorldMapWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateMapSize();

    UE_LOG(LogTemp, Warning, TEXT("Called"));

    // When map opens, add all markers to the canvas
    for (FWorldMapMarker& Marker : PersistentMarkers)
    {
        UE_LOG(LogTemp, Warning, TEXT("Marker"));
        if (Marker.Widget && !MarkerCanvas->HasChild(Marker.Widget))
        {
            MarkerCanvas->AddChild(Marker.Widget);
            UE_LOG(LogTemp, Warning, TEXT("Added Marker at %s"), *Marker.WorldLocation.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid"));
        }
    }
}


void UWorldMapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	UpdateMapSize(); // Ensure map size is always correct
    UpdateMarkerPositions();
}

void UWorldMapWidget::UpdateMapSize()
{
    if (MapImage)
    {
        // Cache current size
        CachedMapSize = MapImage->GetCachedGeometry().GetLocalSize();
    }
}

void UWorldMapWidget::UpdateMarkerPositions()
{
    for (FWorldMapMarker& Marker : PersistentMarkers)
    {
        if (Marker.Widget && MarkerCanvas)
        {
            FVector2D MapPos = WorldToMapPosition(Marker.WorldLocation);

            if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Marker.Widget->Slot))
            {
                CanvasSlot->SetPosition(MapPos);
                CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f)); // Center the icon
                CanvasSlot->SetAutoSize(false);
                CanvasSlot->SetSize(FVector2D(32.f, 32.f));
            }
        }
    }
}
