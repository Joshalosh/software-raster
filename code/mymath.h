
union V2 {
    struct {
        R32 x, y;
    };
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

INTERNAL V2 operator*(V2 b, R32 a) {
    V2 result = a*b;
    return result;
}

INTERNAL V2 &operator*=(V2 &b, R32 a) {
    b = a * b;
    return b;
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

INTERNAL V2 &operator+=(V2 a, V2 b) {
    a = a + b;
    return a;
}

INTERNAL V2 operator-(V2 a, V2 b) {
    V2 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}
