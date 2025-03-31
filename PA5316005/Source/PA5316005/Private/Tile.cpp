#include "Tile.h"
#include "Components/TextRenderComponent.h"


// Sets default values
ATile::ATile()
{
	PrimaryActorTick.bCanEverTick = false;

	
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));

	TileTextNumber = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TileTextNumber"));
	TileTextNumber->SetupAttachment(RootComponent);

	TileTextLetter = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TileTextLetter"));
	TileTextLetter->SetupAttachment(RootComponent);


	SetRootComponent(Scene);
	StaticMeshComponent->SetupAttachment(Scene);

    // Initialize default tile state.
	Status = ETileStatus::EMPTY;
	TileGameStatus = ETileGameStatus::FREE;
	bIsLegal = false;
	TileGridPosition = FVector2D(-1, -1);
	PlayerOwner = -1;
}



void ATile::BeginPlay()
{
	Super::BeginPlay();

}

void ATile::SetTileStatus(int32 TileOwner, ETileStatus NewTileStatus, AGameUnit* TileGameUnit)
{
    PlayerOwner = TileOwner;
    Status = NewTileStatus;
    GameUnit = TileGameUnit;

    
    SetTileMaterial(); 

}



void ATile::SetTileGameStatus(ETileGameStatus NewTileGameStatus)
{
	TileGameStatus = NewTileGameStatus;

	(NewTileGameStatus == ETileGameStatus::LEGAL_MOVE || NewTileGameStatus == ETileGameStatus::CAN_ATTACK)
		? bIsLegal = true
		: bIsLegal = false;

	SetTileMaterial();
}

void ATile::SetGridPosition(const double InX, const double InY)
{
	TileGridPosition.Set(InX, InY);
}



void ATile::SetTileMaterial() const
{
    if (!StaticMeshComponent)
    {
        return;
    }


    if (GetTileStatus() == ETileStatus::OBSTACLE)
    {

        int32 RandomIndex = FMath::RandRange(1, 3);
        UMaterialInterface* ChosenMaterial = nullptr;
        switch (RandomIndex)
        {
        case 1: ChosenMaterial = ObstacleMaterial1; break;
        case 2: ChosenMaterial = ObstacleMaterial2; break;
        case 3: ChosenMaterial = ObstacleMaterial3; break;
        }
        if (ChosenMaterial)
        {
            StaticMeshComponent->SetMaterial(0, ChosenMaterial);
        }
        return;
    }
    else
    {
   
        UMaterialInstanceDynamic* DynMaterial = StaticMeshComponent->CreateDynamicMaterialInstance(0);
        if (!DynMaterial)
        {
            return;
        }

        
        FLinearColor NewColor = FLinearColor::White; 
        switch (TileGameStatus)
        {
        case ETileGameStatus::FREE:
            NewColor = FLinearColor::White;
            break;
        case ETileGameStatus::SELECTED:
            NewColor = FLinearColor::Blue;
            break;
        case ETileGameStatus::LEGAL_MOVE:
            NewColor = FLinearColor::Green;
            break;
        case ETileGameStatus::CAN_ATTACK:
            NewColor = FLinearColor::Red;
            break;
        default:
            break;
        }
        DynMaterial->SetVectorParameterValue(TEXT("BaseColor"), NewColor);
    }
}



ETileStatus ATile::GetTileStatus() const
{
    return Status;
}

ETileGameStatus ATile::GetTileGameStatus() const
{
    return TileGameStatus;
}

inline bool ATile::IsLegalTile() const
{
    return bIsLegal;
}

FVector2D ATile::GetGridPosition() const
{
    return TileGridPosition;
}

int32 ATile::GetTileOwner() const
{
    return PlayerOwner;
}

AGameUnit* ATile::GetGameUnit() const
{
    return GameUnit;
}

FString ATile::GameStatusToString() const
{
    switch (Status)
    {
    case ETileStatus::EMPTY:    return TEXT("EMPTY");
    case ETileStatus::OCCUPIED: return TEXT("OCCUPIED");
    case ETileStatus::OBSTACLE: return TEXT("OBSTACLE");
    default:                    return TEXT("Unknown");
    }
}










