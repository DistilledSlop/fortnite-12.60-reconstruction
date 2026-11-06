// Original exec: 0x142aa67b0; native body: 0x141d15ca0.
// CONFIDENCE: high
if (AActor* Owner = GetOwner())
{
    Owner->FlushNetDormancy();
}
DesiredDirection = Direction;
