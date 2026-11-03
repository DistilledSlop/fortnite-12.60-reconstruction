# Item evolution restriction text (build pending)

Complete static body and manifest supersede the earlier draft. Native14202d350
switches on EItemEvolutionRestrictionReason: seven exact source/key pairs in the
FortItem namespace, default empty text. Exec142958df0, reflection0x14022401.
Mapped UTF-16 literals were decoded from the original12.60 runtime image; the
complete .inc preserves spellings, punctuation and key names.

Localization helper142bedc40 calls singleton142c274f0 and142c244c0, matching
UE Internationalization/TextCache.cpp FindOrCache and NSLOCTEXT. Fresh probes
native94-text prove the cache structure and source comparison. Default1407c2c80
returns a static FText initialized through142c0c150. Native96-deps/142c0c150
constructs thread-safe shared empty FString storage, localized text data and
shared owner with FText flags0, matching UE Core/Private/Internationalization/
Text.cpp FText(EInitToEmptyString) lines227-231 and FText::GetEmpty singleton.
No build/runtime/tests; next checkpoint3,040 bodies.
