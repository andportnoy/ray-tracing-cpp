/* Goal: try to use C++ features to create abstractions I like for rendering.
 * In particular:
 * + a vector class that overloads arithmetic operators + has a norm method
 * - a viewport that can be range-iterated, producing viewport points
 * - a screen class that implements the << operator so it can be dumped
 * - maybe an object class that has methods like normal/intersect
 *   where Sphere is a child class
 *
 * Ok, I would like to start by implementing the Vector class.
 * + addition/subtraction vector/vector
 * + scaling
 * + normalization
 * + dot
 *
 * For the screen class, looks like << is for formatted io. I just want a raw
 * binary dump, so I used fwrite.
 */

float lighting(const std::vector<Light>& sources, Vector point, Vector normal) {
	float intensity = 0;
	for (const Light& light: sources) {
		switch (light.type) {
		case LIGHT_AMBIENT:
			intensity += light.intensity;
			break;
		case LIGHT_POSITIONAL:
		case LIGHT_DIRECTIONAL:
		{
			Vector L = light.type==LIGHT_POSITIONAL?
			  light.position-point: light.direction;
			/* now the intensity is proportional to the cosine of
			 * the angle between the normal and the direction of
			 * light
			 * assume the normal has been normalized
			 */
			if (L.norm() == 0.)
				break;
			float cosine = normal.dot(L) / L.norm();
			if (!(cosine<0))
				intensity += light.intensity*cosine;
			break;
		}
		}
	}
	return intensity>1? 1: intensity;
}

void render(Screen& screen, const std::vector<Sphere>& spheres,
  Viewport& viewport, Vector camera, const std::vector<Light>& sources) {
	const Vector O = camera;
	int w = screen.width;
	int h = screen.height;
	for (int x=-(w/2);   x<w/2;   ++x)
	for (int y=-(h/2)+1; y<h/2+1; ++y) {
		/* calculate the viewport point */
		const Vector V {
			viewport.scaling_factor*x,
			viewport.scaling_factor*y,
			viewport.distance,
		};
		Vector D = V - O;
		Sphere const *closest_sphere = nullptr;
		float tmin = INFINITY; /* smallest t>1 */
		for (const Sphere& sphere: spheres) {
			float radius = sphere.radius;
			Vector C = sphere.center;
			/* Want to check for intersection with the sphere.
			 * |P-C| = r^2,
			 * if P is a point on the sphere with center at C and
			 * radius r.
			 * P = O + t(V-O),
			 * if P is a point on the ray coming out of the origin
			 * O through the viewport point V.
			 * Our goal is to solve for t.
			 * |P-C| - r^2
			 *   = <P-C, P-C>
			 *   = <O+t(V-O)-C, O+t(V-O)-C>
			 *   = <O+tD-C, O+tD-C>           (D=V-O)
			 *   = <O-C+tD, O-C+tD>
			 *   = <O-C, O-C> + <O-C, tD> + <tD, O-C> + <tD, tD>
			 *   = <O-C, O-C> + 2t<O-C, D> + (t^2)<D, D>
			 *   = r^2.
			 * So
			 * <O-C, O-C> + 2t<O-C, D> + (t^2)<D, D> - r^2
			 *  = at^2 + bt + c               (a=<D,D>, b=2<O-C,D>,
			 *  = 0.                           c=<O-C,O-C>-r^2)
			 *
			 * The discriminant then is given by disc = b^2 - 4ac.
			 * disc < 0 => no intersection
			 * disc = 0 => 1  intersection
			 * disc > 0 => 2  intersections
			 *
			 * If disc >= 0, then t = (-b +- dis)/(2a).
			 * From this calculation we need to record the smallest
			 * t.
			 */

			/* Ok, so we need to calculate a, b, c to solve for t:
			 * D = V-O  (ray from the origin to viewport point)
			 * CO = O-C (sphere center to the origin)
			 * a = <D, D>
			 * b = 2<O-C, D> = 2<CO, D>
			 * c = <O-C,O-C>-r^2 = <CO,CO>-r^2
			 *
			 * So to compute the above we need vector subtraction
			 * and dot product.
			 */
			Vector CO = O-C;
			float a = D.dot(D);
			float b = 2*CO.dot(D);
			float c = CO.dot(CO) - radius*radius;

			float discriminant = b*b - 4*a*c;
			if (discriminant < 0)
				continue;
			float t1 = (-b + std::sqrt(discriminant))/(2*a);
			float t2 = (-b - std::sqrt(discriminant))/(2*a);

			/* -----O----V----- */

			/* How do we interpret the t's?
			 * t<0 => P is to the left of O
			 * t=0 => P = O
			 * t=1 => P = V
			 * t>1 => P is to the right of V
			 *
			 * Here we only care about points that are to the right
			 * of V because then they are visible through the
			 * viewport.
			 */

			float t = (t1<t2 && t1>1)? t1:
			          (t2<t1 && t2>1)? t2: INFINITY;

			if (t < tmin) {
				tmin = t;
				closest_sphere = &sphere;
			}
			tmin = t<tmin? t: tmin;
		}
		Pixel background {0,0,0};
		Pixel color = background;
		if (closest_sphere) {
			const Sphere closest = *closest_sphere;
			Vector P = O + tmin*D;
			Vector N = (P - closest.center).normalize();
			float intensity = lighting(sources, P, N);
			assert(!(intensity<0) && !(1<intensity));
			color = Pixel {
				static_cast<uint8_t>(
				  std::round(closest.color.r * intensity)),
				static_cast<uint8_t>(
				  std::round(closest.color.g * intensity)),
				static_cast<uint8_t>(
				  std::round(closest.color.b * intensity)),
			};
		}

		screen.put(x, y, color);
	}
}

