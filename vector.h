#ifndef VECTOR2_H
#define VECTOR2_H

struct vector2
{
    float x, y;
};

inline vector2 operator-(vector2 x, vector2 y)
{
    return (vector2){x.x - y.x, x.y - y.y};
}

inline bool operator==(vector2 a, vector2 b)
{
    return (a.x == b.x && a.y == b.y);
}

inline vector2 operator+(vector2 a, vector2 b)
{
    return (vector2){a.x + b.x, a.y + b.y};
}

inline vector2 operator+(vector2 b, float a)
{
    return (vector2){b.x + a, a + b.y};
}

inline vector2 operator*(vector2 a, vector2 b)
{
    return (vector2){a.x * b.x, a.y * b.y};
}

inline vector2 operator*(vector2 a, float b)
{
    return (vector2){a.x * b, a.y * b};
}

inline vector2 operator/(vector2 a, float b)
{
    return (vector2){a.x / b, a.y / b};
}

#endif