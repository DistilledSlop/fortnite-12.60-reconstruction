// CONFIDENCE: HIGH (source recovery; build/runtime pending)
// Original projects overlap tests to XY, copies full outer boxes including validity,
// and appends four potentially overlapping strips; output is never reset here.
const FBox& Outer = Delta.OuterWorldBounds;
const FBox& Inner = Delta.InnerWorldBounds;
if (Inner.Min.X <= Outer.Max.X && Outer.Min.X <= Inner.Max.X &&
    Inner.Min.Y <= Outer.Max.Y && Outer.Min.Y <= Inner.Max.Y)
{
    if (Inner.Max.X < Outer.Max.X)
    {
        FBox Part = Outer;
        Part.Min.X = Inner.Max.X;
        OutBoxes.Add(Part);
    }
    if (Inner.Max.Y < Outer.Max.Y)
    {
        FBox Part = Outer;
        Part.Min.Y = Inner.Max.Y;
        OutBoxes.Add(Part);
    }
    if (Outer.Min.X <= Inner.Min.X && Inner.Min.X != Outer.Min.X)
    {
        FBox Part = Outer;
        Part.Max.X = Inner.Min.X;
        OutBoxes.Add(Part);
    }
    if (Outer.Min.Y <= Inner.Min.Y && Inner.Min.Y != Outer.Min.Y)
    {
        FBox Part = Outer;
        Part.Max.Y = Inner.Min.Y;
        OutBoxes.Add(Part);
    }
    return;
}
OutBoxes.Add(Outer);
