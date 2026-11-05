"""Restore the reflected native initialization-interface contract."""

def interface_preamble(name):
    return []


def interface_declarations(name):
    if name != 'UFortInitializationInterface':
        return []
    # UHT supplies the virtual destructor and UObject owner getter.
    return [
        'public:',
        '    UFUNCTION()',
        '    virtual bool NeedsWorldReady() const = 0;',
        '    UFUNCTION()',
        '    virtual void WorldReady() = 0;',
        '    UFUNCTION()',
        '    virtual void MatchStarted() = 0;',
    ]
