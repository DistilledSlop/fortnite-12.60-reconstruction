// Script multicast at +0x1208; integration must declare this non-reflected member.
const FSimpleMetricInformation Snapshot = SimpleMetricInformation;
SimpleMetricInformationChanged.Broadcast(this, Snapshot);
