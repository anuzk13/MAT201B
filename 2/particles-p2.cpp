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

struct Body {
  std::string name;
  double mass;     // kilograms
  double radius;   // meters
  double distance; // meters
  double velocity; // meters/second
};

std::vector<Body> body = {
    {    "Sun",1989100000e21, 695508e3,        0,      0},
    {"Jupiter",   1898187e21,  69911e3,  778.3e9, 13.1e3},
    { "Saturn",    568317e21,  58232e3,   1427e9,  9.7e3},
    { "Uranus",     86813e21,  25362e3,   2871e9,  6.8e3},
    {"Neptune",    102413e21,  24622e3, 4497.1e9,  5.4e3},
    {  "Earth",    5972.4e21,   6371e3,  149.6e9, 29.8e3},
    {  "Venus",    4867.5e21,   6052e3,  108.2e9,   35e3},
    {   "Mars",    641.71e21, 3389.5e3,  227.9e9, 24.1e3},
    {"Mercury",    330.11e21, 2439.4e3,   57.9e9, 47.9e3},
    // Pluto? Moons? 'Oumuamua?
};

const double G = 6.674e-11;    // the actual "big G"

struct AlloApp : App {
  Parameter pointSize{"/pointSize", "", 0.01, "", 0.01, 0.05};
  Parameter timeStep{"/timeStep", "", 1e3 , "", 0.01, 1e7};
  Parameter dragFactor{"/dragFactor", "", 0.0, "", 0.0, 1.0};
  // G = 6.67430 x 10-11 m3*kg-1*s-2
  // I have only been able to find simulation values that work for a higher value for the G constant

  ShaderProgram pointShader;

  //  simulation state
  vector<Vec3f> position;
  vector<Vec3f> velocity;
  vector<Vec3f> acceleration;
  vector<float> mass;

  bool warp_size = true;
  bool warp_distance = true;

  void onInit() override {
    // set up GUI
    auto GUIdomain = GUIDomain::enableGUI(defaultWindowDomain());
    auto &gui = GUIdomain->newGUI();
    gui.add(pointSize);  // add parameter to GUI
    gui.add(timeStep);   // add parameter to GUI
    gui.add(dragFactor);   // add parameter to GUI
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

    // Sun
    for (int i = 0; i< body.size(); i++) {
      position.push_back(Vec3f(body[i].distance, 0, 0));
      mass.push_back(body[i].mass);
      // separate state arrays
      velocity.push_back(Vec3f(0,body[i].velocity,0));
      acceleration.push_back(0);
    }
    

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
    for (int i = 0; i < position.size(); i++) {
      for (int j = i+1; j < position.size(); j++) {
          // calculate force
          Vec3f forceionj = gravitationalForce(mass[i], mass[j], position[i], position[j]);
          Vec3f forcejoni = gravitationalForce(mass[j], mass[i], position[j], position[i]);
          // apply force
          acceleration[i] += forcejoni / mass[i];
          acceleration[j] += forceionj / mass[j];
      }
    }
    // drag
    for (int i = 0; i < velocity.size(); i++) {
      acceleration[i] -= velocity[i] * dragFactor;
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
    for (int i = 0; i < velocity.size(); i++) {
      // "semi-implicit" Euler integration
      velocity[i] += acceleration[i] * dt;
      position[i] += velocity[i] * dt;

      // Explicit (or "forward") Euler integration would look like this:
      // position[i] += velocity[i] * dt;
      // velocity[i] += acceleration[i] * dt;
    }

    // clear all accelerations (IMPORTANT!!)
    for (auto &a : acceleration) a.zero();
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

    return true;
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
    for (int i = 0; i < position.size(); i++) {
      mesh.color(HSV(wrap(0.1666666 + float(i) / position.size())));
      if (warp_distance)
        mesh.vertex(sqrt(position[i] / 1e10));
      else
        mesh.vertex(position[i] / 1e10);

      if (warp_size)
        mesh.texCoord(sqrt(body[i].radius / 5e4), 0); // s, t
      else
        mesh.texCoord((body[i].radius / 5e4), 0); // s, t
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