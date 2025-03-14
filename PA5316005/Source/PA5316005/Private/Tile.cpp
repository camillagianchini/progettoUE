#include "Tile.h"
#include "Components/TextRenderComponent.h"


// Sets default values
ATile::ATile()
{
	PrimaryActorTick.bCanEverTick = false;

	// Creazione dei componenti
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));

	TileTextNumber = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TileTextNumber"));
	TileTextNumber->SetupAttachment(RootComponent);

	TileTextLetter = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TileTextLetter"));
	TileTextLetter->SetupAttachment(RootComponent);

	// Imposto il componente di root e lo attacco alla scena
	SetRootComponent(Scene);
	StaticMeshComponent->SetupAttachment(Scene);

	// Inizializzazione degli stati
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

void ATile::SetTileStatus(const int32 TileOwner, const ETileStatus TileStatus, AGameUnit* TileGameUnit)
{
    PlayerOwner = TileOwner;
    Status = TileStatus;
    GameUnit = TileGameUnit;

    // Aggiorna anche TileGameStatus in base al TileStatus
    if (TileStatus == ETileStatus::EMPTY)
    {
        TileGameStatus = ETileGameStatus::FREE;
    }
    else // Occupied
    {
        TileGameStatus = ETileGameStatus::SELECTED; // o un altro valore che ritieni appropriato
    }

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

    // Crea o ottieni l'istanza dinamica del materiale
    UMaterialInstanceDynamic* DynMaterial = StaticMeshComponent->CreateDynamicMaterialInstance(0);
    if (!DynMaterial)
    {
        return;
    }

    // Seleziona un colore in base allo stato della tile
    FLinearColor NewColor = FLinearColor::White; // Default per FREE
    switch (TileGameStatus)  // Se usi TileGameStatus, assicurati di averlo aggiornato in SetTileStatus, oppure usa direttamente lo stato "Status"
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
    default:                    return TEXT("Unknown");
    }
}










