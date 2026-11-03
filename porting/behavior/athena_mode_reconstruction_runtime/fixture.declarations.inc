public:
    // Existing Linux fixture/runtime support; original native bodies remain open.
    // Definitions stay in Private/FortniteAthenaGameplay.cpp.
    AFortGameModeAthena();
    virtual void StartPlay() override;
    virtual void RestartPlayer(AController* NewPlayer) override;

private:
    void PositionPawnOnTerrain(AController* Player);
    void SpawnApolloBackground();
