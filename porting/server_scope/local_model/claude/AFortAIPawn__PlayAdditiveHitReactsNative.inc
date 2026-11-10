// CONFIDENCE: MEDIUM. Plays an additive hit-react montage whose section is chosen from the hit direction (Forward, Back, Left or Right).
// Original address: 0x1427a5650 (exec). Native body: 0x1422e2ff0 (direction pick 0x14234c7b0, montage play 0x142341760).
static const FName ForwardName(TEXT("Forward"));
static const FName BackName(TEXT("Back"));
static const FName LeftName(TEXT("Left"));
static const FName RightName(TEXT("Right"));

// Angle-based pick: |yaw| <= 45 deg -> Forward; 45..120 deg -> Right (positive yaw) or Left; otherwise Back.
const FName Direction = PickHitReactDirection(HitDirection, ForwardName, BackName, LeftName, RightName);
PlayAdditiveMontage(NewAnimMontage, 1.0f, Direction, false);
AIFlags1340 |= 0x04;
