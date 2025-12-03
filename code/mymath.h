
union V2 {
    struct {R32 x, y;};
    struct {R32 u, v;};
    R32 e[2];
};

union V3 {
    struct {R32 x, y, z;};
    struct {R32 u, v, w;};
    struct {R32 r, g, b;};
    struct {V2 xy; R32 ignored0_;};
    struct {R32 ignored1_; V2 yz;};
    struct {V2 uv; R32 ignored2_;};
    struct {R32 ignored3_; V2 vw;};
    R32 e[3];
};

union V4 {
    struct {
        union {
            V3 xyz;
            struct {R32 x, y, z;};
        };
        R32 w;
    };
    struct {
        union {
            V3 rgb;
            struct {R32 r, g, b;};
        };
        R32 a;
    };
    struct {V2 xy; R32 ignored0_; R32 ignored1_;};
    struct {R32 ignored2_; V2 yz; R32 ignored3_;};
    struct {R32 ignored4_; R32 ignored5_; V2 zw;};
    R32 e[4];
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
