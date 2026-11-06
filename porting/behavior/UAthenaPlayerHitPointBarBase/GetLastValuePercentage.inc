// CONFIDENCE: high; Callee divides reflected ValueLast at +0x258 by ValueMax at +0x25c only when ValueMax is positive.
return ValueMax > 0.0f ? ValueLast / ValueMax : 0.0f;
