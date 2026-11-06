// Script multicast at +0x11f8; integration must declare this non-reflected member.
// Broadcast owns a by-value snapshot, so callbacks cannot observe subsequent mutation.
const FDetailedMetricInformation Snapshot = MetricInformation;
MetricInformationChanged.Broadcast(this, Snapshot);
