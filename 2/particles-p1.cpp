// Karl Yerkes
// 2022-01-20

#include "al/app/al_App.hpp"
#include "al/app/al_GUIDomain.hpp"
#include "al/math/al_Random.hpp"
#include <cmath> // M_PI
#include "al/math/al_Functions.hpp"

using namespace al;

#include <fstream>
#include <vector>
using namespace std;

Vec3f randomVec3f(float scale) {
  return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()) * scale;
}
string slurp(string fileName);  // forward declaration

template <typename T> T sqrt(T &&that) {
  return T(that).normalize(sqrt(that.mag()));
}
// T is supposed to be a Vec type: Vec3f, Vec3d, Vec
// This function makes a copy T(that) that points in the same direction
// as the original, but has a length that is the sqrt of the original
// length.

const double G = 6.674e-11;    // the actual "big G"

struct Body {
  double mass;     // kilograms
  double radius;   // meters
  Vec3f position; // meters
  Vec3f velocity; // meters/second
  Vec3f acceleration; // meters/second * second
};


struct AlloApp : App {
  Parameter pointSize{"/pointSize", "", 0.01, "", 0.01, 0.05};
  Parameter timeStep{"/timeStep", "", 1e6 , "", 0.01, 1e7};

  ShaderProgram pointShader;

  //  simulation state
  vector<Body> bodies;

  bool warp_size = true;
  bool warp_distance = true;

  void onInit() override {
    // set up GUI
    auto GUIdomain = GUIDomain::enableGUI(defaultWindowDomain());
    auto &gui = GUIdomain->newGUI();
    gui.add(pointSize);  // add parameter to GUI
    gui.add(timeStep);   // add parameter to GUI
  }

  void onCreate() override {

    // compile shaders
    pointShader.compile(slurp("../point-vertex.glsl"),
                        slurp("../point-fragment.glsl"),
                        slurp("../point-geometry.glsl"));

    // set initial conditions of the simulation
    //

    // c++11 "lambda" function
    auto randomColor = []() { return HSV(rnd::uniform(), 1.0f, 1.0f); };

    orbitDemo();

    nav().pos(0, 0, 50);
  }

  bool freeze = false;
  void onAnimate(double dt) override {
    if (freeze) return;

    // ignore the real dt and set the time step;
    dt = timeStep;

    // Calculate forces

    // XXX you put code here that calculates gravitational forces
    // These are pair-wise. Each unique pairing of two particles
    // These are equal but opposite: A exerts a force on B while B exerts that
    // same amount of force on A (but in the opposite direction!) Use a nested
    // for loop to visit each pair once The time complexity is O(n*n)
    //
    for (int i = 0; i < bodies.size(); i++) {
      for (int j = i+1; j < bodies.size(); j++) {
          // calculate force
          Vec3f forceionj = gravitationalForce(bodies[i].mass, bodies[j].mass, bodies[i].position, bodies[j].position);
          Vec3f forcejoni = gravitationalForce(bodies[j].mass, bodies[i].mass, bodies[j].position, bodies[i].position);
          // apply force
          bodies[i].acceleration += forcejoni / bodies[i].mass;
          bodies[j].acceleration += forceionj / bodies[j].mass;
      }
    }

    // Vec3f has lots of operations you might use...
    // • +=
    // • -=
    // • .normalize()
    // • .normalize(float scale)
    // • .mag()
    // • .magSqr()
    // • .dot(Vec3f f)
    // • .cross(Vec3f f)

    // Integration
    //
    for (int i = 0; i < bodies.size(); i++) {
      // "semi-implicit" Euler integration
      bodies[i].velocity += bodies[i].acceleration * dt;
      bodies[i].position += bodies[i].velocity * dt;

      // Explicit (or "forward") Euler integration would look like this:
      // bodies[i].position += bodies[i].velocity * dt;
      // bodies[i].velocity += bodies[i].acceleration * dt;
    }

    // clear all accelerations (IMPORTANT!!)
    for (auto &b : bodies) b.acceleration.zero();
  }

