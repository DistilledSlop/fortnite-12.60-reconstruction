public:
    bool NativeIsInteractionEnabled() const;
protected:
    // The original constructor sets both native state bits. These are not UPROPERTYs.
    bool bNativeButtonEnabled = true;
    bool bNativeInteractionEnabled = true;
