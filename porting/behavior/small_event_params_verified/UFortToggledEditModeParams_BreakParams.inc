// CONFIDENCE: HIGH; captured helper copies each reflected field to its output.
_PlayerThatToggledEditMode = PlayerThatToggledEditMode;
// The emitter renames the output to InEditableActor to avoid the member-name collision.
InEditableActor = EditableActor;
InEditMode = bInEditMode;
