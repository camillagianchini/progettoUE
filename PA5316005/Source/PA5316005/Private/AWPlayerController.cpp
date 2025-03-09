#include "AWPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "GameField.h"
#include "Tile.h"
#include "Sniper.h"
#include "Brawler.h"

AAWPlayerController::AAWPlayerController()
{
    bShowMouseCursor = true;
}

void AAWPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Assicura che la mapping context sia aggiunta
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->AddMappingContext(AWMappingContext, 0);
    }
}

void AAWPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
    {
        EnhancedInput->BindAction(ClickAction, ETriggerEvent::Started, this, &AAWPlayerController::OnClick);
    }
}

void AAWPlayerController::OnClick(const FInputActionValue& Value)
{
    HandleClick();
}

void AAWPlayerController::HandleClick()
{
    FHitResult Hit;
    GetHitResultUnderCursor(ECC_Visibility, false, Hit);

    if (Hit.bBlockingHit)
    {
        AActor* ClickedActor = Hit.GetActor();

        // Se clicchiamo su una nostra unità
        if (ClickedActor->ActorHasTag("HumanUnit"))
        {
            SelectUnit(ClickedActor);
        }
        // Se clicchiamo su una Tile e abbiamo un'unità selezionata
        else if (ATile* Tile = Cast<ATile>(ClickedActor); Tile && SelectedUnit)
        {
            MoveSelectedUnit(FVector2D(Tile->GetActorLocation().X, Tile->GetActorLocation().Y));
        }

        // Se clicchiamo su un nemico e abbiamo un'unità selezionata
        else if (ClickedActor->ActorHasTag("AIUnit") && SelectedUnit)
        {
            AttackWithSelectedUnit(ClickedActor);
        }
    }
}

void AAWPlayerController::SelectUnit(AActor* NewSelectedUnit)
{
    if (!NewSelectedUnit) return;

    // Seleziona solo se è una nuova unità
    if (SelectedUnit != NewSelectedUnit)
    {
        SelectedUnit = NewSelectedUnit;
        bHasMoved = false;
        bHasAttacked = false;
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Unità selezionata"));
    }
}

void AAWPlayerController::MoveSelectedUnit(FVector2D TargetPosition)
{
    if (!SelectedUnit || bHasMoved) return;

    FVector2D StartPos = FVector2D(SelectedUnit->GetActorLocation().X, SelectedUnit->GetActorLocation().Y);
    int32 MovementRange = 0;

    if (ASniper* Sniper = Cast<ASniper>(SelectedUnit))
    {
        MovementRange = Sniper->MovementRange;
    }
    else if (ABrawler* Brawler = Cast<ABrawler>(SelectedUnit))
    {
        MovementRange = Brawler->MovementRange;
    }

    if (!IsMoveValid(StartPos, TargetPosition, MovementRange))
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Movimento fuori range!"));
        return;
    }

    FVector NewLocation = FVector(TargetPosition.X, TargetPosition.Y, SelectedUnit->GetActorLocation().Z);
    SelectedUnit->SetActorLocation(NewLocation);
    bHasMoved = true;
    GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Unità spostata"));
}

void AAWPlayerController::AttackWithSelectedUnit(AActor* Target)
{
    if (!SelectedUnit || !Target || bHasAttacked) return;

    if (ASniper* Sniper = Cast<ASniper>(SelectedUnit))
    {
        Sniper->Attack(Target);
    }
    else if (ABrawler* Brawler = Cast<ABrawler>(SelectedUnit))
    {
        Brawler->Attack(Target);
    }

    bHasAttacked = true;
    GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Attacco eseguito"));
}

bool AAWPlayerController::IsMoveValid(FVector2D Start, FVector2D Target, int32 MovementRange)
{
    int32 Distance = FMath::Abs(Target.X - Start.X) + FMath::Abs(Target.Y - Start.Y); // Distanza Manhattan
    return (Distance <= MovementRange);
}

void AAWPlayerController::ShowMovementRange()
{
    if (!SelectedUnit) return;

    int32 MovementRange = 0;
    FVector2D UnitPos = FVector2D(SelectedUnit->GetActorLocation().X, SelectedUnit->GetActorLocation().Y);

    if (ASniper* Sniper = Cast<ASniper>(SelectedUnit))
    {
        MovementRange = Sniper->MovementRange;
    }
    else if (ABrawler* Brawler = Cast<ABrawler>(SelectedUnit))
    {
        MovementRange = Brawler->MovementRange;
    }

    // Evidenzia tutte le celle raggiungibili
    for (int X = -MovementRange; X <= MovementRange; ++X)
    {
        for (int Y = -MovementRange; Y <= MovementRange; ++Y)
        {
            FVector2D TargetPos = UnitPos + FVector2D(X, Y);
            if (IsMoveValid(UnitPos, TargetPos, MovementRange))
            {
                ATile* Tile = GameField->GetTileAt(TargetPos);
                if (Tile) Tile->HighlightTile(FColor::Blue);  // Funzione che dovrai implementare in Tile
            }
        }
    }
}

