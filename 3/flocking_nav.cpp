// Karl Yerkes
// MAT201B
// 2022-01-04
// minimal app, ready for adapting..
//

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

#include "al/app/al_App.hpp"
#include "al/app/al_GUIDomain.hpp"
#include "al/math/al_Random.hpp"
#include "al/math/al_Functions.hpp"

using namespace al;
using namespace std;

double r() { return rnd::uniformS(); }

Vec3f wrapVec3f(Vec3f val, float hi, float lo) {
  return Vec3f(wrap(val.x, hi, lo), wrap(val.y, hi, lo), wrap(val.z, hi, lo));
}

static const int Nb = 1000;  // Number of boids
static const int cubeSize = 3;  // Number of boids

// A "boid" (play on bird) is one member of a flock.
class Boid {
 public:
  // Each boid has a position and velocity.
  Nav pose;
  float maxspeed;

  void init() {
    maxspeed = rnd::uniform(0.01,0.2);
    pose.pos(r(), r(), r());
    pose.quat().set(r(), r(), r(), r()).normalize();
  }


  void update(float dt) {
    pose.moveF(maxspeed);
    pose.step(dt);
  }
};

struct MyApp : App {
  Parameter timeScale{"Time Sacale", 1, 0.01, 10};
  Parameter cohesionStrenght{"Cohesion Strenght", 0.5, 0.0, 1};
  Parameter separationRadius{"Separation Radius", 1, 0.5, 2};
  Parameter separationStrenght{"Separation Strenght", 0.3, 0.0, 1};
  Parameter alignmentStrenght{"Alignment Strenght", 0.5, 0.0, 1};

  Boid boids[Nb];
  Mesh mesh;

  void onInit() override {
    // set up GUI
    auto GUIdomain = GUIDomain::enableGUI(defaultWindowDomain());
    auto& gui = GUIdomain->newGUI();
    gui.add(timeScale);  // add parameter to GUI
    gui.add(cohesionStrenght);  // add parameter to GUI
    gui.add(separationRadius);  // add parameter to GUI
    gui.add(separationStrenght);  // add parameter to GUI
  }

  void resetBoids() {
    for (auto& b : boids) {
      b.init();
    }
  }

  void onCreate() override {
    // place nav
    nav().pullBack(5);
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
    for (auto& b : boids) {
        b.update(dt * timeScale);
    }
    //
    for (int i = 0; i < Nb; i++)
    {
        Vec3f cohesionCenter(0, 0, 0);
        Vec3f alignmentCenter(0, 0, 0);
        int cohesionCount = 0;
        Vec3f separationCenter(0, 0, 0);
        int separationCount = 0;
        Boid main = boids[i];
        for (int j = 0; j < Nb; j++)
        {
            float distance = (main.pose.pos() - boids[j].pose.pos()).mag();
            if ( i!= j && distance > separationRadius + 0.2 && distance < separationRadius + 1.2) {
                cohesionCenter += boids[j].pose.pos();
                cohesionCount++;
                // but ! this is in the agent's frame of reference
                alignmentCenter += boids[j].pose.pos() + boids[j].pose.quat().rotate(Vec3d(0, 0, -1));

            }
            if (i!= j && distance < separationRadius) {
                separationCenter += boids[j].pose.pos();
                separationCount ++;
            }
        }
        if (cohesionCount > 0) {
            Vec3f centeringPos = cohesionCenter/cohesionCount;
            main.pose.nudgeToward(centeringPos, cohesionStrenght);
            Vec3f alignmentPos = alignmentCenter/cohesionCount;
            main.pose.faceToward(alignmentPos, Vec3d(0, 1, 0), alignmentStrenght);
        }
        if (separationCount > 0) {
            Vec3f centeringPos = -separationCenter/separationCount;
            main.pose.nudgeToward(centeringPos, separationStrenght);
        }
    }
  }

  void onDraw(Graphics& g) override {
    // graphics / drawing settings
    g.clear(1);
    g.meshColor();

    // draw a body for each agent
    for (auto& b : boids) {
      g.pushMatrix();  // push()
      g.translate(wrapVec3f(b.pose.pos(), cubeSize, -cubeSize));
      g.rotate(b.pose.quat());  // rotate using the quat
      g.scale(0.03);
      g.draw(mesh);
      g.popMatrix();  // pop()
    }
  }

};

int main() {
  MyApp app;
  app.configureAudio(48000, 512, 2, 0);
  app.start();
}
