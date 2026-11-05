// CONFIDENCE: HIGH
FFortWindImpulseRadius Result;
Result.Location = ImpulseLocation;
Result.Radius = ImpulseRadius;
Result.CurrentRadius = 0.0f;
Result.PreviousRadius = 0.0f;
Result.Magnitude = ImpulseMagnitude;
Result.CurrentMagnitude = 0.0f;
Result.PreviousMagnitude = 0.0f;
Result.BlendTime = ImpulseBlendTime;
Result.CurrentBlendTime = ImpulseBlendTime;
Result.WorldBounds = FBox(ImpulseLocation, ImpulseLocation);
return Result;
