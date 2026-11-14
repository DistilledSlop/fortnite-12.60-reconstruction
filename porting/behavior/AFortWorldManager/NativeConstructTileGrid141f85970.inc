// CONFIDENCE: HIGH
// Complete 0x141f85970. Needs generated AWorldTileFoundation native X/Y
// int32 fields at original +0xee4/+0xee8. Original offsets are evidence,
// never offsets into the reconstructed host object.
if (GetLocalRole() != ROLE_Authority)
{
    return;
}
const float OriginX = FortWorldManagerOriginalGrid::Origin(X_Size, TileEdgeSize);
const float OriginY = FortWorldManagerOriginalGrid::Origin(Y_Size, TileEdgeSize);
// Original appends zeroed rows but indexes from row zero, even on repeated calls.
// Preserve this unusual behavior; replacing it with SetNum/Empty changes parity.
Y.AddZeroed(Y_Size);
for (int32 Row = 0; Row < Y_Size; ++Row)
{
    Y[Row].X.AddZeroed(X_Size);
    for (int32 Column = 0; Column < X_Size; ++Column)
    {
        const float X = float(FortWorldManagerOriginalGrid::Mul32(
            FortWorldManagerOriginalGrid::Mul32(TileEdgeSize, Column), 512)) + OriginX;
        const float YPosition = float(FortWorldManagerOriginalGrid::Mul32(
            FortWorldManagerOriginalGrid::Mul32(TileEdgeSize, Row), 512)) + OriginY;
        const FVector Position(X, YPosition, 0.0f);
        const FRotator Rotation(0.0f, 0.0f, 0.0f);
        FActorSpawnParameters Parameters;
        Parameters.Owner = this;
        Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AWorldTileFoundation* Foundation = GetWorld()->SpawnActor<AWorldTileFoundation>(
            AWorldTileFoundation::StaticClass(), Position, Rotation, Parameters);
        Y[Row].X[Column] = Foundation;
        Foundation->NativeTileY_0xee8 = Row;
        Foundation->NativeTileX_0xee4 = Column;
    }
}
