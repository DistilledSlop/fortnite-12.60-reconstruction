	FFortDialogDescription_NUI()
		: AdditionalContent(nullptr)
		, LeftAdditionalContent(nullptr)
		, DisplayTime(0.0f)
		, Dismissable(true)
		, Cancelable(false)
		, bShouldWaitForLatentActionOnConfirmAction(false)
		, NotificationHandler(nullptr)
		, ShowSound(nullptr)
	{
	}

	// Project-local names; the original source identifiers were not recovered.
	FNativeConfirmationResult NativeResultCallback;
	FNativeAdvancedConfirmationResult NativeAdvancedResultCallback;
