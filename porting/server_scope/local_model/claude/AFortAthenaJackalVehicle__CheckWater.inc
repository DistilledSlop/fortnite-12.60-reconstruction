// CONFIDENCE: LOW. Original exec 0x1427dade0; native body 0x1426f9960 kept as a direct helper call.
// Parameters are read in declaration order: WaterLocationZ (float), WaterFX (UParticleSystemComponent), WaterAudio (UFortVehicleAudioVoice).
// The body computes the water surface normal at the vehicle location, orients WaterFX to it, pushes its vector parameter,
// and, when WaterAudio is present, sets the audio volume to the vehicle speed scaled by 0.05 and clamped to [0, 1].
UpdateWaterSurfaceFX(WaterLocationZ, WaterFX, WaterAudio);
