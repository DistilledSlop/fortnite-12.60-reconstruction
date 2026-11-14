"""Inject proven GetCharacterMovement() constructor writes for AFortPawn, AFortPlayerPawn, and AFortPlayerPawnAthena."""
import re

PAWN_WRITES = {
    'AFortPawn': (
        '    if (UCharacterMovementComponent* CharMove = GetCharacterMovement())\n'
        '    {\n'
        '        // Original AFortPawn constructor 0x1422fce30: CharMoveComp writes verified by disassembly.\n'
        '        CharMove->bUpdateOnlyIfRendered = false;\n'
        '        CharMove->NavAgentProps.bCanSwim = false;\n'
        '        CharMove->MaxWalkSpeed = 300.0f;\n'
        '        CharMove->JumpZVelocity = 980.0f;\n'
        '        CharMove->AirControl = 0.9f;\n'
        '    }\n'
    ),
    'AFortPlayerPawn': (
        '    if (UFortMovementComp_Character* FortMove = Cast<UFortMovementComp_Character>(GetCharacterMovement()))\n'
        '    {\n'
        '        // Original AFortPlayerPawn constructor 0x1422fe830: CharMoveComp writes verified by disassembly.\n'
        '        FortMove->bCanWalkOffLedgesWhenCrouching = true;\n'
        '        FortMove->CrouchedHalfHeight = 50.0f;\n'
        '        FortMove->MaxWalkSpeed = 450.0f;\n'
        '        FortMove->MaxWalkSpeedCrouched = 275.0f;\n'
        '        FortMove->MinAnalogWalkSpeed = 200.0f;\n'
        '        FortMove->JumpZVelocity = 560.0f;\n'
        '        FortMove->AirControl = 0.9f;\n'
        '        FortMove->bAlwaysCheckFloor = true;\n'
        '        FortMove->PerchRadiusThreshold = 16.0f;\n'
        '        FortMove->bEnablePhysicsInteraction = false;\n'
        '        FortMove->bApplyGravityWhileJumping = false;\n'
        '        FortMove->MaxSimulationIterations = 4;\n'
        '        FortMove->MaxSimulationTimeStep = 0.1f;\n'
        '        FortMove->bUseRVOAvoidance = true;\n'
        '        FortMove->AvoidanceWeight = 1.0f;\n'
        '        FortMove->bUseControllerDesiredRotation = true;\n'
        '        FortMove->RotationRate = FRotator(0.0f, 36000.0f, 0.0f);\n'
        '        FortMove->bAllowPhysicsRotationDuringAnimRootMotion = true;\n'
        '        FortMove->bUpdatesFloorWhenNotInFullSimulation = true;\n'
        '    }\n'
    ),
    'AFortPlayerPawnAthena': (
        '    if (UFortMovementComp_CharacterAthena* AthenaMove = Cast<UFortMovementComp_CharacterAthena>(GetCharacterMovement()))\n'
        '    {\n'
        '        // Original AFortPlayerPawnAthena constructor 0x141a08ef0: CharMoveComp writes verified by disassembly.\n'
        '        AthenaMove->NavAgentProps.bCanCrouch = true;\n'
        '        AthenaMove->MaxSimulationIterations = 2;\n'
        '        AthenaMove->MaxSimulationTimeStep = 0.5f;\n'
        '        AthenaMove->NetworkSmoothingMode = ENetworkSmoothingMode::Linear;\n'
        '        AthenaMove->bAlwaysCheckFloor = false;\n'
        '        AthenaMove->bUseRVOAvoidance = false;\n'
        '        AthenaMove->bUseControllerDesiredRotation = true;\n'
        '        AthenaMove->RotationRate = FRotator(0.0f, -1.0f, 0.0f);\n'
        '        AthenaMove->bUpdatesFloorWhenNotInFullSimulation = false;\n'
        '        AthenaMove->bUpdatesFloorWhenNotInFullSimulationOnlyOnNetUpdate = true;\n'
        '    }\n'
    ),
}


def finalize(model, header, cpp):
    cls = model.get('name')
    body = PAWN_WRITES.get(cls)
    if body is None:
        return header, cpp
    pattern = re.compile(
        r'\b' + re.escape(cls) + r'::' + re.escape(cls)
        + r'\s*\(\s*const\s+FObjectInitializer\s*&\s*ObjectInitializer\s*\)\s*:[^{};]*\{'
    )
    matches = list(pattern.finditer(cpp))
    if len(matches) != 1:
        raise ValueError(cls + ': expected single FObjectInitializer constructor')
    match = matches[0]
    depth, end = 1, match.end()
    while depth and end < len(cpp):
        if cpp[end] == '{':
            depth += 1
        elif cpp[end] == '}':
            depth -= 1
        end += 1
    if depth:
        raise ValueError(cls + ': unterminated constructor')
    return header, cpp[:end - 1] + body + cpp[end - 1:]
