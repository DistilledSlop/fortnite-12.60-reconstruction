# Ability handle access bridge

Original weapon predicates `1426a5d60` and `1426a5c50` call IsActive, re-read ActiveAbility, then directly compare its current spec handle. Target Engine `UGameplayAbility::GetCurrentAbilitySpecHandle()` adds `check(IsInstantiated())`; IsActive can return true for a non-instanced ability, so that checked accessor changes original shipping behavior in a development server.

The generated UFortGameplayAbility subclass exposes its inherited protected CurrentSpecHandle through a public inline synthetic accessor. This is ordinary typed access without layout assumptions or casts. The policy pins class identity/parent, rejects name collisions and checks exact rendered access. It is deliberately absent from the behavior manifest and original-function accounting.
