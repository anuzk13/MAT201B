// Karl Yerkes
// 2022-01-20

#include "al/app/al_App.hpp"
#include "al/app/al_GUIDomain.hpp"
#include "al/math/al_Random.hpp"

using namespace al;

#include <fstream>
#include <vector>
using namespace std;

Vec3f randomVec3f(float scale) {
  return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()) * scale;
}
string slurp(string fileName);  // forward declaration

const double G = 6.674e-11;    // the actual "big G"

struct AlloApp : App {
  Parameter pointSize{"/pointSize", "", 1.0, 0.0, 2.0};
  Parameter timeStep{"/timeStep", "", 0.1, 0.01, 0.6};
  Parameter dragFactor{"/dragFactor", "", 0.0, 0.0, 0.6};
  // it is really hard to control this parameter so something interesting happens with the particles
  Parameter gravConstant{"/gravConstantExponent", "", 11, 1, 11};
  //

  ShaderProgram pointShader;

  //  simulation state
  Mesh mesh;  // position *is inside the mesh* mesh.vertices() are the positions
  vector<Vec3f> velocity;
  vector<Vec3f> acceleration;
  vector<float> mass;
  float maxForce = 0;

  void onInit() override {
    // set up GUI
    auto GUIdomain = GUIDomain::enableGUI(defaultWindowDomain());
    auto &gui = GUIdomain->newGUI();
    gui.add(pointSize);  // add parameter to GUI
    gui.add(timeStep);   // add parameter to GUI
    gui.add(dragFactor);   // add parameter to GUI
    gui.add(gravConstant);   // add parameter to GUI
    //
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

    mesh.primitive(Mesh::POINTS);
    // does 1000 work on your system? how many can you make before you get a low
    // frame rate? do you need to use <1000?
    for (int _ = 0; _ < 1000; _++) {
      mesh.vertex(randomVec3f(5));
      mesh.color(randomColor());

      // float m = rnd::uniform(3.0, 0.5);
      float m = 3 + rnd::normal() / 2;
      if (m < 0.5) m = 0.5;
      mass.push_back(m);

      // using a simplified volume/size relationship
      mesh.texCoord(pow(m, 1.0f / 3), 0);  // s, t

      // separate state arrays
      velocity.push_back(randomVec3f(0.1));
      acceleration.push_back(randomVec3f(1));
    }

    nav().pos(0,0,10);
  }

  Vec3f gravitationalForce(float m1, float m2, Vec3f p1, Vec3f p2) {
    // force of p1 on p2
    Vec3f diff = p1 - p2;
    float r = diff.mag();
    Vec3f dir = diff.normalize();
    double gravConstParam =  6.674 / pow(10, gravConstant);
    float forceMag = (gravConstParam * m1 * m2) / (r * r);
    maxForce = max(maxForce, forceMag);
    return forceMag * dir;
  }

  Vec3f calculatePosAverage() {
    Vec3f posAverage = 0;
    for (int i = 0; i < mesh.vertices().size(); i++) {
      posAverage += mesh.vertices()[i];
    }
    posAverage /= mesh.vertices().size();
    return posAverage;
  }

  bool freeze = false;
  void onAnimate(double dt) override {
    if (freeze) return;

    // ignore the real dt and set the time step;
    dt = timeStep;

    // Calculate forces

    // XXX you put code here that calculates gravitational forces and sets
    // accelerations These are pair-wise. Each unique pairing of two particles
    // These are equal but opposite: A exerts a force on B while B exerts that
    // same amount of force on A (but in the opposite direction!) Use a nested
    // for loop to visit each pair once The time complexity is O(n*n)
    //
    // Vec3f has lots of operations you might use...
    // • +=
    // • -=
    // • +
    // • -
    // • .normalize() ~ Vec3f points in the direction as it did, but has length
    // 1 • .normalize(float scale) ~ same but length `scale` • .mag() ~ length
    // of the Vec3f • .magSqr() ~ squared length of the Vec3f • .dot(Vec3f f) •
    // .cross(Vec3f f)

    auto& vertex = mesh.vertices();
    for (int i = 0; i < vertex.size(); i++) {
      for (int j = i+1; j < vertex.size(); j++) {
        if (i != j) {
          // calculate force
          Vec3f forceionj = gravitationalForce(mass[i], mass[j], vertex[i], vertex[j]);
          Vec3f forcejoni = gravitationalForce(mass[j], mass[i], vertex[j], vertex[i]);
          // apply force
          acceleration[i] += forcejoni / mass[i];
          acceleration[j] += forceionj / mass[j];
        }
      }
    }

    // drag
    for (int i = 0; i < velocity.size(); i++) {
      acceleration[i] -= velocity[i] * dragFactor;
    }

    Vec3f posAvg = calculatePosAverage();

    // Integration
    //
    vector<Vec3f> &position(mesh.vertices());
    for (int i = 0; i < velocity.size(); i++) {
      // "semi-implicit" Euler integration
      velocity[i] += acceleration[i] * dt;
      position[i] += velocity[i] * dt;

      // prevent particles from escaping
      if ((posAvg - position[i]).mag() > 15) {
        velocity[i] = randomVec3f(0.1);
        acceleration[i] = randomVec3f(1);
        position[i] = posAvg + randomVec3f(1);
      }

      // Explicit (or "forward") Euler integration would look like this:
      // position[i] += velocity[i] * dt;
      // velocity[i] += acceleration[i] / mass[i] * dt;
    }

    // clear all accelerations (IMPORTANT!!)
    for (auto &a : acceleration) a.zero();
  }

  bool onKeyDown(const Keyboard &k) override {
    if (k.key() == ' ') {
      freeze = !freeze;
    }

    if (k.key() == '1') {
      // introduce some "random" forces
      for (int i = 0; i < velocity.size(); i++) {
        // F = ma
        acceleration[i] = randomVec3f(1) / mass[i];
      }
    }

    if (k.key() == '2') {
      // go to the center of mass
      nav().pos(calculatePosAverage());
      nav().pullBack(7);
    }



    return true;
  }

  void onDraw(Graphics &g) override {
    g.clear(0.3);
    g.shader(pointShader);
    g.shader().uniform("pointSize", pointSize / 100);
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