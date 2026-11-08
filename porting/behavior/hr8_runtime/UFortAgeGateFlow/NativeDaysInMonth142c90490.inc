// Helper 0x142c90490: ECX=Year, EDX=Month. For Month==2: leap if Year%4==0 and (Year%100!=0 or Year%400==0) -> 29.
// Otherwise returns DaysPerMonth[Month] from the .rdata dword table at 0x1465da1b8 (entries 0..12 read from the PE image:
// 0,31,28,31,30,31,30,31,31,30,31,30,31).
// CONFIDENCE: HIGH - the magic-number divisions in the asm are the standard signed modulus tests (year % 100, year % 400).
if (Month == 2)
{
    if (Year % 4 == 0)
    {
        if (Year % 100 != 0)
        {
            return 29;
        }
        if (Year % 400 == 0)
        {
            return 29;
        }
    }
}
static const int32 DaysPerMonth[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
return DaysPerMonth[Month];
