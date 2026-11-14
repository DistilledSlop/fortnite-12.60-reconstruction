"""Typed default construction recovered from 0x141f7dbe0."""

_CONTRACTS = {
    'FFortZoneInstanceInfo': ('/Script/FortniteGame', 104, 8, None, [
        (0, 'FString WorldId;'), (16, 'FString TheaterId;'),
        (32, 'FString TheaterMissionId;'), (48, 'FString TheaterMissionAlertId;'),
        (64, 'TSoftClassPtr<UFortZoneTheme> ZoneThemeClass;'),
    ]),
    'FFortZoneInstanceDetails': ('/Script/FortniteGame', 296, 8, 'FFortZoneInstanceInfo', [
        (104, 'TSoftClassPtr<UFortMissionGenerator> MissionGenerator;'),
        (144, 'FDataTableRowHandle ZoneDifficultyInfo;'),
        (160, 'FMcpLootResult MissionRewards;'),
        (192, 'TArray<FMcpLootResult> DifficultyIncreaseMissionRewards;'),
        (208, 'FMcpLootResult MissionAlertRewards;'),
        (240, 'FMcpLootResult ZoneModifiers;'),
        (272, 'FString MissionAlertCategoryName;'), (288, 'int32 TileIndex;'),
    ]),
    'FMcpLootResult': ('/Script/McpProfileSys', 32, 8, None, [
        (0, 'FString TierGroupName;'), (16, 'TArray<FMcpLootEntry> Items;'),
    ]),
}


def declarations(model):
    name = model['cpp_name']
    contract = _CONTRACTS.get(name)
    if contract is None:
        return []
    actual = (model['package'], model['size'], model['align'], model['parent'],
              [(m['offset'], m['decl']) for m in model['members']])
    if actual != contract:
        raise ValueError(name + ': native default-construction field contract changed')
    if name != 'FFortZoneInstanceDetails':
        return []
    return [
        '\t// Native 0x141f7dbe0; typed members default-construct to empty/null.',
        '\tFFortZoneInstanceDetails() : TileIndex(INDEX_NONE) {}',
        '',
    ]
