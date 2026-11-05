// CONFIDENCE: HIGH (source recovery; build/runtime pending)
while (WindImpulseCylinderDeltas.Num() > 0)
{
    const int32 TargetUID = WindImpulseCylinderDeltas[0].WindImpulseHandleToModify.UID;
    for (FFortWindImpulseCylinder& Cylinder : WindImpulseCylinders)
    {
        if (Cylinder.Handle.UID == TargetUID)
        {
            Cylinder.bIsChangePending = false;
            Cylinder.bIsChanging = true;
            OnWindImpulseCylinderCreation(Cylinder, WindImpulseCylinderDeltas[0]);
            return;
        }
    }
    WindImpulseCylinderDeltas.RemoveAtSwap(0, 1, false);
}
