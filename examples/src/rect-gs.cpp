#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <string>
#include <avz/gfx/Profiler.hpp>

// --- Shaders ---
const std::string vertexShader = R"glsl(
#version 330 core
layout (location = 0) in vec2 position;
void main() { gl_Position = vec4(position, 0.0, 1.0); }
)glsl";

// GS outputs exactly 4 vertices to make a square
const std::string geometryShader = R"glsl(
#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;
uniform float u_size; // Size in NDC

void main() {
    vec4 center = gl_in[0].gl_Position;
    
    // Triangle strip order: Top-Left, Bottom-Left, Top-Right, Bottom-Right
    gl_Position = center + vec4(-u_size,  u_size, 0.0, 0.0); EmitVertex();
    gl_Position = center + vec4(-u_size, -u_size, 0.0, 0.0); EmitVertex();
    gl_Position = center + vec4( u_size,  u_size, 0.0, 0.0); EmitVertex();
    gl_Position = center + vec4( u_size, -u_size, 0.0, 0.0); EmitVertex();
    
    EndPrimitive();
}
)glsl";

const std::string fragmentShader = R"glsl(
#version 330 core
out vec4 FragColor;
void main() { FragColor = vec4(1.0, 0.5, 0.0, 0.6); } // Orange
)glsl";

int main(int argc, char* argv[]) {
    int numRects = 1000;
    if (argc > 1) numRects = std::stoi(argv[1]);

    sf::RenderWindow window(sf::VideoMode({1000, 1000}), "SFML 3 Rectangle GS vs CPU");
    window.setFramerateLimit(60);

    sf::Shader shader;
    if (!shader.loadFromMemory(vertexShader, geometryShader, fragmentShader)) return -1;
    shader.setUniform("u_size", 0.02f);

    // --- Prepare Data ---
    std::vector<sf::RectangleShape> cpuRects;
    sf::VertexArray gpuPoints(sf::PrimitiveType::Points, numRects);
    
    std::mt19937 rng(123);
    std::uniform_real_distribution<float> dist(-0.9f, 0.9f);

    for (int i = 0; i < numRects; ++i) {
        float x = dist(rng);
        float y = dist(rng);
        gpuPoints[i].position = {x, y};

        sf::RectangleShape r({20.0f, 20.0f});
        r.setOrigin({10.0f, 10.0f});
        r.setPosition({(x + 1.0f) * 500.0f, (1.0f - y) * 500.0f});
        cpuRects.push_back(r);
    }

    avz::Profiler profiler;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
        }

        window.clear(sf::Color(20, 20, 20));

        // 1. GPU GS Benchmark
        profiler.startSection("GPU (GS Quad)");
        window.draw(gpuPoints, &shader);
        glFinish();
        profiler.endSection();

        // 2. CPU Rectangle Benchmark
        profiler.startSection("CPU (RectangleShape)");
        for (const auto& r : cpuRects) {
            window.draw(r);
        }
        glFinish();
        profiler.endSection();

        std::cout << "\ec" << "Benchmark: " << numRects << " Rectangles\n";
        std::cout << "------------------------------------------------------------\n";
        std::cout << profiler.getSummary();

        window.display();
    }
    return 0;
}