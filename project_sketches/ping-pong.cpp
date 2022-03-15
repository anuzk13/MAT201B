/*
Allocore Example: Platonic Solids

Description:
This shows how to draw the five Platonic solids. A Platonic solid has
congruent faces of regular polygons with the same number of faces meeting at
each vertex. The Platonic solids are

        Tetrahedron (4 triangles)
        Cube or Hexahedron (6 squares)
        Octahedron (8 triangles)
        Dodecahedron (12 pentagons)
        Icosahedron (20 triangles)

Author:
Lance Putnam, 12/8/2010 (putnam.lance at gmail dot com)
*/

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/math/al_Random.hpp"

using namespace al;

class MyApp : public App {
 public:
  Mesh solids[5];
  // Light light;
  double angle1,angle2;

  void onCreate() {
    angle1 = 0;
    angle2 = 100;
    int Nv;

    Nv = addIcosahedron(solids[4]);
    for (int i = 0; i < Nv; ++i) {
      float f = float(i) / Nv;
      solids[4].color(HSV(f * 0.1 + 0.7, 1, 1));
    }

    nav().pullBack(16);
  }

  void onAnimate(double dt) {
    angle1 += M_PI* 10/angle2;
    std::cout << "looking at vertex" << angle1 << std::endl;
    if (abs(angle1) > M_PI * 10) {
        angle2 = -angle2;
    }
  }

  void onDraw(Graphics& g) {
    g.clear(0, 0, 0);

    g.depthTesting(true);
    g.lighting(true);

    float angPos = 2 * M_PI / 5;
    float R = 3;

    for (int i = 0; i < 5; ++i) {
      g.pushMatrix();
    //   g.translate(R * cos(i * angPos), R * sin(i * angPos), 0);
      g.rotate(angle1 , 0, 1, 0);
      g.color(0.5, 0.5, 0.5);
      g.polygonFill();
      g.draw(solids[i]);
      g.scale(1.01);
      g.color(0);
      g.polygonLine();
      g.draw(solids[i]);
      g.popMatrix();
    }
  }
};

int main() { MyApp().start(); }
