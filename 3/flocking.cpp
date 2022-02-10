/*
Based on Allolib example
[1] Reynolds, C. W. (1987). Flocks, herds, and schools: A distributed behavioral
    model. Computer Graphics, 21(4):25–34.

Author:
Lance Putnam, Oct. 2014
*/

#include <cmath>
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/math/al_Functions.hpp"
#include "al/math/al_Random.hpp"

using namespace al;

// A "boid" (play on bird) is one member of a flock.
class Boid {
 public:
  // Each boid has a position and velocity.
  Vec3f pos, vel;

  // Update position based on velocity and delta time
  void update(float dt) { pos += vel * dt; }
};

struct MyApp : public App {
  static const int Nb = 32;  // Number of boids
  Boid boids[Nb];
  Mesh heads, tails;

  void onCreate() {

    nav().pullBack(10);

    resetBoids();
  }

  // Randomize boid positions/velocities uniformly inside unit disc
  void resetBoids() {
    for (auto& b : boids) {
      b.pos = rnd::ball<Vec3f>();
      b.vel = rnd::ball<Vec3f>();
    }
  }

  void onAnimate(double dt_ms) {
    float dt = dt_ms;

    // Compute boid-boid interactions
    for (int i = 0; i < Nb - 1; ++i) {
      for (int j = i + 1; j < Nb; ++j) {
        // printf("checking boids %d and %d\n", i,j);

        auto ds = boids[i].pos - boids[j].pos;
        auto dist = ds.mag();

        // Collision avoidance
        float pushRadius = 0.01;
        float pushStrength = 2;
        float push = exp(-al::pow2(dist / pushRadius)) * pushStrength;

        auto pushVector = ds.normalized() * push;
        boids[i].pos += pushVector;
        boids[j].pos -= pushVector;

        // Velocity matching
        float matchRadius = 0.125;
        float nearness = exp(-al::pow2(dist / matchRadius));
        Vec2f veli = boids[i].vel;
        Vec2f velj = boids[j].vel;

        // Take a weighted average of velocities according to nearness
        boids[i].vel = veli * (1 - 0.5 * nearness) + velj * (0.5 * nearness);
        boids[j].vel = velj * (1 - 0.5 * nearness) + veli * (0.5 * nearness);

        // TODO: Flock centering
      }
    }

    // Update boid independent behaviors
    for (auto& b : boids) {
      // Random "hunting" motion
      float huntUrge = 0.2;
      auto hunt = rnd::ball<Vec3f>();
      // Use cubed distribution to make small jumps more frequent
      hunt *= hunt.magSqr();
      b.vel += hunt * huntUrge;

      // Bound boid into a box
      if (b.pos.x > 1 || b.pos.x < -1) {
        b.pos.x = b.pos.x > 0 ? 1 : -1;
        b.vel.x = -b.vel.x;
      }
      if (b.pos.y > 1 || b.pos.y < -1) {
        b.pos.y = b.pos.y > 0 ? 1 : -1;
        b.vel.y = -b.vel.y;
      }
      if (b.pos.z > 1 || b.pos.z < -1) {
        b.pos.z = b.pos.z > 0 ? 1 : -1;
        b.vel.z = -b.vel.z;
      }
    }

    // Generate meshes
    heads.reset();
    heads.primitive(Mesh::POINTS);

    tails.reset();
    tails.primitive(Mesh::LINES);

    for (int i = 0; i < Nb; ++i) {
      boids[i].update(dt);

      heads.vertex(boids[i].pos);
      heads.color(HSV(float(i) / Nb * 0.3 + 0.3, 0.7));

      tails.vertex(boids[i].pos);
      tails.vertex(boids[i].pos - boids[i].vel.normalized(0.07));

      tails.color(heads.colors()[i]);
      tails.color(RGB(0.5));
    }
  }

  void onDraw(Graphics& g) {
    g.clear(0);
    g.depthTesting(true);
    g.pointSize(8);
    // g.nicest();
    // g.stroke(8);
    g.meshColor();
    g.draw(heads);
    g.draw(tails);
  }

  bool onKeyDown(const Keyboard& k) {
    switch (k.key()) {
      case 'r':
        resetBoids();
        break;
    }
    return true;
  }
};

int main() { MyApp().start(); }
