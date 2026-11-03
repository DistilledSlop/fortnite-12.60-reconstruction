// CONFIDENCE: high
if (expiration_date == FDateTime(0))
{
    return true;
}
return FDateTime::UtcNow() < expiration_date;
