// CONFIDENCE: high
// Athena UFortHeroType specialization of factory 0x142017d90.
// Definition class virtual +0x280 is 0x142512d20 for UFortHeroType and
// returns UFortHero::StaticClass(), proven by ctor vtable 0x1461971b8.
// Original factory writes only definition, quantity, and optional local id;
// account level and hero clamp must come from their original CDO/constructor.
UFortHero* Hero = NewObject<UFortHero>(GetTransientPackage());
if (!Hero)
{
    return nullptr;
}
Hero->ItemDefinition = this;
Hero->Quantity = InQuantity;
// 0x141f3ad70 caches FParse::Param(FCommandLine::Get(), TEXT("nomcp")).
static const bool bNoMcp = FParse::Param(FCommandLine::Get(), TEXT("nomcp"));
if (bNoMcp)
{
    // 0x142c9ff10 -> CoCreateGuid; 0x142ca88a0 format1; 0x142bb82f0 lowercases.
    Hero->InstanceId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens).ToLower();
}
return Hero;
