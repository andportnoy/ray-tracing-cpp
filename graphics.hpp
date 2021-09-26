class Vector {
public:
	float x, y, z;
	float norm (void) const;
	Vector normalize (void) const;
	Vector& operator+=(const Vector&);
	Vector& operator-=(const Vector&);
	Vector& operator*=(float);
	Vector& operator/=(float);
	float dot(const Vector&);
};
std::ostream& operator<<(std::ostream&, const Vector&);
Vector operator+(Vector, const Vector&);
Vector operator-(Vector, const Vector&);
Vector operator*(Vector, float);
Vector operator*(float, Vector);
Vector operator/(Vector, float);


class Pixel {
public:
	uint8_t r, g, b;
};


class Screen {
public:
	int height, width;
	Screen(int h, int w) : height{h}, width{w} {
		pixel = std::make_unique<Pixel[]>(h*w); }
	void put(int x, int y, const Pixel&);
	void canvas_pixel_set(int x, int y, const Pixel&);
	void dump(FILE *f);
	void dump(void);
private:
	std::unique_ptr<Pixel[]> pixel;
	void screen_pixel_set(int y, int x, const Pixel&);
};

class Sphere {
public:
	Vector center;
	Vector velocity;
	float radius;
	Pixel color;
	int fixed;
};

class Viewport {
public:
	float scaling_factor; /* world units per pixel */
	float distance;       /* from camera */
};

enum lighttype { LIGHT_AMBIENT, LIGHT_DIRECTIONAL, LIGHT_POSITIONAL };
class Light {
public:
	enum lighttype type;
	float intensity;
	union {
		Vector direction;   /* directional */
		Vector position;    /* positional */
	};
};
