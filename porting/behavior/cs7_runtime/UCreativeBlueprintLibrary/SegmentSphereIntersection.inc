// Original address: 0x141d8f690 (thunk 0x14276be80)
// Evidence: static; RCX = hidden TArray return slot, RDX = SegmentStart, R8 = SegmentEnd, R9 = SphereOrigin, stack arg = SphereRadius. Output array grows via 0x140694640 (TArray<FVector> Add).
// Both endpoints inside the sphere (|P-O|^2 not > r^2) returns empty. Otherwise A = |D|^2 (D = End-Start; A <= 1e-8 returns empty), B = D.(Start-O), C = |Start-O|^2 - r^2, disc = (2B)^2 - 4AC (disc < 0 returns empty).
// |disc| <= 1e-8: tangent point t = -B/A, added if t <= 1. Otherwise t1 = (sqrt(disc) - 2B)*0.5/A and t2 = (-2B - sqrt(disc))*0.5/A, each added if t <= 1 (no lower bound in asm).
// Rdata constants decoded from the runtime image: 1e-8f (0x1459c03a0), 4.0f (0x1459c03c0), 0.5f (0x1459a86f0), 1.0f (0x1459a86f8).
// CONFIDENCE: MEDIUM
TArray<FVector> Intersections;

const float RadiusSq = SphereRadius * SphereRadius;
const float StartX = SegmentStart.X - SphereOrigin.X;
const float StartY = SegmentStart.Y - SphereOrigin.Y;
const float StartZ = SegmentStart.Z - SphereOrigin.Z;
const float EndX = SegmentEnd.X - SphereOrigin.X;
const float EndY = SegmentEnd.Y - SphereOrigin.Y;
const float EndZ = SegmentEnd.Z - SphereOrigin.Z;
const float StartDistSq = StartX * StartX + StartY * StartY + StartZ * StartZ;
const float EndDistSq = EndX * EndX + EndY * EndY + EndZ * EndZ;

if (!(StartDistSq > RadiusSq) && !(EndDistSq > RadiusSq))
{
	return Intersections;
}

const float DX = SegmentEnd.X - SegmentStart.X;
const float DY = SegmentEnd.Y - SegmentStart.Y;
const float DZ = SegmentEnd.Z - SegmentStart.Z;
const float A = DX * DX + DY * DY + DZ * DZ;
if (!(A > 1e-8f))
{
	return Intersections;
}

const float C = StartDistSq - RadiusSq;
const float TwoB = 2.0f * (DX * StartX + DY * StartY + DZ * StartZ);
const float Disc = TwoB * TwoB - (C * A) * 4.0f;
if (!(Disc >= 0.0f))
{
	return Intersections;
}

const float HalfOverA = 0.5f / A;

if (FMath::Abs(Disc) <= 1e-8f)
{
	const float TangentT = -(HalfOverA * TwoB);
	if (!(TangentT > 1.0f))
	{
		Intersections.Add(FVector(SegmentStart.X + DX * TangentT, SegmentStart.Y + DY * TangentT, SegmentStart.Z + DZ * TangentT));
	}
	return Intersections;
}

const float Root = FMath::Sqrt(Disc);

const float T1 = (Root - TwoB) * HalfOverA;
if (!(T1 > 1.0f))
{
	Intersections.Add(FVector(SegmentStart.X + DX * T1, SegmentStart.Y + DY * T1, SegmentStart.Z + DZ * T1));
}

const float T2 = (-TwoB - Root) * HalfOverA;
if (!(T2 > 1.0f))
{
	Intersections.Add(FVector(SegmentStart.X + DX * T2, SegmentStart.Y + DY * T2, SegmentStart.Z + DZ * T2));
}

return Intersections;
