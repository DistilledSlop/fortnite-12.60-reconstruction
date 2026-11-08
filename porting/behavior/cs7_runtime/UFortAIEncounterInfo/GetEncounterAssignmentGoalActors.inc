// Original address: 0x14165a7f0 (thunk 0x142798200)
// Evidence: [this+0xac8] EncounterAssignments Data and [this+0xad0] Num (8-byte UFortAIAssignment* elements). Each assignment's [+0xc8] = Goals (TArray<FFortAIGoal>, element stride 0x28, Actor weak pointer at element+0).
// Pass 1 appends Actor.Get() for each Actor passing the IsValid check (0x142ee7880 / 0x142ee5dd0) to a temp array; pass 2 appends the non-null temp entries to OutGoalActors without clearing it (0x140690bb0 = TArray Add grow).
// CONFIDENCE: MEDIUM
TArray<AActor*> GoalCandidates;
for (UFortAIAssignment* Assignment : EncounterAssignments)
{
	if (!Assignment)
	{
		continue;
	}
	for (const FFortAIGoal& Goal : Assignment->Goals)
	{
		if (Goal.Actor.IsValid())
		{
			GoalCandidates.Add(Goal.Actor.Get());
		}
	}
}
for (AActor* GoalActor : GoalCandidates)
{
	if (GoalActor)
	{
		OutGoalActors.Add(GoalActor);
	}
}
