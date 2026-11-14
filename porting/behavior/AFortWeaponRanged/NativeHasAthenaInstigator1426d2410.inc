// CONFIDENCE: HIGH. Original1426d2410; preserve both original class tests.
APawn* OwnerPawn = GetInstigator();
AFortPawn* FortPawn = Cast<AFortPawn>(OwnerPawn);
return FortPawn && Cast<AFortPlayerPawnAthena>(FortPawn) != nullptr;
