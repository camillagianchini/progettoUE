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

void ATile::SetTileStatus(int32 TileOwner, ETileStatus NewTileStatus, AGameUnit* TileGameUnit)
{
    PlayerOwner = TileOwner;
    Status = NewTileStatus;
    GameUnit = TileGameUnit;

    // Non toccare TileGameStatus (niente FREE/SELECTED qui)
    SetTileMaterial(); // Se vuoi, lo chiami, ma non alteri TileGameStatus

    // Aggiorna solo la mesh per riflettere "occupato" o "vuoto" se ti serve
    // Oppure gestisci la parte visiva con un materiale separato
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

    // Se la tile è un ostacolo, usa uno dei materiali d'ostacolo.
    if (GetTileStatus() == ETileStatus::OBSTACLE)
    {
        // Supponiamo che ObstacleMaterial1, ObstacleMaterial2 e ObstacleMaterial3 siano
        // membri UPROPERTY già impostati nel Blueprint o nel codice.
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
        // Per le tile non ostacolo, creiamo un'istanza dinamica del materiale
        UMaterialInstanceDynamic* DynMaterial = StaticMeshComponent->CreateDynamicMaterialInstance(0);
        if (!DynMaterial)
        {
            return;
        }

        // Imposta il colore in base allo stato di evidenziazione della tile (TileGameStatus)
        FLinearColor NewColor = FLinearColor::White; // Default per FREE
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










