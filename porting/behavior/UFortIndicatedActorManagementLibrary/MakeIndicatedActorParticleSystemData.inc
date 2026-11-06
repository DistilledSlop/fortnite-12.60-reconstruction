// CONFIDENCE: high; native capture and reflection contracts reviewed.
{
    FIndicatedActorParticleSystemData Result;
    Result.ParticleSystem = ParticleSystem;
    Result.ParticleSystemActorParamName = ActorParamName;
    Result.ParticleSystemOffset = Offset;
    Result.ParticleSystemDBNOOffset = DBNOOffset;
    Result.ParticleSystemVectorParamName = VectorParamName;
    return Result;
}
