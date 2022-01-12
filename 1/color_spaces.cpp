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
  int imgWidth, imgHeight;

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
    imgHeight = imageData.height();

    for (int j = 0; j < imgHeight; ++j) {
      for (int i = 0; i < imgWidth; ++i) {
        auto pixel = imageData.at(i, j);
         // remap from 0, width to -1,1
        float mapX = map(-1,1,0,imgWidth,(float)i);
        float mapY = map(1,-1,0,imgHeight,(float)j);
         // remap from 0, 255 to 0,1
        float mapR = map(0,1,0,255,(float)pixel.r);
        float mapG = map(0,1,0,255,(float)pixel.g);
        float mapB = map(0,1,0,255,(float)pixel.b);
        mesh.vertex(mapX,mapY);
        mesh.color(mapR,mapG,mapB);
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
    // need to call this to show colors
    g.meshColor();
     // draw the mesh
    g.draw(mesh);
    // g.draw(wire);
  }

  bool onKeyDown(const Keyboard &k) {
    if (k.key() == '1') {
      // Image mode
      keyMode = 1;
    } else if (k.key() == '2') {
      // RGB Mode
      keyMode = 2;
    }  else if (k.key() == '3') {
      // HSV Mode
      keyMode = 3;
    }
    else if (k.key() == '4') {
      // CIE Mode
      keyMode = 4;
    }
    else if (k.key() == '5') {
      // Lab Mode
      keyMode = 5;
    }
    else if (k.key() == '6') {
      // HCLab Mode
      keyMode = 6;
    }
    else if (k.key() == '7') {
      // Luv Mode
      keyMode = 7;
    }
    else if (k.key() == '8') {
      // Loop Animation Mode
      keyMode = 8;
    }
    return true;
  }

  void onAnimate(double dt_ms) {
    if (keyMode == 1) {
      auto& vertex = mesh.vertices();
      for (int i = 1; i < vertex.size(); i++) {
        int x = i % imgWidth;
        int y = i / imgWidth;
        float mapX = map(-1,1,0,imgWidth,(float)x);
        float mapY = map(1,-1,0,imgHeight,(float)y);
        Vec3f newPos = Vec3f(mapX,mapY,0);
        vertex[i].lerp(newPos, 0.01);
      }
      
    } else if (keyMode == 2) {
      auto& vertex = mesh.vertices();
      auto colors = mesh.colors();
      for (int i = 1; i < vertex.size(); i++) {
        float mapR = map(-1,1,0,1,colors[i].r);
        float mapG = map(-1,1,0,1,colors[i].g);
        float mapB = map(-1,1,0,1,colors[i].b);
        Vec3f newPos = Vec3f(mapR,mapG,mapB);
        vertex[i].lerp(newPos, 0.01);
      }
    } else if (keyMode == 3) {
      auto& vertex = mesh.vertices();
      auto colors = mesh.colors();
      for (int i = 1; i < colors.size(); i++) {
        HSV hsvColor = HSV(colors[i]);
        float degrees = map(0,360,0,1,hsvColor.h);
        float radians = degrees * M_PI / 180.0;
        float x = hsvColor.s * cos(radians);
        float y = hsvColor.s * sin(radians);
        float z = map(-1,1,0,1,hsvColor.v);
        Vec3f newPos = Vec3f(x,y,z);
        vertex[i].lerp(newPos, 0.01);
      }
    } else if (keyMode == 4) {
      auto& vertex = mesh.vertices();
      auto colors = mesh.colors();
      for (int i = 1; i < colors.size(); i++) {
        //  r< red component in [0, 1]
        //  g< green component in [0, 1]
        //  b< blue component in [0, 1]
        CIE_XYZ cieColor = CIE_XYZ(colors[i]);
        float x = map(-1,1,0,1,cieColor.x);
        float y = map(-1,1,0,1,cieColor.y);
        float z = map(-1,1,0,1,cieColor.z);
        Vec3f newPos = Vec3f(x,y,z);
        vertex[i].lerp(newPos, 0.01);
      }
    } else if (keyMode == 5) {
      auto& vertex = mesh.vertices();
      auto colors = mesh.colors();
      for (int i = 1; i < colors.size(); i++) {
        
        //  l< Lightness component in [0, 100]
        //  a< red-green axis (red is positive, green is negative)
        //    range in [-85.9293, 97.9631] (8-bit rgb gamut)
        //  b< yellow-blue axis (yellow is positive, blue is
        //    negative) range in [-107.544, 94.2025]
        Lab labColor = Lab(colors[i]);
        float x = map(-1,1,0,100,labColor.l);
        float y = map(-1,1,-85.9293, 97.9631,labColor.a);
        float z = map(-1,1,-107.544, 94.2025,labColor.b);
        Vec3f newPos = Vec3f(x,y,z);
        vertex[i].lerp(newPos, 0.01);
      }
    } else if (keyMode == 6) {
      auto& vertex = mesh.vertices();
      auto colors = mesh.colors();
      for (int i = 1; i < colors.size(); i++) {
        // h< hue component in [0, 1]
        // c< chroma component in [0, 1]
        // l< luminance(ab) component in [0, 1]
        HCLab hcLabColor = HCLab(colors[i]);
        float x = map(-1,1,0,1,hcLabColor.h);
        float y = map(-1,1,0,1,hcLabColor.c);
        float z = map(-1,1,0,1,hcLabColor.l);
        Vec3f newPos = Vec3f(x,y,z);
        vertex[i].lerp(newPos, 0.01);
      }
    } else if (keyMode == 7) {
      auto& vertex = mesh.vertices();
      auto colors = mesh.colors();
      for (int i = 1; i < colors.size(); i++) {
        //  l< Lightness component in [0, 100]
        //  u< red-green axis in [-82.7886, 174.378] (8-bit rgb gamut)
        //  v< yellow-blue axis in [-133.556, 107.025]
        Luv luvColor = Luv(colors[i]);
        float x = map(-1,1,0, 100,luvColor.l);
        float y = map(-1,1,-82.7886, 174.378,luvColor.u);
        float z = map(-1,1,-133.556, 107.025,luvColor.v);
        Vec3f newPos = Vec3f(x,y,z);
        vertex[i].lerp(newPos, 0.01);
      }
    }
    else if (keyMode == 8) {
      auto& vertex = mesh.vertices();
      auto colors = mesh.colors();
      float speed = map(0.01,0.05,0, 1,colors[0].r);
      vertex[0].lerp(vertex.back(), speed);
      for (int i = 1; i < vertex.size(); i++) {
        float speed = map(0.01,0.05,0, 1,colors[i].r);
        vertex[i].lerp(vertex[i - 1], speed);
      }
    }
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
