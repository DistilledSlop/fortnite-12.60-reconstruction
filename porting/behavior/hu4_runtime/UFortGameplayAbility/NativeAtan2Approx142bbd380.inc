// Helper 0x142bbd380 (UFortGameplayAbility static). Single-precision atan2(Y, X) approximation.
// Asm evidence: XMM0 = Y, XMM1 = X (callers load vec.X into XMM1 and vec.Y into XMM0). AbsY > AbsX selects the
// dominant axis; MaxAbs = max(|Y|,|X|), MinAbs = min(|Y|,|X|); MaxAbs == 0 (or NaN) returns +0.
// Ratio = MinAbs / MaxAbs; polynomial in Ratio^2 with the six .rdata coefficients at 0x1465c1928..0x1465c193c and
// constant 1.0 at 0x1459a86f8 (read from the PE image); Angle = Ratio * poly. Then: if |Y| > |X|, Angle = pi/2 - Angle
// (0x1459a978c = 1.5707963705062866); if X < 0, Angle = pi - Angle (0x1459a9790 = 3.1415927410125732); if Y < 0, negate
// (sign mask 0x1459a3d10). Abs uses the 0x1459a3d00 mask, equivalent to FMath::Abs.
// CONFIDENCE: HIGH
const float AbsY = FMath::Abs(Y);
const float AbsX = FMath::Abs(X);
const bool bAbsYGreater = AbsY > AbsX;
const float MaxAbs = bAbsYGreater ? AbsY : AbsX;
const float MinAbs = bAbsYGreater ? AbsX : AbsY;
if (!(MaxAbs > 0.0f))
{
    return 0.0f;
}
const float Ratio = MinAbs / MaxAbs;
const float RatioSq = Ratio * Ratio;
float Poly = RatioSq * 0.0072128851898014545f;
Poly -= 0.03505967929959297f;
Poly *= RatioSq;
Poly += 0.08167587965726852f;
Poly *= RatioSq;
Poly -= 0.13374657928943634f;
Poly *= RatioSq;
Poly += 0.19856563210487366f;
Poly *= RatioSq;
Poly -= 0.3332499861717224f;
Poly *= RatioSq;
Poly += 1.0f;
float Angle = Ratio * Poly;
if (bAbsYGreater)
{
    Angle = 1.5707963705062866f - Angle;
}
if (X < 0.0f)
{
    Angle = 3.1415927410125732f - Angle;
}
if (Y < 0.0f)
{
    Angle = -Angle;
}
return Angle;
