// Helper 0x142bc0d80: float remainder (FMath::Fmod shape).
// Evidence: abs mask [0x1459a3d00]=0x7fffffff; divisor threshold [0x1459c03a0]=1e-8f;
// truncation limit [0x1465c1948]=2^23; sign mask [0x1459a3d10]=0x80000000 (negation of |Y|).
// Every instruction of the 33-insn body is accounted for, including the JC/JNC/JBE/JA targets.
// CONFIDENCE: HIGH
const float AbsDivisor = FMath::Abs(Y);
if (!(AbsDivisor > 1.0e-8f))
{
    return 0.0f;
}
const float Quotient = X / Y;
const float AbsQuotient = FMath::Abs(Quotient);
float Truncated = Quotient;
if (!(AbsQuotient >= 8388608.0f))
{
    Truncated = static_cast<float>(static_cast<int32>(Quotient));
}
float Product = Truncated * Y;
const float AbsProduct = FMath::Abs(Product);
const float AbsDividend = FMath::Abs(X);
if (!(AbsProduct <= AbsDividend))
{
    Product = X;
}
const float Result = X - Product;
if (Result < -AbsDivisor)
{
    return Result;
}
return (Result < AbsDivisor) ? Result : AbsDivisor;
