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
  int keyMode;
  int imgWidth;

  void onCreate() {
    keyMode = 1;
    // Load a .jpg file
    const char *filename = "./data/colors.jpg";

    auto imageData = Image(filename);

    if (imageData.array().size() == 0) {
      cout << "failed to load image" << endl;
    }
    cout << "loaded image size: " << imageData.width() << ", "
         << imageData.height() << endl;

    imgWidth = imageData.width();

    for (int j = 0; j < imageData.height(); ++j) {
      for (int i = 0; i < imageData.width(); ++i) {
        auto pixel = imageData.at(i, j);
        // this does not seem to show anything
        // float rNorm = ((int)pixel.r/255 * 2) -1;
        // float gNorm = ((int)pixel.r/255 * 2) -1;
        // float bNorm = ((int)pixel.r/255 * 2) -1;
        // float rNorm = ((int)pixel.r)/10 - 25/2;
        // float gNorm = ((int)pixel.g)/10 - 25/2;
        // float bNorm = ((int)pixel.b)/10 - 25/2;
        mesh.vertex(i/10,j/10,0);
        mesh.color((int)pixel.r,(int)pixel.g,(int)pixel.b);
      }
    }
    // Generate the geometry onto which to display the texture
    mesh.primitive(Mesh::POINTS);
    nav().pullBack(100);

  }

  void onDraw(Graphics &g) {
    //color of bakcgrund
    g.clear(0.2f);
    // size of point primitive
    g.pointSize(8);
    // need to call this to show colors
    g.meshColor();
     // draw the mesh
    g.draw(mesh);
    // g.draw(wire);
  }

  bool onKeyDown(const Keyboard &k) {
    if (k.key() == '1') {
      // Press space bar to change mode
      keyMode = 1;
    } else if (k.key() == '2') {
      // Press s to change mode
      keyMode = 2;
    }
    return true;
  }

  void onAnimate(double dt_ms) {
    if (keyMode == 1) {
      auto& vertex = mesh.vertices(); // 'vertex' becomes an alias for 'mesh.vertices()'
      for (int i = 1; i < vertex.size(); i++) {
        int x = i % imgWidth;
        int y = i / imgWidth;
        Vec3f newPos = Vec3f(x/10,y/10,0);
        vertex[i].lerp(newPos, 0.01);
      }
      
    } else if (keyMode == 2) {
      auto& vertex = mesh.vertices(); // 'vertex' becomes an alias for 'mesh.vertices()'
      auto& colors = mesh.colors();
      for (int i = 1; i < vertex.size(); i++) {
        float rNorm = ((int)colors[i].r)/10 - 25/2;
        float gNorm = ((int)colors[i].g)/10 - 25/2;
        float bNorm = ((int)colors[i].b)/10 - 25/2;
        Vec3f newPos = Vec3f(rNorm,gNorm,bNorm);
        vertex[i].lerp(newPos, 0.01);
      }
    }
  }
};


int main() {
  MyApp app;
  app.dimensions(600, 400);
  app.title("imageTexture");
  app.start();
}
