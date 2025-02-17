#include <SFML/Graphics.hpp>
#include <Eigen/Dense>
#include <vector>
#include <iostream>
#include <imgui.h>
#include <imgui-SFML.h>

// Définition de la structure Planète
struct Planet {
    Eigen::Vector2d position;  // Position de la planète
    Eigen::Vector2d velocity;  // Vitesse de la planète
    float mass;                // Masse de la planète (modifiée en float)
    sf::CircleShape shape;     // Forme graphique de la planète

    // Constructeur de la planète
    Planet(double x, double y, double vx, double vy, float m, sf::Color color) 
        : position(x, y), velocity(vx, vy), mass(m) {
        shape.setRadius(5);          // Définir le rayon de la planète
        shape.setFillColor(color);   // Définir la couleur de la planète
        shape.setOrigin(5, 5);       // Définir l'origine de la forme au centre
    }

    // Mettre à jour la position de la planète en fonction de sa vitesse
    void updatePosition(double dt) {
        position += velocity * dt;
        shape.setPosition(position.x(), position.y());
    }
};

// Fonction pour calculer les forces gravitationnelles entre deux planètes
Eigen::Vector2d computeGravity(const Planet& p1, const Planet& p2) {
    const double G = 1;  // Constante gravitationnelle (échelle arbitraire)
    Eigen::Vector2d diff = p2.position - p1.position;  // Différence de position entre les deux planètes
    double dist = diff.norm() + 1e-6;  // Distance entre les deux planètes (ajout d'une petite valeur pour éviter la division par zéro)
    return G * (p1.mass * p2.mass) / (dist * dist) * diff.normalized();  // Force gravitationnelle
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Three-Body Problem");  // Créer une fenêtre SFML
    if (!ImGui::SFML::Init(window)) {  // Initialiser ImGui-SFML et vérifier le retour
        std::cerr << "Failed to initialize ImGui-SFML" << std::endl;
        return -1;
    }

    // Initialiser les planètes avec des valeurs prédéfinies
    std::vector<Planet> planets = {
        Planet(200, 300, 0, 1, 10.0f, sf::Color::Red),
        Planet(400, 300, 0, -1, 10.0f, sf::Color::Green),
        Planet(600, 300, 0, 0, 10.0f, sf::Color::Blue)
    };

    double dt = 0.1;  // Intervalle de temps pour la simulation
    bool isRunning = false;  // Indicateur pour savoir si la simulation est en cours

    // Correction : sf::VertexArray au lieu de std::vector<sf::VertexArray>
    std::vector<sf::VertexArray> trajectories(planets.size());

    // Initialiser chaque VertexArray avec le type de primitive
    for (auto& va : trajectories) {
        va.setPrimitiveType(sf::LineStrip);
    }

    sf::Clock deltaClock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);  // Traiter les événements ImGui
            if (event.type == sf::Event::Closed) {
                window.close();  // Fermer la fenêtre si l'événement de fermeture est détecté
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space) {
                    isRunning = !isRunning;  // Basculer entre pause et lecture
                } else if (event.key.code == sf::Keyboard::R) {
                    // Réinitialiser les planètes avec des valeurs prédéfinies
                    planets = {
                        Planet(200, 300, 0, 1, 10.0f, sf::Color::Red),
                        Planet(400, 300, 0, -1, 10.0f, sf::Color::Green),
                        Planet(600, 300, 0, 0, 10.0f, sf::Color::Blue)
                    };
                    trajectories.clear();  // Effacer les trajectoires
                    trajectories.resize(planets.size());  // Redimensionner les trajectoires
                }
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());  // Mettre à jour ImGui-SFML

        ImGui::Begin("Planet Parameters");  // Créer une fenêtre ImGui
        for (size_t i = 0; i < planets.size(); ++i) {
            ImGui::Text("Planet %zu", i + 1);
            float position[2] = { static_cast<float>(planets[i].position.x()), static_cast<float>(planets[i].position.y()) };
            float velocity[2] = { static_cast<float>(planets[i].velocity.x()), static_cast<float>(planets[i].velocity.y()) };
            ImGui::SliderFloat2("Position", position, 0.0f, 800.0f);
            ImGui::SliderFloat2("Velocity", velocity, -100.0f, 100.0f);
            ImGui::SliderFloat("Mass", &planets[i].mass, 1.0f, 100.0f);
            planets[i].position = Eigen::Vector2d(position[0], position[1]);
            planets[i].velocity = Eigen::Vector2d(velocity[0], velocity[1]);
        }
        ImGui::End();

        if (isRunning) {
            std::vector<Eigen::Vector2d> forces(planets.size(), Eigen::Vector2d(0, 0));  // Initialiser les forces

            for (size_t i = 0; i < planets.size(); ++i) {
                for (size_t j = 0; j < planets.size(); ++j) {
                    if (i != j) {
                        forces[i] += computeGravity(planets[i], planets[j]);  // Calculer les forces gravitationnelles
                    }
                }
            }

            for (size_t i = 0; i < planets.size(); ++i) {
                planets[i].velocity += forces[i] / planets[i].mass * dt;  // Mettre à jour la vitesse des planètes
                planets[i].updatePosition(dt);  // Mettre à jour la position des planètes

                // Ajouter un point à la VertexArray
                sf::Vertex point;
                point.position = sf::Vector2f(planets[i].position.x(), planets[i].position.y());
                point.color = planets[i].shape.getFillColor();
                trajectories[i].append(point);  // Ajouter le point à la trajectoire
            }
        }

        window.clear();  // Effacer la fenêtre

        // Dessiner chaque VertexArray
        for (const auto& va : trajectories) {
            window.draw(va);
        }

        for (const auto& planet : planets) {
            window.draw(planet.shape);  // Dessiner chaque planète
        }

        ImGui::SFML::Render(window);  // Rendre ImGui-SFML
        window.display();  // Afficher le contenu de la fenêtre
    }

    ImGui::SFML::Shutdown();  // Arrêter ImGui-SFML
    return 0;
}