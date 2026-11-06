// Original exec: 0x142aa7180; native body: 0x141d16d80.
// CONFIDENCE: high
WaterInteractionComponent = WaterComponent;
UFortWaterInteractionComponent* Interaction = WaterComponent;
if (Interaction)
{
    Interaction->bAutoActivate = false;
}
