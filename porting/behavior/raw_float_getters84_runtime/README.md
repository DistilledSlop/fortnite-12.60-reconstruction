# Raw-bit float getters

Run `python3 porting/behavior/raw_float_getters84_runtime/prepare.py` after the known-field and getter-storage scans. It examines the four reflected Float returns rejected by the latter solely because their complete inline execs transfer a dword with EAX. A proven free interval would be proposed as neutral `uint32` storage and returned using `FMemory::Memcpy` into a float, preserving all bits without asserting the original source field type. No defaults, initialization, writers, or lifetime are inferred.

Current result: zero proposals. Three `UFortClientSettingsRecord` getters lack a proven immediate `UFortGenericRecord` size. The `UFortUICameraManagerComponent::GetZoomLevel` read at 0x1ac lies inside the conservatively reserved unknown-width `CameraMapping` Map interval 0xc0..0x220. The report records full source bytes, instructions, bounds and rejection reasons. No image digest was recomputed, and no generation, builds or runtime tests were run.
