float Vector::norm(void) const {
	return std::sqrt(x*x + y*y + z*z);
}

Vector Vector::normalize (void) const {
	return *this / this->norm();
}

std::ostream& operator<<(std::ostream& os, const Vector& v) {
	os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
	return os;
}

Vector& Vector::operator+=(const Vector& v) {
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

Vector& Vector::operator-=(const Vector& v) {
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

Vector& Vector::operator*=(float f) {
	x *= f;
	y *= f;
	z *= f;
	return *this;
}

Vector& Vector::operator/=(float f) {
	x /= f;
	y /= f;
	z /= f;
	return *this;
}

Vector operator+(Vector lhs, const Vector& rhs) { return lhs += rhs; }
Vector operator-(Vector lhs, const Vector& rhs) { return lhs -= rhs; }

Vector operator*(Vector v, float f) { return v *= f; }
Vector operator*(float f, Vector v) { return v *= f; }
Vector operator/(Vector v, float f) { return v /= f; }

float Vector::dot(const Vector& v) { return x*v.x + y*v.y + z*v.z; }

void Screen::screen_pixel_set(int y, int x, const Pixel& pixel) {
	if ((y < 0) || !(y < height) || (x < 0) || !(x < width))
		std::cerr << "(" << y << "," << x << ") out of bounds for "
		  << height << "x" << width << " screen" << std::endl;
	this->pixel[y*width + x] = pixel;
}

void Screen::canvas_pixel_set(int x, int y, const Pixel& pixel) {
	int screenx =  x + width/2;
	int screeny = -y + height/2;
	screen_pixel_set(screeny, screenx, pixel);
}

void Screen::put(int x, int y, const Pixel& pixel) {
	canvas_pixel_set(x, y, pixel);
}

void Screen::dump(FILE *f) {
	fprintf(f, "P6\n");
	fprintf(f, "%d %d\n", width, height);
	fprintf(f, "255\n");
	fwrite(pixel.get(), height*width*sizeof(Pixel), 1, f);
}

void Screen::dump(void) {
	dump(stdout);
}
