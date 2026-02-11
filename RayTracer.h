#include <cmath>
#include <iostream>
#include <vector>
#include <string>

#include "SFML/Graphics.hpp"

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

    // Rotates the x and z, ignoring the vertical component

    Vec3 rotateXZ(float angle)
    {
        return Vec3(x * std::cos(angle) - z * std::sin(angle), y, x * std::sin(angle) + z * std::cos(angle));
    }

    std::string str()
    {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
    }

    Color toColor()
    {
        return Color{(unsigned char)std::min(255, (int)(x)),
            (unsigned char)std::min(255, (int)(y)),
            (unsigned char)std::min(255, (int)(z))
        };
    }

    static Vec3 fromColor(const Color& color)
    {
        return Vec3(color.r, color.g, color.b);
    }
};

class Material
{
public:

    Color color;
    float kD = 0.3; // Diffuse coefficient
    float kS = 0.1; // Specular coefficient
    float N = 10.0;
    float glaze = -1.0; // Set to value in [0, 1] to mix color with reflected light

    Material(Color color) : color{color} {}
};

class Shape3D
{
public:

    Material material;

    Shape3D(Material material) : material{material} {}
    virtual Vec3 getNormal(Vec3 intersectPos) = 0;
    virtual float checkIntersection(Vec3 S, Vec3 D) = 0;
};

class Sphere : public Shape3D
{
    Vec3 center;
    float radius;

public:

    Sphere(Vec3 center, float radius, Material material) : Shape3D{material}
    {
        this->center = center;
        this->radius = radius;
    }

    // Returns a positive t if intersected, otherwise negative

    virtual float checkIntersection(Vec3 S, Vec3 D) override
    {
        Vec3 V (S - center);
        float quadB = 2 * V.dot(D);
        float quadC = V.dot(V) - (radius * radius);

        // Check discriminant to test for intersection
        // Only smallest t is checked (invisible from inside)
        // t = (-b - disc)/2a (a = 1)

        float disc = quadB * quadB - 4 * quadC;
        if (disc >= 0)
            return 0.5 * (-quadB - std::sqrt(disc));
        else
            return -1.0;
    }

    virtual Vec3 getNormal(Vec3 intersectPos) override
    {
        return (intersectPos - center).normalized();
    }
};

class Plane : public Shape3D
{
    Vec3 origin;
    Vec3 normal;

public:

    Plane(Vec3 origin, Vec3 normal, Material material) : Shape3D{material}
    {
        this->origin = origin;
        this->normal = normal;
    }

    virtual float checkIntersection(Vec3 S, Vec3 D) override
    {
        // Using implicit plane equation and explicit ray equation:
        // (origin + tD - S) . normal = 0
        // Simplifying: t = (origin - S) . N / (D . N)

        // Don't show if viewed from behind

        if (D.dot(normal) > 0)
            return -1;

        float denominator = D.dot(normal);
        if (denominator == 0.0)
            return -1;
        else
            return (origin - S).dot(normal) / denominator;
    }

    virtual Vec3 getNormal(Vec3) override
    {
        return normal;
    }
};

struct Light
{
    Vec3 position;
    float intensity;
    float ambient;
};

// Used for neatly passing information into the update function

struct UpdateInfo
{
    float deltaTime;
    bool keyW, keyA, keyS, keyD, keySPACE, keySHIFT, keyE, keyR, keyP;
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

    std::vector<Shape3D*> shapes;
    std::vector<Light*> lights;

    // Misc constants

    Color getRayColor(Vec3 S, Vec3 D, int recursionLevel);

public:

    RayTracer();
    void update(const UpdateInfo& info);
    void createImage(unsigned char* image, int width, int height);
    void saveImage(const std::string& filename, unsigned char* image, int width, int height);
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

    shapes.push_back(new Sphere(Vec3(0.0, -2.0, -3.0), 1.0, Color{255, 100, 100}));
    shapes.push_back(new Sphere(Vec3(-3.0, -1.4, -3.0), 1.6, Color{100, 255, 100}));
    lights.push_back(new Light{Vec3(-5.0, 2.0, -3.0), 3.0, 0.2});
    lights.push_back(new Light{Vec3(5.0, 5.0, -5.0), 0.4, 0.1});

    Material blueMat{Color{100, 100, 255}};
    blueMat.kS = 0.3;
    blueMat.N = 30;
    shapes.push_back(new Sphere(Vec3(0.0, -2.0, -5.0), 1.0, blueMat));
    
    Material planeMat{Color{80, 80, 80}};
    planeMat.kS = 0.0;
    planeMat.glaze = 0.2;
    shapes.push_back(new Plane(Vec3(0.0, -3.0, 0.0), Vec3(0.0, 1.0, 0.0), planeMat));
}

