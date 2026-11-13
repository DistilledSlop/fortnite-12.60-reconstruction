// CONFIDENCE: HIGH
// Original 0x141f85430. NewObject uses this as Outer, NAME_None and RF_NoFlags.
UE_LOG(LogFortWorld, Log, TEXT("AFortWorldManager::CreateNewWorldRecord() creating new world record for %s"),
    *InstanceDetails.WorldId);
UFortWorldRecord* Record = NewObject<UFortWorldRecord>(this);
Record->ZoneInstanceInfo = InstanceDetails;
Record->WUID = InstanceDetails.WorldId;
return Record;
