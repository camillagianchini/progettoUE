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

    // Crea un'istanza dinamica del materiale
    UMaterialInstanceDynamic* DynMaterial = StaticMeshComponent->CreateDynamicMaterialInstance(0);
    if (!DynMaterial)
    {
        UE_LOG(LogTemp, Error, TEXT("DynMaterial == nullptr!"));
        return;
    }

    // 1) Dichiara e inizializza la variabile FLinearColor
    FLinearColor NewColor = FLinearColor::White; // default

    // 2) In base allo stato della tile, scegli il colore
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

    // 3) Usa la variabile NewColor per impostare il parametro “BaseColor”
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










