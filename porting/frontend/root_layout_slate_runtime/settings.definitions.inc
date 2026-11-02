bool GFortUIEnableLetoBorder = true;
float GFortUILetoBorderWidth = 36.0f;
float GFortUILetoBorderedContentScale = 0.95f;

static FAutoConsoleVariableRef CVarFortUIEnableLetoBorder(
	TEXT("FortUI.EnableLetoBorder"), GFortUIEnableLetoBorder, TEXT(""), ECVF_Default);
static FAutoConsoleVariableRef CVarFortUILetoBorderWidth(
	TEXT("FortUI.LetoBorderWidth"), GFortUILetoBorderWidth, TEXT(""), ECVF_Default);
static FAutoConsoleVariableRef CVarFortUILetoBorderedContentScale(
	TEXT("FortUI.LetoBorderedContentScale"), GFortUILetoBorderedContentScale, TEXT(""), ECVF_Default);
