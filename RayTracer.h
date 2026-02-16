#include <cmath>
#include <iostream>
#include <vector>
#include <string>

// #define INCLUDE_SFML

#ifdef INCLUDE_SFML
#include "SFML/Graphics.hpp"
#endif

constexpr float pi = 3.1415926535;

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

    Vec3 operator*(const Vec3& other)
    {
        return Vec3(x * other.x, y * other.y, z * other.z);
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

struct Material
{
    Color color;
    float kD = 0.3; // Diffuse coefficient
    float kS = 0.1; // Specular coefficient
    float N = 10.0;
    float glaze = -1.0; // Set to value in [0, 1] to mix color with reflected light
    float translucent = false;
};

struct IntersectData
{
    float t;
    Vec3 normal;
};

class Shape3D
{
public:

    Material material;

    Shape3D(Material material) : material{material} {}
    virtual IntersectData checkIntersection(Vec3 S, Vec3 D) = 0;
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

    virtual IntersectData checkIntersection(Vec3 S, Vec3 D) override
    {
        Vec3 V (S - center);
        float quadB = 2 * V.dot(D);
        float quadC = V.dot(V) - (radius * radius);

        // Check discriminant to test for intersection
        // Only smallest t is checked (invisible from inside)
        // t = (-b - disc)/2a (a = 1)

        float disc = quadB * quadB - 4 * quadC;
        if (disc >= 0)
        {
            float t = 0.5 * (-quadB - std::sqrt(disc));
            return IntersectData{t, (S + D * t - center).normalized()};
        }
        else
            return IntersectData{-1, Vec3()};
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

    virtual IntersectData checkIntersection(Vec3 S, Vec3 D) override
    {
        // Using implicit plane equation and explicit ray equation:
        // (origin + tD - S) . normal = 0
        // Simplifying: t = (origin - S) . N / (D . N)

        // Don't show if viewed from behind

        if (D.dot(normal) > 0)
            return IntersectData{-1, Vec3()};

        float denominator = D.dot(normal);
        if (denominator == 0.0)
            return IntersectData{-1, Vec3()};
        else
            return IntersectData{(origin - S).dot(normal) / denominator, normal};
    }
};

class PlaneTriangle : public Shape3D
{
    // v1, v2, v3 specify vertices
    // Should be specified in counterclockwise order

    Vec3 v1, v2, v3;
    Vec3 normal;

public:

    PlaneTriangle(Vec3 v1, Vec3 v2, Vec3 v3, Material material)
        : Shape3D{material}, v1{v1}, v2{v2}, v3{v3}
    {
        normal = (v2 - v1).cross(v3 - v1).normalized();
    }

    virtual IntersectData checkIntersection(Vec3 S, Vec3 D) override
    {
        // See Plane for plane intersection logic

        if (D.dot(normal) > 0)
            return IntersectData{-1, Vec3()};
        float denominator = D.dot(normal);
        if (denominator == 0.0)
            return IntersectData{-1, Vec3()};
        float t = (v1 - S).dot(normal) / denominator;
        Vec3 x = S + D * t;

        // Check if intersection point is within bounds of triangle

        return (
            ((v2 - v1).cross(x - v1).dot(normal) > 0) &&
            ((v3 - v2).cross(x - v2).dot(normal) > 0) &&
            ((v1 - v3).cross(x - v3).dot(normal) > 0)
        )
            ? IntersectData{t, normal}
            : IntersectData{-1, Vec3()};
    }
};

class Tetrahedron : public Shape3D
{
    PlaneTriangle t1, t2, t3, t4;

public:

    // t1, t2, t3 should be counterclockwise when viewed from bottom
    // t4 should be top vertex

    Tetrahedron(Vec3 v1, Vec3 v2, Vec3 v3, Vec3 v4, Material material)
        : Shape3D{material},
        t1{v1, v2, v3, material},
        t2{v1, v3, v4, material},
        t3{v3, v2, v4, material},
        t4{v2, v1, v4, material}
    {}

    virtual IntersectData checkIntersection(Vec3 S, Vec3 D) override
    {
        // Since we are not rendering back-faces, finding any intersection is sufficient

        IntersectData data = t1.checkIntersection(S, D);
        if (data.t >= 0)
            return data;
        data = t2.checkIntersection(S, D);
        if (data.t >= 0)
            return data;
        data = t3.checkIntersection(S, D);
        if (data.t >= 0)
            return data;
        return t4.checkIntersection(S, D);
    }
};

struct Light
{
    Vec3 direction;
    float intensity;
    float ambient;
};

// Used for neatly passing information into the update function

struct UpdateInfo
{
    float deltaTime;
    bool keyW, keyA, keyS, keyD, keySPACE, keySHIFT, keyE, keyR, keyP, keyC;
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

    Color getRayColor(Vec3 S, Vec3 D, int recursionLevel);
    void saveImage(const std::string& filename, unsigned char* image, int width, int height);

public:

    RayTracer();
    void update(const UpdateInfo& info);
    void createImage(unsigned char* image, int width, int height);
    void renderFrame(int frame, unsigned char* image, int width, int height);
};

RayTracer::RayTracer()
{
    // Set up scene parameters

    viewpoint = Vec3(0, 0, 0);
    lookAt = Vec3(0, 0, -1);
    upVec = Vec3(0, 1, 0);
    // viewpoint = Vec3(0, 0, 4);
    // lookAt = Vec3(0, -0.2, -1).normalized();
    // upVec = lookAt.cross(Vec3(-1, 0, 0)).normalized();
    viewW = 6.0;
    viewH = 6.0;
    projDist = 5.0; 
    doPerspective = true;

    // Create scene objects

    Material redMat{Color{255, 100, 100}};
    Material greenMat{Color{100, 255, 100}};
    greenMat.kS = 0.2;
    greenMat.N = 20;
    Material blueMat{Color{100, 100, 255}};
    blueMat.kS = 0.3;
    blueMat.N = 30;
    Material translucentMat{Color{80, 255, 255}};
    translucentMat.translucent = true;
    Material planeMat{Color{80, 80, 80}};
    planeMat.kS = 0.0;
    planeMat.glaze = 0.2;

    shapes.push_back(new Tetrahedron(Vec3(1, -3, -3), Vec3(4, -3, -2), Vec3(1, -3, 0), Vec3(2.5, 0, -2), redMat));
    shapes.push_back(new Sphere(Vec3(-2.0, -1.4, -3.0), 1.6, greenMat));
    shapes.push_back(new Sphere(Vec3(1.0, -2.0, -5.0), 1.0, blueMat));
    // shapes.push_back(new Sphere(Vec3(5.0, -1.0, -4.0), 2.0, translucentMat));
    shapes.push_back(new Plane(Vec3(0.0, -3.0, 0.0), Vec3(0.0, 1.0, 0.0), planeMat));
    lights.push_back(new Light{Vec3(5.0, -4.0, 3.0), 3.0, 0.2});
    lights.push_back(new Light{Vec3(-5.0, -5.0, 5.0), 0.4, 0.1});
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
        doPerspective = !doPerspective;
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
    #ifdef INCLUDE_SFML

    sf::Image img = sf::Image(sf::Vector2u(width, height));
    for (int x = 0; x < width; x++)
        for (int y = 0; y < width; y++)
        {
            int idx = ((height - 1 - y) * width + x) * 3;
            img.setPixel(sf::Vector2u(x, y), sf::Color(image[idx], image[idx+1], image[idx+2]));
        }
    bool _result = img.saveToFile(std::filesystem::path(filename));

    #endif
}

Color RayTracer::getRayColor(Vec3 S, Vec3 D, int recursionLevel)
{
    // Determine color using sphere intersection
    // Takes color from closest sphere

    Shape3D* closest = nullptr;
    Vec3 closestNormal = Vec3();
    float smallestDist = 9999.0;
    for (Shape3D* shape : shapes)
    {
        // From ray tracing Wikipedia (and class lecture):
        // t^2 + (2V . D)t + (V^2 - r^2) = 0, where
        // Sphere: |X - C|^2 = r^2
        // Ray: R(t) = S + Dt
        // As a shorthand, V = S - C

        IntersectData intersection = shape->checkIntersection(S, D);
        if (intersection.t >= 0 && intersection.t < smallestDist)
        {
            smallestDist = intersection.t;
            closest = shape;
            closestNormal = intersection.normal;
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
        Vec3 normal = closestNormal;
        for (Light* light : lights)
        {
            // Shadow
            // Find distance from light to current object
            // Any object with smaller distance will block it (0.01 used for bias)
            // Ignored if the object is already occluding itself

            bool isShadow = false;
            Vec3 vL = -light->direction.normalized();
            if (closest->checkIntersection(intersectPos + normal * 0.01, vL).t <= 0)
            {
                for (Shape3D* shadowShape : shapes)
                {
                    if (shadowShape->checkIntersection(intersectPos + normal * 0.01, vL).t > 0)
                    {
                        isShadow = true;
                        break;
                    }
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

        // Translucent surface: mix color with second ray

        if (material.translucent && recursionLevel < 3)
        {
            float alpha = 0.2;
            alpha += intensity * 0.6;
            Color behindCol = getRayColor(intersectPos + lookAt * 0.6, D, recursionLevel + 1);
            col = (colVec * alpha + Vec3::fromColor(behindCol) * (1 - alpha)).toColor();
        }
    }
    return col;
}

// Helper functions for rendering

float ease(float x)
{
    return (x < 0.5) ? (4 * std::pow(x, 3.0)) : (1 - std::pow(-2 * x + 2, 3) * 0.5);
}

float lerp(float a, float b, float x)
{
    return a + x * (b - a);
}

float inverse_lerp(float a, float b, float x)
{
    return std::min(std::max((x - a) / (b - a), 0.f), 1.f);
}

void RayTracer::renderFrame(int frame, unsigned char* image, int width, int height)
{
    float T = frame / 60.0;

    #define NORMAL_DEMO
    #ifdef NORMAL_DEMO
    
    // [0, 4]: Rotate around the spheres

    if (T < 4.0)
    {
        float t = ease(inverse_lerp(0, 4, T));
        float angle = t * 2 * pi + pi;
        viewpoint = Vec3(std::sin(angle) * 9, 0, std::cos(angle) * 9 - 3);
        lookAt = (Vec3(0, -3, -3) - viewpoint).normalized();
        upVec = (lookAt.rotateXZ(pi / 2) * Vec3(1, 0, 1)).cross(lookAt).normalized();
    }

    // [4, 6]: Move closer
    // [6, 8]: Move back

    else if (T < 6.0)
    {
        float t = ease(inverse_lerp(4, 6, T));
        viewpoint = Vec3(0, 0, -12 + 6 * t);
    }
    else if (T < 8.0)
    {
        float t = ease(inverse_lerp(6, 8, T));
        viewpoint = Vec3(0, 0, -6 - 6 * t);
    }

    // [8, 11]: Angle camera above spheres

    else if (T < 11.0)
    {
        float t = ease(inverse_lerp(8, 11, T));
        float angle = t * pi / 2;
        viewpoint = Vec3(0, 9 * std::sin(angle), -3 - 9 * std::cos(angle));
        lookAt = (Vec3(0, -3, -3) - viewpoint).normalized();
        upVec = (lookAt.rotateXZ(pi / 2) * Vec3(1, 0, 1)).cross(lookAt).normalized();
    }

    #else

    // [0, 5]: Pan around spheres

    if (T < 5.0)
    {
        float t = ease(inverse_lerp(0, 5, T));
        float angle = t * 2.2 + 0.5;
        viewpoint = Vec3(std::sin(angle) * 9 + 4, 0, std::cos(angle) * 9 - 3);
        lookAt = (Vec3(0, 0, -3) - viewpoint).normalized();
    }

    #endif

    // Finally, render it

    createImage(image, width, height);
    std::string paddedFrame = std::to_string(frame);
    while (paddedFrame.length() < 4)
        paddedFrame = '0' + paddedFrame;
    saveImage(".output/img" + paddedFrame + ".png", image, width, height);
}