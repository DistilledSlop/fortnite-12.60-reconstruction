// CONFIDENCE: high; native returns dead unless health is strictly positive (including NaN).
return !(GetHealth() > 0.0f) || bIsDying || IsPendingKill();
