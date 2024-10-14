#pragma once

typedef struct RGBAValue
{
    double r;
    double g;
    double b;
    double a;

    RGBAValue() { r = 0.0; g = 0.0; b = 0.0; a = 1.0; }
    RGBAValue(double vr, double vg, double vb, double va) { r = vr; g = vg; b = vb; a = va; }
} RGBAValue;

typedef struct Double2DValue
{
    union
    {
        double value[2];
        struct
        {
            double x;
            double y;
        };
    };

    Double2DValue(double vx, double vy) { x = vx; y = vy; }
} Double2DValue;

typedef struct Int2DValue
{
    union
    {
        int value[2];
        struct
        {
            int x;
            int y;
        };
    };

    Int2DValue(int vx, int vy) { x = vx; y = vy; }
} Int2DValue;

// in1, in2 values, Progress [0.0 - 1.0]
double InterpolateBetween2Vals(const double in0, const double in1, float Progress)
{
    double ldy = in1 - in0;
    double _out = in0 + (ldy * Progress);
    return (_out);
}

// col1, col2 RGBA colors, progress [0.0 - 1.0]
RGBAValue interpolate2Colors(const RGBAValue col1, const RGBAValue col2, float progress)
{
    RGBAValue _OUT;
    _OUT.r = InterpolateBetween2Vals(col1.r, col2.r, progress);
    _OUT.g = InterpolateBetween2Vals(col1.g, col2.g, progress);
    _OUT.b = InterpolateBetween2Vals(col1.b, col2.b, progress);
    _OUT.a = InterpolateBetween2Vals(col1.a, col2.a, progress);
    return (_OUT);

}


RGBAValue col2BGRA(const RGBAValue col)
{
    RGBAValue _OUT;
    _OUT.r = col.b;
    _OUT.g = col.g;
    _OUT.b = col.r;
    _OUT.a = col.a;
    return (_OUT);

}

