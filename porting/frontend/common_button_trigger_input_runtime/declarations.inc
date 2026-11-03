// Original 1407fa9e0 (Blueprint exec 14080d8d0 calls it directly, non-virtual).
// Row argument is FDataTableRowHandle by value; MSVC passes it by hidden pointer (rdx).
public:
    UFUNCTION(BlueprintCallable)
    void SetTriggeringInputAction(FDataTableRowHandle InputActionRow);
