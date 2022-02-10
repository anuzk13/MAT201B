// Karl Yerkes
// MAT201B
// 2022-01-04
// minimal app, ready for adapting..
//

#include "al/app/al_App.hpp"
#include "al/app/al_GUIDomain.hpp"
#include "al/math/al_Random.hpp"

using namespace al;

double r() { return rnd::uniformS(); }

struct MyApp : App {
  ParameterColor color{"Color"};
  ParameterInt mode{"Mode", "", 1, 1, 4};

  static const int Nb = 20;  // Number of boids
  Nav boids[Nb];
  Mesh mesh;


  void onInit() override {
    // set up GUI
    auto GUIdomain = GUIDomain::enableGUI(defaultWindowDomain());
    auto& gui = GUIdomain->newGUI();
    gui.add(mode);  // add parameter to GUI
    gui.add(color);  // add parameter to GUI
  }

  void resetBoids() {
    for (auto& b : boids) {
      b.pos(r(), r(), r());
      b.quat().set(r(), r(), r(), r()).normalize();
    }
  }

  void onCreate() override {
    // place nav
    nav().pos(0.5, 0.7, 5);
    nav().faceToward(Vec3d(0, 0, 0), Vec3d(0, 1, 0));

    // create a prototype agent body
    mesh.primitive(Mesh::TRIANGLE_FAN);
    mesh.vertex(0, 0, -2);
    mesh.color(0, 0, 0);
    mesh.vertex(0, 1, 0);
    mesh.color(1, 0, 0);
    mesh.vertex(-1, 0, 0);
    mesh.color(0, 1, 0);
    mesh.vertex(1, 0, 0);
    mesh.color(0, 0, 1);
    mesh.vertex(0, 1, 0);
    mesh.color(1, 0, 0);

    // create the boids in random positions
    resetBoids();
  }

  void onAnimate(double dt) override {
    //
  }

  void onDraw(Graphics& g) override {
    // graphics / drawing settings
    g.clear(1);
    g.meshColor();

    // draw a body for each agent
    for (auto& b : boids) {
      g.pushMatrix();  // push()
      g.translate(b.pos());
      g.rotate(b.quat());  // rotate using the quat
      g.scale(0.03);
      g.draw(mesh);
      g.popMatrix();  // pop()
    }
  }

  bool onKeyDown(const Keyboard& k) override {
    return false;
  }

  void onSound(AudioIOData& io) override {
    //
  }
};

int main() {
  MyApp app;
  app.configureAudio(48000, 512, 2, 0);
  app.start();
}
