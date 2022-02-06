// Karl Yerkes
// 2022-01-20

#include "al/app/al_App.hpp"
#include "al/app/al_GUIDomain.hpp"
#include "al/math/al_Random.hpp"
#include "al/graphics/al_DefaultShaderString.hpp"

using namespace al;

#include <fstream>
#include <vector>
using namespace std;

Vec3f randomVec3f(float scale) {
  return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()) * scale;
}
string slurp(string fileName);  // forward declaration

float map (float min_d, float max_d, float min_o, float max_o, float x) {
    return (max_d-min_d)*(x - min_o) / (max_o - min_o) + min_d ;
}

const double G = 6.674e-11;    // the actual "big G"

struct FlowField {
    vector<Vec2d> vectors; // meters/second
    int res;
};

struct AlloApp : App {
  FlowField field = {vector<Vec2d>(), 50};
  Parameter pointSize{"/pointSize", "", 1.0, 0.0, 2.0};
  Parameter timeStep{"/timeStep", "", 0.1, 0.01, 0.6};
  Parameter dragFactor{"/dragFactor", "", 0.0, 0.0, 0.6};
  // it is really hard to control this parameter so something interesting happens with the particles
  Parameter gravConstant{"/gravConstantExponent", "", 11, 1, 11};
  // 
  Parameter maxSpeedVF{"/maxSpeedVF", "", 5, 0, 10};

  ShaderProgram pointShader;
  ShaderProgram defaultShader;

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
    gui.add(maxSpeedVF);
    //
  }

  void onCreate() override {
    // compile shaders
    pointShader.compile(slurp("../point-vertex.glsl"),
                        slurp("../point-fragment.glsl"),
                        slurp("../point-geometry.glsl"));
    defaultShader.compile(defaultShaderVertexColor(false, false, false).vert,
                          defaultShaderVertexColor(false, false, false).frag);

    
    // set initial conditions of the simulation
    //
    for (int j = 0; j < field.res; ++j) {
      for (int i = 0; i < field.res; ++i) {
        // rotate an angle in a square starting on the top left corner
        Vec3f vect = Vec3f(1.f/field.res,0,0).lerp(Vec3f(0,1.f/field.res,0), rnd::uniform());
        field.vectors.push_back(vect);
      }
    }
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

  // particle pos is assumed to be given in the screen space coords
  Vec2f getFieldVector(Vec2f particlePos) {
    if (abs(particlePos.x) <= 0.5f && abs(particlePos.y)<= 0.5f ) {
      float x_index = map(0, field.res, -0.5f, 0.5f, particlePos.x);
      float y_index = map(0, field.res, -0.5f, 0.5f, particlePos.y);
      return field.vectors[floor(y_index) * field.res + floor(x_index)];
    }
  }

  float map (float min_d, float max_d, float min_o, float max_o, float x) {
    return (max_d-min_d)*(x - min_o) / (max_o - min_o) + min_d ;
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
        // calculate force
        Vec3f forceionj = gravitationalForce(mass[i], mass[j], vertex[i], vertex[j]);
        Vec3f forcejoni = gravitationalForce(mass[j], mass[i], vertex[j], vertex[i]);
        // apply force
        acceleration[i] += forcejoni / mass[i];
        acceleration[j] += forceionj / mass[j];
      }
    }

    // drag
    for (int i = 0; i < velocity.size(); i++) {
      acceleration[i] -= velocity[i] * dragFactor;
    }

    // vector field
    for (int i = 0; i < vertex.size(); i++) {
      if (abs(vertex[i].x) <= 1.5 &&  abs(vertex[i].y) <= 1.5) {
        int xpos = floor(map(0,field.res,-1.5f,1.5f,vertex[i].x));
        int ypos = floor(map(0,field.res,-1.5f,1.5f,vertex[i].y));
        Vec3f steer = field.vectors[ypos*field.res + xpos] * maxSpeedVF.get() - velocity[i];
        acceleration[i] += steer / mass[i];
      }
    }

    Vec3f posAvg = calculatePosAverage();

    // Integration
    //
    vector<Vec3f> &position(mesh.vertices());
    for (int i = 0; i < velocity.size(); i++) {
      // "semi-implicit" Euler integration
      velocity[i] += acceleration[i] * dt;
      position[i] += velocity[i] * dt;

      // prevent particles from escaping particles get pushed back
      if ((posAvg - position[i]).mag() > 15) {
        velocity[i] = velocity[i].mag() * (posAvg - position[i]).normalize();
        acceleration[i] = randomVec3f(1);
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
      // go to the center of mass
      nav().pos(Vec3f(0,0,0));
      nav().pullBack(10);
    }

    if (k.key() == '2') {
      // go to the center of mass
      nav().pos(calculatePosAverage());
      nav().pullBack(10);
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

    g.shader(defaultShader);
     // use the color stored in the mesh
    g.meshColor();
    Mesh fieldMesh(Mesh::LINES);
    for (int i = 0; i < field.res; ++i) {
      for (int j = 0; j < field.res; ++j) {
        Vec2f orientation = field.vectors[i*field.res + j];
        float originX = map(-1.5f,1.5f,0,field.res,i);
        float originY = map(-1.5f,1.5f,0,field.res,j);
        Vec3f originPoint = Vec3f(originX, originY, 0.f);
        Vec3f endPoint = Vec3f(originX + orientation.x,
                                originY + orientation.y,  0.f);

        Color color = Color(1,i/field.res,0);

        // here we're rendering a point based on the vector field
        fieldMesh.vertex(originPoint);
        fieldMesh.color(color);
        fieldMesh.vertex(endPoint);
        fieldMesh.color(color);
      }
    }
    g.draw(fieldMesh);
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