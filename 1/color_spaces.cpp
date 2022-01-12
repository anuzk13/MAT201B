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
        float mapY = map(-1,1,0,imgHeight,(float)j);
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
      // Press space bar to change mode
      keyMode = 1;
    } else if (k.key() == '2') {
      // Press s to change mode
      keyMode = 2;
    }  else if (k.key() == '3') {
      // Press s to change mode
      keyMode = 3;
    }
    else if (k.key() == '4') {
      // Press s to change mode
      keyMode = 4;
    }
    return true;
  }

  void onAnimate(double dt_ms) {
    if (keyMode == 1) {
      auto& vertex = mesh.vertices(); // 'vertex' becomes an alias for 'mesh.vertices()'
      for (int i = 1; i < vertex.size(); i++) {
        int x = i % imgWidth;
        int y = i / imgWidth;
        float mapX = map(-1,1,0,imgWidth,(float)x);
        float mapY = map(-1,1,0,imgHeight,(float)y);
        Vec3f newPos = Vec3f(mapX,mapY,0);
        vertex[i].lerp(newPos, 0.01);
      }
      
    } else if (keyMode == 2) {
      auto& vertex = mesh.vertices(); // 'vertex' becomes an alias for 'mesh.vertices()'
      auto& colors = mesh.colors();
      for (int i = 1; i < vertex.size(); i++) {
        float mapR = map(-1,1,0,1,colors[i].r);
        float mapG = map(-1,1,0,1,colors[i].g);
        float mapB = map(-1,1,0,1,colors[i].b);
        Vec3f newPos = Vec3f(mapR,mapG,mapB);
        vertex[i].lerp(newPos, 0.01);
      }
    } else if (keyMode == 3) {
      // this might have broken due new values of color
      auto& vertex = mesh.vertices(); // 'vertex' becomes an alias for 'mesh.vertices()'
      auto& colors = mesh.colors();
      for (int i = 1; i < colors.size(); i++) {
        // use the constructor of HSV instead of the function
        // @param[in] v      RGB color to convert from
        // HSV(const RGB &v) { *this = v; }
        Vec3f hsv = rgb2hsv(colors[i]);
        float x = hsv[1] * cos(hsv[0]);
        float y = hsv[1] * sin(hsv[0]);
        float z = hsv[2];
        Vec3f newPos = Vec3f(x*10,y*10, z/10);
        vertex[i].lerp(newPos, 0.01);
      }
    } else if (keyMode == 4) {
      // TODO: my custom animation
    }
  }

  float map (float min_d, float max_d, float min_o, float max_o, float x) {
    return (max_d-min_d)*(x - min_o) / (max_o - min_o) + min_d ;
  }

  // modified from https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
  Vec3f rgb2hsv(Color in)
  {
      float h,s,v;
      double  min, max, delta;

      min = in.r < in.g ? in.r : in.g;
      min = min  < in.b ? min  : in.b;

      max = in.r > in.g ? in.r : in.g;
      max = max  > in.b ? max  : in.b;

      v = max;                                // v
      delta = max - min;
      if (delta < 0.00001)
      {
          s = 0;
          h = 0; // undefined, maybe nan?
          return Vec3f(h,s,v);
      }
      if( max > 0.0 ) { // NOTE: if Max is == 0, this divide would cause a crash
          s = (delta / max);                  // s
      } else {
          // if max is 0, then r = g = b = 0              
          // s = 0, h is undefined
          s = 0.0;
          h = NAN;                            // its now undefined
          return Vec3f(h,s,v);
      }
      if( in.r >= max )                           // > is bogus, just keeps compilor happy
          h = ( in.g - in.b ) / delta;        // between yellow & magenta
      else
      if( in.g >= max )
          h = 2.0 + ( in.b - in.r ) / delta;  // between cyan & yellow
      else
          h = 4.0 + ( in.r - in.g ) / delta;  // between magenta & cyan

      h *= 60.0;                              // degrees

      if( h < 0.0 )
          h += 360.0;

      return Vec3f(h,s,v);;
  }

};


int main() {
  MyApp app;
  app.dimensions(600, 400);
  app.title("imageTexture");
  app.start();
}
