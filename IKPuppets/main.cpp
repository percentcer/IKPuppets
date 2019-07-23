#include <iostream>
#include <SFML/Graphics.hpp>

template<int N>
struct Bones {
    int parent[N];
    
    float length[N];
    float rotation[N];
    float initialRotation[N];

    size_t size = N;
};

template <int N>
sf::Transform localTransform(const Bones<N>& bones, int i) {
    return sf::Transform().rotate(bones.rotation[i]);
}

template <int N>
sf::Transform ancestryTransform(const Bones<N>& bones, int i) {
    sf::Transform accumulation;
    
    int current = i;
    while ((current = bones.parent[current]) != -1) {
        sf::Transform parentTransform;
        parentTransform.rotate(bones.rotation[current]);
        parentTransform.translate(bones.length[current], 0);
        accumulation = parentTransform * accumulation;
    }

    return accumulation;
}

template <int N>
sf::Transform worldTransform(const Bones<N>& bones, int i) {
    return ancestryTransform(bones, i) * localTransform(bones, i);
}

template<int N>
void drawBones(const Bones<N>& bones, sf::RenderTarget& target) {
    for (int i = 0; i < bones.size; i++) {
        sf::ConvexShape shape;
        shape.setFillColor(sf::Color::Cyan);
        shape.setPointCount(4);
        shape.setPoint(0, sf::Vector2f(0, 0));
        shape.setPoint(1, sf::Vector2f(10, 10));
        shape.setPoint(2, sf::Vector2f(bones.length[i], 0));
        shape.setPoint(3, sf::Vector2f(10, -10));

        // --- hacky way to center the joint chain, remove ----------------------
        static const sf::Transform offset = sf::Transform(1, 0, 640, 0, 1, 360, 0, 0, 1);
        target.draw(shape, offset * worldTransform(bones, i));
    }
}

int main() {
    // Init
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    sf::RenderWindow window(sf::VideoMode(1280, 720), "IK Puppets", sf::Style::Default, settings);

    sf::Clock clock;
    sf::Vector2f target;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::KeyPressed:
                switch (event.key.code) {}
                break;
            case sf::Event::MouseMoved:
                target = sf::Vector2f{ (float)event.mouseMove.x, (float)event.mouseMove.y };
                break;
            default:
                break;
            }
        }

        // render stuff
        {
            window.clear(sf::Color(32, 32, 32, 255));

            const float dt = clock.getElapsedTime().asSeconds();
            Bones<8> bones;
            for (int i = 0; i < bones.size; i++) {
                bones.parent[i] = i - 1;
                bones.length[i] = 100.f - 10 * i;
                bones.initialRotation[i] = bones.rotation[i] = 10.f * i * cos(dt * (1.0f + i));
            }
            bones.rotation[0] = dt * 4.f;

            drawBones(bones, window);

            window.display();
        }
    }

    return 0;
}
