#include <iostream>
#include <SFML/Graphics.hpp>
#include <Eigen/Dense>

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
    // --- hacky way to center the joint chain, remove ----------------------
    static const sf::Transform offset = sf::Transform(1, 0, 640, 0, 1, 640, 0, 0, 1);
    return offset * ancestryTransform(bones, i) * localTransform(bones, i);
}

template<int N>
void drawBones(const Bones<N>& bones, const std::vector<sf::ConvexShape>& boneShapes, sf::RenderTarget& target) {
    for (size_t i = 0; i < bones.size; i++) {        
        target.draw(boneShapes[i], worldTransform(bones, (int)i));
    }
}

template <int N>
sf::Vector2f jointPosition(Bones<N>& bones, int i) {
    sf::Transform world = worldTransform(bones, i);
    return world.transformPoint(bones.length[i], 0);
}

template <int N>
sf::Vector2f endPosition(Bones<N>& bones) {
    return jointPosition(bones, N - 1);
}

template <int N>
void jacobianIK(Bones<N>& bones, sf::Vector2f target) {
    sf::Vector2f sfEndPos = endPosition(bones);
    Eigen::Vector2f endPos(sfEndPos.x, sfEndPos.y);

    const Eigen::Vector2f mousePos(target.x, target.y);

    int giveUp = 500;
    while ((mousePos - endPos).squaredNorm() > 1.f) {
        Eigen::Matrix<float, 2, N> jacobian;
        for (int i = 0; i < N; i++)
        {
            const sf::Vector2f sfJointPos = jointPosition(bones, i);
            const Eigen::Vector2f jointPos(sfJointPos.x, sfJointPos.y);
            const Eigen::Vector2f jointToEnd = endPos - jointPos;
            const Eigen::Vector3f jointToEnd3f = Eigen::Vector3f(jointToEnd.x(), jointToEnd.y(), 0.0f);
            static const Eigen::Vector3f axisZ(0.0f, 0.0f, 1.0f);
            const Eigen::Vector3f dTheta = axisZ.cross(jointToEnd3f);
            const Eigen::Vector2f dTheta2f = Eigen::Vector2f(dTheta.x(), dTheta.y());
            jacobian.col(i) = dTheta2f;
        }
        
        const Eigen::Matrix<float, N, 1> d0 = jacobian.transpose() * (mousePos - endPos);
        for (int i = 0; i < N; i++) {
            bones.rotation[i] += d0[i] * .0001f;
        }

        sf::Vector2f newPosition = endPosition(bones);
        endPos = Eigen::Vector2f(newPosition.x, newPosition.y);

        if (giveUp-- == 0) break;
    }
}

int main() {
    // Init
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    sf::RenderWindow window(sf::VideoMode(1280, 1280), "IK Puppets", sf::Style::Default, settings);

    sf::Clock clock;
    sf::Vector2f target;

    bool wiggleMode = true;

    // init bones
    Bones<4> bones;
    for (size_t i = 0; i < bones.size; i++) {
        bones.parent[i] = (int)i - 1;
        bones.length[i] = 100.f - 10 * i;
        bones.initialRotation[i] = 0.0f;
    }

    // init boneshapes
    std::vector<sf::ConvexShape> boneShapes;
    for (size_t i = 0; i < bones.size; i++) {
        sf::ConvexShape shape;
        shape.setFillColor(sf::Color::Cyan);
        shape.setPointCount(4);
        shape.setPoint(0, sf::Vector2f(0, 0));
        shape.setPoint(1, sf::Vector2f(10, 10));
        shape.setPoint(2, sf::Vector2f(bones.length[i], 0));
        shape.setPoint(3, sf::Vector2f(10, -10));
        boneShapes.emplace_back(shape);
    }

    // mouse target dot
    const float rad = 10.f;
    sf::CircleShape dot(rad);
    dot.setFillColor(sf::Color::Yellow);

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
                switch (event.key.code) {
                case sf::Keyboard::Space:
                    wiggleMode = !wiggleMode;
                    break;
                }
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

            if (wiggleMode) {
                for (size_t i = 0; i < bones.size; i++) {
                    bones.rotation[i] = 10.f * i * cos(dt * (1.0f + i));
                }
                bones.rotation[0] = dt * 4.f;
            }
            else {
                dot.setPosition(target - sf::Vector2f{ rad, rad });
                window.draw(dot);
                jacobianIK(bones, target);
            }            

            drawBones(bones, boneShapes, window);

            window.display();
        }
    }

    return 0;
}
