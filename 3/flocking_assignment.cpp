// minimal app, ready for adapting..
//

#include "al/app/al_App.hpp"
#include "al/math/al_Random.hpp"

using namespace al;


// A "boid" (play on bird) is one member of a flock.
class Boid {
 public:
  // Each boid has a position and velocity.
  Vec3f location;
  Vec3d velocity;
  Vec3f acceleration;

  float maxspeed;
  float maxforce;

  void applyForce(Vec3f force) {
    // We could add mass here if we want A = F / M
    acceleration += force;
  }

  void update(float dt) {
    velocity += acceleration * dt;
    velocity = velocity.mag() > maxforce ? velocity.norm(maxforce):(velocity);
    location += velocity * dt;
    acceleration.zero();
  }

};

struct MyApp : App {
  static const int Nb = 10;  // Number of boids
  Mesh heads, tails;
  Boid boids[Nb];
  void onCreate() override {
    resetBoids();
    nav().pullBack(10);
  }

  // Randomize boid positions/velocities uniformly inside unit disc
  void resetBoids() {
    for (auto& b : boids) {
      b.velocity = rnd::ball<Vec3f>();
      b.location = Vec3f(0,0,0);
      b.acceleration.zero();
      b.maxspeed =  rnd::uniform(2.f,5.f);
      b.maxforce =  rnd::uniform(1.f,5.f);
    }
  }

  void onAnimate(double dt) override {
  
    // Generate meshes
    heads.reset();
    heads.primitive(Mesh::POINTS);

    tails.reset();
    tails.primitive(Mesh::LINES);

    for (int i = 0; i < Nb; ++i) {
      boids[i].update(dt);

      heads.vertex(boids[i].location);
      heads.color(HSV(float(i) / Nb * 0.3 + 0.3, 0.7));

      tails.vertex(boids[i].location);
      tails.vertex(boids[i].location - boids[i].velocity.normalized(0.07));

      tails.color(heads.colors()[i]);
      tails.color(RGB(0.5));
    }
  }

  bool onKeyDown(const Keyboard &k) override {
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

    if (k.key() == '3') {
      // go to the center of mass
      resetBoids();
    }

    return true;
  }

  Vec3f calculatePosAverage() {
    Vec3f posAverage = 0;
    for (int i = 0; i < Nb; i++) {
      posAverage += boids[i].location;
    }
    posAverage /= Nb;
    return posAverage;
  }

  void onDraw(Graphics& g) override {
    g.clear(0, 0, 0);
    g.pointSize(8);
    g.meshColor();
    g.draw(heads);
    g.draw(tails);
  }
};

int main() {
  MyApp app;
  app.start();
}