void RayTracer::update(const UpdateInfo& info)
{
    // Translate camera
    // WASD depends on camera rotation

    constexpr float SPEED = 5.0;
    if (info.keySPACE)
        viewpoint = viewpoint + Vec3(0.0, SPEED * info.deltaTime, 0);
    if (info.keySHIFT)
        viewpoint = viewpoint + Vec3(0.0, -SPEED * info.deltaTime, 0);
    Vec3 translate {};
    if (info.keyW)
        translate = translate + Vec3(0.0, 0.0, -1.0);
    if (info.keyA)
        translate = translate + Vec3(-1.0, 0.0, 0.0);
    if (info.keyS)
        translate = translate + Vec3(0.0, 0.0, 1.0);
    if (info.keyD)
        translate = translate + Vec3(1.0, 0.0, 0.0);
    translate = translate.rotateXZ(std::atan2(lookAt.x, -lookAt.z));
    viewpoint = viewpoint + translate * SPEED * info.deltaTime;

    // Rotate camera

    constexpr float RSPEED = 2.0;
    if (info.keyE || info.keyR)
    {
        float angle = std::atan2(lookAt.z, lookAt.x);
        angle += (info.keyE ? -RSPEED : RSPEED) * info.deltaTime;
        lookAt.x = std::cos(angle);
        lookAt.z = std::sin(angle);
    }

    // Toggle perspective

    if (info.keyP)
    {
        doPerspective = !doPerspective;
    }
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
            // D must be a unit vector due to how the quadratic has been simplified

            float u_scalar = ((float)x / width - 0.5) * viewW;
            float v_scalar = ((float)y / height - 0.5) * viewH;
            Vec3 S = doPerspective ? e : (e + u * u_scalar + v * v_scalar);
            Vec3 D = doPerspective ? (-w * projDist + u * u_scalar + v * v_scalar) : -w;
            D = D.normalized();

            // Get color of ray
            // This is delegated to another method to allow for recursive sampling

            Color col = getRayColor(S, D, 0);

            // Set array to color

            int idx = (y * width + x) * 3;
            image[idx] = col.r;
            image[idx+1] = col.g;
            image[idx+2] = col.b;
        }
    }
}

void RayTracer::saveImage(const std::string& filename, unsigned char* image, int width, int height)
{
    sf::Image img = sf::Image(sf::Vector2u(width, height));
    for (int x = 0; x < width; x++)
        for (int y = 0; y < width; y++)
        {
            int idx = ((height - 1 - y) * width + x) * 3;
            img.setPixel(sf::Vector2u(x, y), sf::Color(image[idx], image[idx+1], image[idx+2]));
        }
    bool _result = img.saveToFile(std::filesystem::path(filename));
}

Color RayTracer::getRayColor(Vec3 S, Vec3 D, int recursionLevel)
{
    // Determine color using sphere intersection
    // Takes color from closest sphere

    Shape3D* closest = nullptr;
    float smallestDist = 9999.0;
    for (Shape3D* shape : shapes)
    {
        // From ray tracing Wikipedia (and class lecture):
        // t^2 + (2V . D)t + (V^2 - r^2) = 0, where
        // Sphere: |X - C|^2 = r^2
        // Ray: R(t) = S + Dt
        // As a shorthand, V = S - C

        float t = shape->checkIntersection(S, D);
        if (t >= 0 && t < smallestDist)
        {
            smallestDist = t;
            closest = shape;
        }
    }

    // Work out pixel color if closest sphere was found
    
    Color col = Color{100, 100, 100};
    if (closest != nullptr)
    {
        Material& material = closest->material;
        float intensity = 0.0;
        float specIntensity = 0.0;

        // Calculate diffuse light
        // Start by finding normal vector of intersection point

        Vec3 intersectPos = S + D * smallestDist;
        Vec3 vE = (viewpoint - intersectPos).normalized();
        Vec3 normal = closest->getNormal(intersectPos);
        for (Light* light : lights)
        {
            // Shadow
            // Find distance from light to current object
            // Any object with smaller distance will block it (0.01 used for bias)

            bool isShadow = false;
            Vec3 vL = (light->position - intersectPos).normalized();
            float shadowDist = closest->checkIntersection(light->position, -vL) - 0.01;
            for (Shape3D* shadowShape : shapes)
            {
                float t = shadowShape->checkIntersection(light->position, -vL);
                if (t >= 0 && t < shadowDist)
                {
                    isShadow = true;
                    break;
                }
            }

            // Ambient included regardless of shadow
            
            intensity += light->ambient;
            if (!isShadow)
            {
                // Diffuse & specular

                intensity += material.kD * light->intensity * std::max(0.f, normal.dot(vL));
                Vec3 vH = (vL + vE).normalized();
                specIntensity += material.kS * light->intensity * std::pow(std::max(0.f, normal.dot(vH)), material.N);
                intensity += specIntensity;
            }
        }

        // Normal intensity adds object color
        // Specular intensity adds white light
        // Shadow takes priority
        
        intensity = std::min(1.f, intensity);
        specIntensity = std::min(1.f, specIntensity);
        Vec3 colVec = Vec3::fromColor(material.color) * intensity;
        colVec = colVec + Vec3(255, 255, 255) * specIntensity;
        col = colVec.toColor();

        // Glazed surface: shoot a second ray, and mix with final color

        if (material.glaze > 0.0 && recursionLevel < 1)
        {
            Vec3 vR = normal * normal.dot(vE) * 2.0 - vE;
            Color reflectedCol = getRayColor(intersectPos, vR, recursionLevel + 1);
            col = (colVec * (1 - material.glaze) + Vec3::fromColor(reflectedCol) * material.glaze).toColor();
        }
    }
    return col;
}