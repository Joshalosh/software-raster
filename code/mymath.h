
union V2 {
    struct {R32 x, y;};
    struct {R32 u, v;};
    R32 e[2];
};

INTERNAL V2 Vec2(R32 x, R32 y) {
    V2 result;
    result.x = x;
    result.y = y;
    return result;
}

INTERNAL V2 operator*(R32 a, V2 b) {
    V2 result;
    result.x = a * b.x;
    result.y = a * b.y;
    return result;
}

INTERNAL V2 operator*(V2 a, R32 b) {
    V2 result = b*a;
    return result;
}

INTERNAL V2 &operator*=(V2 &a, R32 b) {
    a.x *= b;
    a.y *= b;
    return a;
}


INTERNAL V2 operator-(V2 a) {
    V2 result;
    result.x = -a.x;
    result.y = -a.y;
    return result;
}

INTERNAL V2 operator+(V2 a, V2 b) {
    V2 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

INTERNAL V2 &operator+=(V2 &a, V2 b) {
    a.x += b.x;
    a.y += b.y;
    return a;
}

INTERNAL V2 operator-(V2 a, V2 b) {
    V2 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}