void AAWPlayerController::ClearMovementRange()
{
    GameField->ClearAllHighlightedTiles();  // Implementa questa funzione in GameField
}

void AAWPlayerController::ShowAttackRange()
{
    if (!SelectedUnit) return;

    int32 AttackRange = 0;
    FVector2D UnitPos = FVector2D(SelectedUnit->GetActorLocation().X, SelectedUnit->GetActorLocation().Y);

    if (ASniper* Sniper = Cast<ASniper>(SelectedUnit))
    {
        AttackRange = Sniper->AttackRange;
    }
    else if (ABrawler* Brawler = Cast<ABrawler>(SelectedUnit))
    {
        AttackRange = Brawler->AttackRange;
    }

    for (int X = -AttackRange; X <= AttackRange; ++X)
    {
        for (int Y = -AttackRange; Y <= AttackRange; ++Y)
        {
            FVector2D TargetPos = UnitPos + FVector2D(X, Y);
            if (GameField->IsValidTile(TargetPos))  // Se la cella è valida
            {
                ATile* Tile = GameField->GetTileAt(TargetPos);
                if (Tile) Tile->HighlightTile(FColor::Red);
            }
        }
    }
}

void AAWPlayerController::ClearAttackRange()
{
    GameField->ClearAllHighlightedTiles();
}

void AAWPlayerController::EndTurn()
{
    GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("Turno terminato, passa all'IA"));

    // Chiama il GameMode per passare il turno
    AAWGameMode* GameMode = Cast<AAWGameMode>(GetWorld()->GetAuthGameMode());
    if (GameMode)
    {
        GameMode->StartAITurn();  // Implementa questa funzione in GameMode
    }
}

bool AAWPlayerController::HaveAllUnitsActed()
{
    // Controlla se entrambe le unità hanno usato movimento e attacco
    bool AllUnitsDone = true;
    TArray<AActor*> PlayerUnits;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), "HumanUnit", PlayerUnits);

    for (AActor* Unit : PlayerUnits)
    {
        ASniper* Sniper = Cast<ASniper>(Unit);
        ABrawler* Brawler = Cast<ABrawler>(Unit);

        if ((Sniper && (!Sniper->HasMoved || !Sniper->HasAttacked)) ||
            (Brawler && (!Brawler->HasMoved || !Brawler->HasAttacked)))
        {
            AllUnitsDone = false;
        }
    }

    return AllUnitsDone;
}

// Dopo ogni mossa o attacco, controlliamo se il turno è finito
void AAWPlayerController::MoveSelectedUnit(FVector2D TargetPosition)
{
    if (!SelectedUnit || bHasMoved) return;

    FVector2D StartPos = FVector2D(SelectedUnit->GetActorLocation().X, SelectedUnit->GetActorLocation().Y);
    int32 MovementRange = 0;

    if (ASniper* Sniper = Cast<ASniper>(SelectedUnit))
    {
        MovementRange = Sniper->MovementRange;
        Sniper->HasMoved = true;  // Segna l'azione
    }
    else if (ABrawler* Brawler = Cast<ABrawler>(SelectedUnit))
    {
        MovementRange = Brawler->MovementRange;
        Brawler->HasMoved = true;
    }

    if (!IsMoveValid(StartPos, TargetPosition, MovementRange)) return;

    FVector NewLocation = FVector(TargetPosition.X, TargetPosition.Y, SelectedUnit->GetActorLocation().Z);
    SelectedUnit->SetActorLocation(NewLocation);
    bHasMoved = true;

    if (HaveAllUnitsActed())
    {
        EndTurn();
    }
}

void AAWPlayerController::AttackWithSelectedUnit(AActor* Target)
{
    if (!SelectedUnit || !Target || bHasAttacked) return;

    if (ASniper* Sniper = Cast<ASniper>(SelectedUnit))
    {
        Sniper->Attack(Target);
        Sniper->HasAttacked = true;
    }
    else if (ABrawler* Brawler = Cast<ABrawler>(SelectedUnit))
    {
        Brawler->Attack(Target);
        Brawler->HasAttacked = true;
    }

    bHasAttacked = true;

    if (HaveAllUnitsActed())
    {
        EndTurn();
    }
}
