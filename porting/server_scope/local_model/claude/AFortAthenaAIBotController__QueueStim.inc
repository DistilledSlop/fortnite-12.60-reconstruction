// CONFIDENCE: MEDIUM. Original exec 0x1427c3410; the thunk reads the declared parameters in order
// (SourceActor, SourceLocation, SourceDirection, NewStimType, StimStrength, StimTag) and forwards them to the native
// perception handler 0x1416a83f0 (kept as a direct helper call). That handler is the OnPerceptionSensed path: it
// computes the stimulus delay and strength from the CachePerceptionDigestedSkillSet, and either queues the stimulus
// into DelayedStimulus or handles it immediately. Stimulus type 2 and 5 (MightHaveSeen) are unsupported and log a warning.
HandlePerceptionStim(SourceActor, SourceLocation, SourceDirection, NewStimType, StimStrength, StimTag);
