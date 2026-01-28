#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <string>
#include <avz/gfx/Profiler.hpp>

// --- Shaders ---
std::string geometryShaderSource = R"glsl(
#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = MAX_VERTS) out;
uniform int u_segments;
uniform float u_radius;

void main() {
    float PI = 3.14159265;
    for (int i = 0; i <= u_segments; i++) {
        float ang = (float(i) / float(u_segments)) * 2.0 * PI;
        gl_Position = gl_in[0].gl_Position;
        EmitVertex();
        vec4 offset = vec4(cos(ang) * u_radius, sin(ang) * u_radius, 0.0, 0.0);
        gl_Position = gl_in[0].gl_Position + offset;
        EmitVertex();
    }
    EndPrimitive();
}
)glsl";

const std::string vertexShader = R"glsl(
#version 330 core
layout (location = 0) in vec2 position;
void main() { gl_Position = vec4(position, 0.0, 1.0); }
)glsl";

const std::string fragmentShader = R"glsl(
#version 330 core
out vec4 FragColor;
void main() { FragColor = vec4(0.0, 0.7, 1.0, 0.4); }
)glsl";

int main(int argc, char* argv[]) {
    int segments = 30;
    int numCircles = 500;

    if (argc >= 3) {
        segments = std::stoi(argv[1]);
        numCircles = std::stoi(argv[2]);
    }

    sf::RenderWindow window(sf::VideoMode({1000, 1000}), "SFML 3 GS Benchmark");
    window.setFramerateLimit(60);

    // Prepare Shader
    int maxVerts = (segments + 1) * 2;
    size_t pos = geometryShaderSource.find("MAX_VERTS");
    if (pos != std::string::npos) geometryShaderSource.replace(pos, 9, std::to_string(maxVerts));

    sf::Shader shader;
    shader.loadFromMemory(vertexShader, geometryShaderSource, fragmentShader);
    shader.setUniform("u_segments", segments);
    shader.setUniform("u_radius", 0.01f);

    // Prepare Data
    std::vector<sf::CircleShape> cpuCircles;
    sf::VertexArray gpuPoints(sf::PrimitiveType::Points, numCircles);
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-0.9f, 0.9f);

    for (int i = 0; i < numCircles; ++i) {
        float x = dist(rng);
        float y = dist(rng);
        gpuPoints[i].position = {x, y};
        sf::CircleShape c(15.0f, segments);
        c.setOrigin({15.0f, 15.0f});
        c.setPosition({(x + 1.0f) * 500.0f, (1.0f - y) * 500.0f});
        cpuCircles.push_back(c);
    }

    avz::Profiler profiler;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
        }

        window.clear(sf::Color(10, 10, 10));

        // 1. GPU Section
        profiler.startSection("GPU (GS Strip)");
        window.draw(gpuPoints, &shader);
        glFinish();
        profiler.endSection();

        // 2. CPU Section
        profiler.startSection("CPU (CircleShapes)");
        for (const auto& c : cpuCircles) {
            window.draw(c);
        }
        glFinish();
        profiler.endSection();

        // Clear terminal and print summary (ANSI escape code \033[H\033[2J clears screen)
        std::cout << "\ec" << "Benchmark: " << numCircles << " circles @ " << segments << " segments\n";
        std::cout << "------------------------------------------------------------\n";
        std::cout << profiler.getSummary();

        window.display();
    }
    return 0;
}