  bool onKeyDown(const Keyboard &k) override {
    if (k.key() == ' ') {
      freeze = !freeze;
    }

    if (k.key() == '1') {
      // change warp_size
      warp_size = !warp_size;
    }

    if (k.key() == '2') {
      // change warp_size
      warp_distance = !warp_distance;
    }

    if (k.key() == '3') {
      // Orbit demo
      orbitDemo();
    }

    if (k.key() == '4') {
      // double orbit demo
      doubleOrbitDemo();
    }

    if (k.key() == '5') {
      // double orbit demo
      randomPlanerDemo();
    }

    return true;
  }

  void orbitDemo() {
    freeze = false;
    clearParticles();

    // Big central object
    Body b1 = {  1989100000e21, 695508e3, Vec3f(0, 0, 0), Vec3f(0,0,0), 0};
    bodies.push_back(b1);

    // // Orbiting object
    Body b2 = { 1898187e21, 69911e3, Vec3f(778.3e9, 0, 0), Vec3f(0,13.1e3,0), 0};
    bodies.push_back(b2);

  }

  
  void doubleOrbitDemo() {
    freeze = false;
    clearParticles();

    // Big central object
    Body b1 = { 1898187e21, 69911e3, Vec3f(-778.3e9/4, 0, 0), Vec3f(0,-50e1,0), 0};
    bodies.push_back(b1);

    // // Orbiting object
    Body b2 = { 1898187e21, 69911e3, Vec3f(778.3e9/4, 0, 0), Vec3f(0,50e1,0), 0};
    bodies.push_back(b2);

  }

  void randomPlanerDemo() {
    clearParticles();
    for (int i = 0; i < 50; i++) {
      Body b = randomPlanet();
      bodies.push_back(b);
    }
  }

  Body randomPlanet() {
    double mass = rnd::uniform(330e21, 1898187e21);
    // double radius = pow(mass / 5428, 1/3) * 3/4 *1/ M_PI;
    double radius = rnd::uniform(2439.4e3, 69911e3);
    Vec3f position = Vec3f(rnd::uniform(57.9e9, 778.3e9), rnd::uniform(57.9e9, 778.3e9), rnd::uniform(57.9e9, 778.3e9));
    Vec3f velocity = Vec3f(rnd::uniform(-1e2, 1e2), rnd::uniform(-1e2, 1e2), rnd::uniform(-1e2, 1e2));
    Vec3f acceleration = 0;
    Body r = { mass, radius, position, velocity, acceleration};
    return r;
  }

  void clearParticles() {
    bodies.clear();
  }

  Vec3f gravitationalForce(float m1, float m2, Vec3f p1, Vec3f p2) {
    // force of p1 on p2
    Vec3f diff = p1 - p2;
    float r = diff.mag();
    Vec3f dir = diff.normalize();
    float forceMag = (G * m1 * m2) / (r * r);
    return forceMag * dir;
  }

  void onDraw(Graphics &g) override {
    Mesh mesh;
    mesh.primitive(Mesh::POINTS);
    for (int i = 0; i < bodies.size(); i++) {
      mesh.color(HSV(wrap(0.1666666 + float(i) / bodies.size())));
      if (warp_distance)
        mesh.vertex(sqrt(bodies[i].position / 1e10));
      else
        mesh.vertex(bodies[i].position / 1e10);

      if (warp_size)
        mesh.texCoord(sqrt(bodies[i].radius / 5e4), 0); // s, t
      else
        mesh.texCoord((bodies[i].radius / 5e4), 0); // s, t
    }
    g.clear(0.3);
    g.shader(pointShader);
    g.shader().uniform("pointSize", pointSize);
    g.blending(true);
    g.blendTrans();
    g.depthTesting(true);
    g.draw(mesh);
  }
};

int main() {
  AlloApp app;
  app.configureAudio(48000, 512, 2, 0);
  app.start();
}

string slurp(string fileName) {
  fstream file(fileName);
  string returnValue = "";
  while (file.good()) {
    string line;
    getline(file, line);
    returnValue += line + "\n";
  }
  return returnValue;
}