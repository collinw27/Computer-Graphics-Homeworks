
#include <cmath>
#include <iostream>
#include <vector>
#include <string>

struct Color
{
    unsigned char r, g, b;
};

class Vec3
{
public:

    float x, y, z;

    Vec3() : x{0}, y{0}, z{0} {}
    Vec3(float x, float y, float z) : x{x}, y{y}, z{z} {}

    Vec3 operator-()
    {
        return Vec3(-x, -y, -z);
    }

    Vec3 operator+(const Vec3& other)
    {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }

    Vec3 operator-(const Vec3& other)
    {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    Vec3 operator*(float scalar)
    {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }

    float dot(const Vec3& other)
    {
        return x * other.x + y * other.y + z * other.z;
    }

    Vec3 cross(const Vec3& other)
    {
        return Vec3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    float length()
    {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vec3 normalized()
    {
        float l = length();
        return Vec3(x / l, y / l, z / l);
    }

    std::string str()
    {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
    }
};

struct Sphere
{
    Vec3 center;
    float radius;
    Color color;
};

struct Light
{
    Vec3 position;
    float intensity;
};

// Used for neatly passing information into the update function

struct UpdateInfo
{
    float deltaTime;
    bool keyW, keyA, keyS, keyD, keySPACE, keySHIFT;
};

// Handles the program logic of the ray tracer
// This allows it to be kept independent of GLFW code and
// makes it more readable

class RayTracer
{
    Vec3 viewpoint;
    Vec3 lookAt;
    Vec3 upVec;
    float viewW;
    float viewH;
    float projDist; // Perspective-specific 
    bool doPerspective;

    // Scene objects

    std::vector<Sphere*> spheres;
    std::vector<Light*> lights;

public:

    RayTracer();
    void update(const UpdateInfo& info);
    void createImage(unsigned char* image, int width, int height);
};

RayTracer::RayTracer()
{
    // Set up scene parameters

    viewpoint = Vec3(0, 0, 0);
    lookAt = Vec3(0, 0, -1);
    upVec = Vec3(0, 1, 0);
    viewW = 8.0;
    viewH = 8.0;
    projDist = 5.0; 
    doPerspective = true;

    // Create scene objects

    spheres.push_back(new Sphere{Vec3(0.0, -2.0, -7.0), 1.0, Color{100, 255, 100}});
    spheres.push_back(new Sphere{Vec3(0.0, -2.0, -5.0), 1.0, Color{100, 100, 255}});
    spheres.push_back(new Sphere{Vec3(0.0, -2.0, -3.0), 1.0, Color{255, 100, 100}});
    lights.push_back(new Light{Vec3(-5.0, 5.0, -5.0), 5.0});
}

void RayTracer::createImage(unsigned char* image, int width, int height)
{
    // Infer other vectors

    Vec3 e = viewpoint;
    Vec3 w = -lookAt;
    Vec3 v = upVec;
    Vec3 u = v.cross(w).normalized();

    // Calculate image

    for(int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            // Calculate center and direction of ray
            // Changes depending on perspective/orthographic
            // D must be a unit vector due to how the quadratic
            // calculations have been simplified

            float u_scalar = ((float)x / width - 0.5) * viewW;
            float v_scalar = ((float)y / height - 0.5) * viewH;
            Vec3 S = doPerspective ? e : (e + u * u_scalar + v * v_scalar);
            Vec3 D = doPerspective ? (-w * projDist + u * u_scalar + v * v_scalar) : -w;
            D = D.normalized();

            // Determine color using sphere intersection
            // Takes color from closest sphere

            Sphere* closest = nullptr;
            float smallestDist = 9999.0;
            for (Sphere* sphere : spheres)
            {
                // From ray tracing Wikipedia (and class lecture):
                // t^2 + (2V . D)t + (V^2 - r^2) = 0, where
                // Sphere: |X - C|^2 = r^2
                // Ray: R(t) = S + Dt
                // As a shorthand, V = S - C

                Vec3 V (S - sphere->center);
                float quadB = 2 * V.dot(D);
                float quadC = V.dot(V) - (sphere->radius * sphere->radius);

                // Check discriminant to test for intersection

                float disc = quadB * quadB - 4 * quadC;
                if (disc >= 0)
                {
                    // (-b +- disc)/2a (a = 1)
                    // Check both +- for being closest

                    float t = 0.5 * (-quadB - std::sqrt(disc));
                    if (t >= 0 && t < smallestDist)
                    {
                        smallestDist = t;
                        closest = sphere;
                    }
                    else
                    {
                        t = 0.5 * (-quadB + std::sqrt(disc));
                        if (t >= 0 && t < smallestDist)
                        {
                            smallestDist = t;
                            closest = sphere;
                        }
                    }
                }
            }

            // Use color of closest sphere (if applicable)
            
            Color col = Color{100, 100, 100};
            if (closest != nullptr)
            {
                col = closest->color;
            }

            // Set array to color

            int idx = (y * width + x) * 3;
            image[idx] = col.r;
            image[idx+1] = col.g;
            image[idx+2] = col.b;
        }
    }
}

void RayTracer::update(const UpdateInfo& info)
{
    constexpr float SPEED = 3.0;
    if (info.keyW)
        viewpoint = viewpoint + Vec3(0.0, 0.0, -SPEED * info.deltaTime);
    if (info.keyA)
        viewpoint = viewpoint + Vec3(-SPEED * info.deltaTime, 0, 0);
    if (info.keyS)
        viewpoint = viewpoint + Vec3(0.0, 0.0, SPEED * info.deltaTime);
    if (info.keyD)
        viewpoint = viewpoint + Vec3(SPEED * info.deltaTime, 0, 0);
    if (info.keySPACE)
        viewpoint = viewpoint + Vec3(0.0, SPEED * info.deltaTime, 0);
    if (info.keySHIFT)
        viewpoint = viewpoint + Vec3(0.0, -SPEED * info.deltaTime, 0);
}