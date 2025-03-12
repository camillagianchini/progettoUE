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
    // Verifica che il componente esista
    if (!StaticMeshComponent)
    {
        return;
    }

    // Crea o ottieni l'istanza dinamica del materiale sullo slot 0
    UMaterialInstanceDynamic* DynMaterial = StaticMeshComponent->CreateDynamicMaterialInstance(0);
    if (!DynMaterial)
    {
        return;
    }

    // Definisci il colore da applicare in base allo stato della tile
    FLinearColor NewColor = FLinearColor::White; // Colore di default per FREE

    switch (TileGameStatus)
    {
    case ETileGameStatus::FREE:
        NewColor = FLinearColor::White; // Puoi scegliere un colore base neutro
        break;
    case ETileGameStatus::SELECTED:
        NewColor = FLinearColor::Blue;  // Evidenzia l'unità selezionata
        break;
    case ETileGameStatus::LEGAL_MOVE:
        NewColor = FLinearColor::Green; // Evidenzia le mosse legali
        break;
    case ETileGameStatus::CAN_ATTACK:
        NewColor = FLinearColor::Red;   // Evidenzia possibili attacchi
        break;
    default:
        break;
    }

    // Assicurati che il materiale usato abbia un parametro colore (ad es. "BaseColor")
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
    switch (TileGameStatus)
    {
    case ETileGameStatus::FREE: return FString("FREE");
    case ETileGameStatus::SELECTED: return FString("SELECTED");
    case ETileGameStatus::LEGAL_MOVE: return FString("LEGAL_MOVE");
    case ETileGameStatus::CAN_ATTACK: return FString("CAN_ATTACK");
    default: return FString("Unknown");
   }
}









