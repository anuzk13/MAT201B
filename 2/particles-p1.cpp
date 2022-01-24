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

struct AlloApp : App {
  Parameter pointSize{"/pointSize", "", 0.025, "", 0.0, 0.05};
  Parameter timeStep{"/timeStep", "", 10 , "", 0.01, 10};
  Parameter maxForce{"/maxForce", "", 1.0 * 1e2, "", 0.01, 1.0 * 1e5};
  Parameter dragFactor{"/dragFactor", "", 0.0, "", 0.0, 1.0};
  // G = 6.67430 x 10-11 m3*kg-1*s-2
  // I have only been able to find simulation values that work for a higher value for the G constant
  float G = 6.67 * 1e3 / 1e11;
  float currMaxForce = 0;

  ShaderProgram pointShader;

  //  simulation state
  Mesh mesh;  // position *is inside the mesh* mesh.vertices() are the positions
  vector<Vec3f> velocity;
  vector<Vec3f> acceleration;
  vector<float> mass;

  void onInit() override {
    // set up GUI
    auto GUIdomain = GUIDomain::enableGUI(defaultWindowDomain());
    auto &gui = GUIdomain->newGUI();
    gui.add(pointSize);  // add parameter to GUI
    gui.add(timeStep);   // add parameter to GUI
    gui.add(maxForce);  // add parameter to GUI
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

    mesh.primitive(Mesh::POINTS);
    // does 1000 work on your system? how many can you make before you get a low
    // frame rate? do you need to use <1000?
    for (int _ = 0; _ < 100; _++) {
      mesh.vertex(randomVec3f(5));
      mesh.color(randomColor());

      int pown = rnd::uniform(2.0, 5.0);
      float m = rnd::uniform(1.0, 10.0) * pow(10, pown);
      // if (m < 0.5) m = 0.5;
      mass.push_back(m);

      // using a simplified volume/size relationship
      mesh.texCoord(pow(m, 1.0f / (pown+1)), 0);  // s, t
      // separate state arrays
      velocity.push_back(randomVec3f(50.0 * 1e-5));
      acceleration.push_back(randomVec3f(0));
    }

    // // cute double orbit
    // float initialSpeed = 15.0 * 1e-5;
    // mesh.vertex(Vec3f(0, 0, 0));
    // mesh.color(randomColor());
    // mass.push_back(1000);
    // mesh.texCoord(pow(0.5, 1.0f / 3), 0);  // s, t
    // velocity.push_back(Vec3f(-initialSpeed, 0, 0));
    // acceleration.push_back(Vec3f(0, 0, 0));

    // mesh.vertex(Vec3f(0, 1 , 0));
    // mesh.color(randomColor());
    // mass.push_back(1000);
    // mesh.texCoord(pow(0.5, 1.0f / 3), 0);  // s, t
    // velocity.push_back(Vec3f(initialSpeed, 0 , 0));
    // acceleration.push_back(Vec3f(0, 0, 0));

    // // single orbit
    // initialSpeed = 50.0 * 1e-5;
    // mesh.vertex(Vec3f(1, 0, 0));
    // mesh.color(randomColor());
    // mass.push_back(1000);
    // mesh.texCoord(pow(0.5, 1.0f / 3), 0);  // s, t
    // velocity.push_back(Vec3f(0, 0, 0));
    // acceleration.push_back(Vec3f(0, 0, 0));

    // mesh.vertex(Vec3f(1, 1 , 0));
    // mesh.color(randomColor());
    // mass.push_back(100);
    // mesh.texCoord(pow(0.5, 1.0f / 3), 0);  // s, t
    // velocity.push_back(Vec3f(initialSpeed, 0 , 0));
    // acceleration.push_back(Vec3f(0, 0, 0));


    nav().pos(0, 0, 10);
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
    auto& vertex = mesh.vertices();
    for (int i = 0; i < vertex.size(); i++) {
      for (int j = 0; j < vertex.size(); j++) {
        if (i != j) {
          // calculate force
          Vec3f forceionj = gravitationalForce(mass[i], mass[j], vertex[i], vertex[j]);
          Vec3f forcejoni = gravitationalForce(mass[j], mass[i], vertex[j], vertex[i]);
          // apply force
          acceleration[i] += forcejoni / mass[i];
          acceleration[j] += forceionj / mass[j];

          float tempForce = max(forceionj.mag(), forcejoni.mag());
          currMaxForce = max(currMaxForce, tempForce);

          if (i==0 && j==1) cout << currMaxForce << endl;
        }
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
    vector<Vec3f> &position(mesh.vertices());
    for (int i = 0; i < velocity.size(); i++) {
      // "semi-implicit" Euler integration
      velocity[i] += acceleration[i] / mass[i] * dt;
      position[i] += velocity[i] * dt;

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

    return true;
  }

  Vec3f gravitationalForce(float m1, float m2, Vec3f p1, Vec3f p2) {
    // force of p1 on p2
    Vec3f diff = p1 - p2;
    float r = diff.mag();
    Vec3f dir = diff.normalize();
    float forceMag = (G * m1 * m2) / (r * r);
    return min(forceMag, maxForce.get()) * dir;
  }

  void onDraw(Graphics &g) override {
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