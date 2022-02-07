// minimal app, ready for adapting..
//

#include "al/app/al_App.hpp"
#include "al/math/al_Random.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/math/al_Functions.hpp"
#include "al/app/al_GUIDomain.hpp"

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

  // Update position based on velocity and delta time
  Vec3f seek(Vec3f target) {
    Vec3f desired = location - target;
    desired.normalize();
    desired *= maxspeed;
    Vec3f steer = velocity - desired;
    return steer;
  }

  void update(float dt) {
    velocity += acceleration * dt;
    velocity = velocity.mag() > maxspeed ? velocity.norm(maxspeed):(velocity);
    location += velocity * dt;
    acceleration.zero();
  }

  void borders() {
    if (location.x > 1 || location.x < -1) {
      location.x = location.x > 0 ? 1 : -1;
      velocity.x = -velocity.x;
    }
    if (location.y > 1 || location.y < -1) {
      location.y = location.y > 0 ? 1 : -1;
      velocity.y = -velocity.y;
    }
    if (location.z > 1 || location.z < -1) {
      location.z = location.z > 0 ? 1 : -1;
      velocity.z = -velocity.z;
    }
  }

};

struct MyApp : App {
  Parameter separationRadius{"/separationRadius", "", 0.05, "", 0.01, 0.1};
  Parameter separationStrength{"/separationStrength", "", 1, "", 0.01, 2};
  Parameter alignmentRadius{"/alignmentRadius", "", 0.05, "", 0.01, 0.1};

  static const int Nb = 20;  // Number of boids
  Mesh heads, tails;
  Boid boids[Nb];
  VAOMesh mCube;

  void onInit() override {
    // set up GUI
    auto GUIdomain = GUIDomain::enableGUI(defaultWindowDomain());
    auto &gui = GUIdomain->newGUI();
    gui.add(separationRadius);  // add parameter to GUI
    gui.add(separationStrength);  // add parameter to GUI
    gui.add(alignmentRadius);  // add parameter to GUI
  }

  void onCreate() override {
    addCube(mCube, false, 1);
    mCube.primitive(Mesh::LINE_STRIP);
    mCube.update();
    nav().pullBack(10);
    resetBoids();
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

    // Flock behavior
    for (int i = 0; i < Nb - 1; ++i) {
      for (int j = i + 1; j < Nb; ++j) {
        Vec3f ds = boids[i].location - boids[j].location;
        float dist = ds.mag();

        // Collision avoidance
        // exp = e^val
        // the further it is the less strong is the push
        // different from daniel shiffman implementation in that he does an average
        float push = exp(-al::pow2(dist / separationRadius.get())) * separationStrength.get();

        auto pushVector = ds.normalized() * push;
        boids[i].location += pushVector;
        boids[j].location -= pushVector;

        // Velocity matching = alignment? = orientation mimicry?
        // this does not work, why?
        // float matchRadius = 0.125;
        // float nearness = exp(-al::pow2(dist / matchRadius));
        // Vec2f veli = boids[i].velocity;
        // Vec2f velj = boids[j].velocity;

        // Take a weighted average of velocities according to nearness
        // boids[i].velocity = veli * (1 - 0.5 * nearness) + velj * (0.5 * nearness);
        // boids[j].velocity = velj * (1 - 0.5 * nearness) + veli * (0.5 * nearness);

        // float nearness = exp(-al::pow2(dist / alignmentRadius.get()));
        // boids[i].velocity = Vec3f(boids[i].velocity).lerp(boids[j].velocity, 1 - 0.5 * nearness).normalize() * boids[i].velocity.mag();
        // boids[j].velocity = Vec3f(boids[j].velocity).lerp(boids[i].velocity, 0.5 * nearness).normalize() * boids[j].velocity.mag();
      }
    }

    for (int i = 0; i < Nb - 1; ++i) {
      Vec3f sum = Vec3f(0,0,0);
      int count = 0;
      for (int j = 0; j < Nb - 1; ++j) {
        if (i != j) {
          Vec3f ds = boids[i].location - boids[j].location;
          float dist = ds.mag();
          float cohesionRadius = 0.05;
          float nearness = exp(-al::pow2(dist / cohesionRadius));
          sum += boids[j].location * nearness;
          count +=1;
        }
      }
      if (count > 0) {
        sum /= count;
      }
      Vec3f cohesion = boids[i].seek(sum);
      boids[i].applyForce(cohesion);
    }

    // Individual behavior
    for (auto& b : boids) {
      b.update(dt);
      b.borders();
    }

    // Display
    heads.reset();
    heads.primitive(Mesh::POINTS);

    tails.reset();
    tails.primitive(Mesh::LINES);

    for (int i = 0; i < Nb; ++i) {
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
    g.clear(1, 1, 1);
    g.pointSize(8);
    g.meshColor();
    g.draw(heads);
    g.draw(tails);
    g.draw(mCube);
  }
};

int main() {
  MyApp app;
  app.start();
}