void scene_update(std::vector<Sphere>& spheres) {
	std::vector<Vector> fields {
		{0,  0, 2},
		{1, -1, 3},
		{1,  1, 4},
	};
	for (Sphere& sphere : spheres) {
		if (sphere.fixed)
			continue;
		/* update position from velocity */
		sphere.center = sphere.center + 0.01*sphere.velocity;
		/* update velocity based on a field */
		for (Vector field : fields) {
			Vector d = field - sphere.center;
			float dot = d.dot(d);
			float len = std::sqrt(dot);
			d = d / len; /* normalize */
			if (len < sphere.radius) { /* bound below */
				len = sphere.radius;
				dot = len*len;
			}
			sphere.velocity = sphere.velocity + 0.3f/dot*d;
		}
	}
}

int main() {
	int w = 640, h = 360;
	Screen screen {h, w};
	std::vector<Sphere> spheres {
		{
		  .center={.1, .1, 4},
		  .velocity={0, 0, 0},
		  .radius=0.2,
		  .color={255,   0,   0},
		  .fixed=0,
		},
		{
		  .center={0, 0, 2},
		  .velocity={0, 0, 0},
		  .radius=0.1,
		  .color={255,   255,   255},
		  .fixed=1,
		},
		{
		  .center={1,  -1, 3},
		  .velocity={0, 0, 0},
		  .radius=0.1,
		  .color={255,   255,   255},
		  .fixed=1,
		},
		{
		  .center={1,   1, 4},
		  .velocity={0, 0, 0},
		  .radius=0.1,
		  .color={255,   255,   255},
		  .fixed=1,
		},
	};
	Viewport viewport = {
		.scaling_factor = 1.f/(w<h? w: h),
		.distance = 1,
	};
	Vector camera {0, 0, 0};
	std::vector<Light> sources {
		{.type=LIGHT_DIRECTIONAL, .intensity=1, .direction={-1, 0, -1}},
	};
	for (int i=0, n=1000; i<n; ++i) {
		render(screen, spheres, viewport, camera, sources);
		screen.dump();
		scene_update(spheres);
		std::cerr << i << "/" << n << std::endl;
	}
}
