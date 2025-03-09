// Fill out your copyright notice in the Description page of Project Settings.


#include "Tile.h"

// Sets default values
ATile::ATile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));

	SetRootComponent(Scene);
	StaticMeshComponent->SetupAttachment(Scene);

	Status = ETileStatus::EMPTY;
	PlayerOwner = -1;
	TileGridPosition = FVector2D(0, 0);
}

void ATile::SetTileStatus(const int32 TileOwner, const ETileStatus TileStatus)
{
	PlayerOwner = TileOwner;
	Status = TileStatus;
}

ETileStatus ATile::GetTileStatus()
{
	return Status;
}

int32 ATile::GetOwner()
{
	return PlayerOwner;
}

void ATile::SetGridPosition(const double InX, const double InY)
{
	TileGridPosition.Set(InX, InY);
}

FVector2D ATile::GetGridPosition()
{
	return TileGridPosition;
}

// Called when the game starts or when spawned
void ATile::BeginPlay()
{
	Super::BeginPlay();

}

#include "Tile.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void ATile::HighlightTile(const FColor& InColor)
{
	// Creiamo un Dynamic Material Instance (DMI) se non esiste
	// In un gioco di esempio, potresti farlo in BeginPlay() una volta sola
	UMaterialInstanceDynamic* DynMat = StaticMeshComponent->CreateAndSetMaterialInstanceDynamic(0);
	if (DynMat)
	{
		// Se nel tuo materiale hai un parametro tipo "BaseColor", puoi settarlo:
		// Nota: dipende da come è fatto il tuo materiale
		DynMat->SetVectorParameterValue(FName("BaseColor"), FVector(InColor.R, InColor.G, InColor.B));
	}
}

void ATile::ResetHighlight()
{
	// Torna al materiale di default, oppure usa un colore neutro.
	// Esempio: usa di nuovo un DMI con un colore "grigio"
	UMaterialInstanceDynamic* DynMat = StaticMeshComponent->CreateAndSetMaterialInstanceDynamic(0);
	if (DynMat)
	{
		DynMat->SetVectorParameterValue(FName("BaseColor"), FVector(1.f, 1.f, 1.f));
	}
}


// Called every frame
//void ATile::Tick(float DeltaTime)
//{
	//Super::Tick(DeltaTime);
//
//}

