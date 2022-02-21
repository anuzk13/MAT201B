/*
  This example shows how to use Image, Array and Texture to read a .jpg file,
display it as an OpenGL texture and print the pixel values on the command line.
Notice that while the intput image has only 4 pixels, the rendered texture is
smooth.  This is because interpolation is done on the GPU.

  Karl Yerkes and Matt Wright (2011/10/10)
*/

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

#include "al/app/al_App.hpp"
#include "al/math/al_Functions.hpp"
#include "al/graphics/al_Image.hpp"

using namespace al;
using namespace std;


class MyApp : public App {
public:
  Mesh mesh;
  // Mesh wire;
  int imgWidth, imgHeight;

  void onCreate() {
    // Load a .jpg file
    const char *filename = "./data/displacement.png";

    auto imageData = Image(filename);

    if (imageData.array().size() == 0) {
      cout << "failed to load image" << endl;
      exit(1);
    }
    cout << "loaded image size: " << imageData.width() << ", "
         << imageData.height() << endl;

    imgWidth = imageData.width();
    imgHeight = imageData.height();

    for (int j = 0; j < imgHeight; ++j) {
      for (int i = 0; i < imgWidth; ++i) {
        auto pixel = imageData.at(i, j);
         // remap from 0, width to -1,1
        float x = map(-1,1,0,imgWidth,(float)i);
        float y = map(1,-1,0,imgHeight,(float)j);
         // remap from 0, 255 to 0,1
        float r = map(0,1,0,255,(float)pixel.r);
        float g = map(0,1,0,255,(float)pixel.g);
        float b = map(0,1,0,255,(float)pixel.b);
        Color color = Color(r,g,b);
        HSV hsvColor = HSV(color);
        float z = map(0,0.2,0,1,hsvColor.v);

        mesh.vertex(x,y,z);
        mesh.color(color);
      }
    }
    // Generate the geometry onto which to display the texture
    mesh.primitive(Mesh::POINTS);
    nav().pullBack(4);

  }

  void onDraw(Graphics &g) {
    //color of bakcgrund
    g.clear(0.2f);
    // size of point primitive
    g.pointSize(1);
    // show color of the vertex
    g.meshColor();
     // draw the mesh
    g.draw(mesh);
    // g.draw(wire);
  }

  bool onKeyDown(const Keyboard &k) {
    return true;
  }

  void onAnimate(double dt_ms) {
  }

  float map (float min_d, float max_d, float min_o, float max_o, float x) {
    return (max_d-min_d)*(x - min_o) / (max_o - min_o) + min_d ;
  }

};


int main() {
  MyApp app;
  app.dimensions(600, 400);
  app.title("imageTexture");
  app.start();
}
