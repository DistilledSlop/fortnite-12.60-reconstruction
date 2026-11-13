# POI discoverability component replication

Original constructor 0x1418b5f70 calls SetIsReplicated(true) through 0x1440ca8f0.
The reconstructed component lacked that native default. source_policy.py merges
SetIsReplicatedByDefault(true) into its generated constructor, preserving stable
component replication without changing the reflected property layout.

This is a prerequisite only. Discoverable-volume query readiness, initial entries,
server visit updates and client rendering remain incomplete. The adjacent initial
query proposal is deliberately not admitted to production: the original unreflected
array cleanup and readiness lifecycle must first be recovered.
