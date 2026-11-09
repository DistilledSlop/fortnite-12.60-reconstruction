// Original address: 0x1407fb210; native-ui-startup-checkpoint-2 vtable +0x260; common-input-manager-queue-12/1407fb210.c; native-ui-startup-50/14421ee60.c. CONFIDENCE: MEDIUM.
const UGameInstance* GameInstance = static_cast<const UGameInstance*>(Outer);
const FWorldContext* Context = GameInstance->GetWorldContext();
return !Context || !Context->RunAsDedicated;
