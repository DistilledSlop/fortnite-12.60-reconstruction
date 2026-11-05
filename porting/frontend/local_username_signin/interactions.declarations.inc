protected:
    virtual void NativeConstruct() override;
    virtual void NativeHandleActivated() override;
    virtual void NativeHandleDeactivated() override;
private:
    bool SelectLocalCredentials();
