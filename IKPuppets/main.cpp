#include <iostream>
#include <SFML/Graphics.hpp>

struct Joint {
    Joint* parent;
    float length;
    float rotation;
};

sf::Transform localTransform(const Joint& joint) {
    return sf::Transform().rotate(joint.rotation);
}

sf::Transform ancestryTransform(const Joint& joint) {
    sf::Transform accumulation;
    const Joint* current = &joint;
    while ((current = current->parent) != nullptr) {
        sf::Transform parentTransform;
        parentTransform.rotate(current->rotation);
        parentTransform.translate(current->length, 0);
        accumulation = parentTransform * accumulation;
    }
    return accumulation;
}

sf::Transform worldTransform(const Joint& joint) {
    return ancestryTransform(joint) * localTransform(joint);
}

void drawBone(const Joint& joint, sf::RenderTarget& target) {
    sf::ConvexShape shape;
    shape.setFillColor(sf::Color::Cyan);
    shape.setPointCount(4);
    shape.setPoint(0, sf::Vector2f(0, 0));
    shape.setPoint(1, sf::Vector2f(10, 10));
    shape.setPoint(2, sf::Vector2f(joint.length, 0));
    shape.setPoint(3, sf::Vector2f(10, -10));
    
    // --- hacky way to center the joint chain, remove ----------------------
    static const sf::Transform offset = sf::Transform(1,0,640,0,1,360,0,0,1);
    target.draw(shape, offset * worldTransform(joint));
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
            Joint root = { nullptr, 100.0f, dt };
            Joint upper = { &root, 80.0f, 20.f * cos(dt / 2.f)};
            Joint lower = { &upper, 60.0f, 40.f * cos(dt) };
            Joint end = { &lower, 50.0f, 50.0f * sin(dt) };
            
            drawBone(root, window);
            drawBone(upper, window);
            drawBone(lower, window);
            drawBone(end, window);

            window.display();
        }
    }

    return 0;
}
