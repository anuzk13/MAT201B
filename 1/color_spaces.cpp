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
  double t = 1.0;
  vector<Vec3f> posMap;
  vector<Vec3f> rgbMap;
  vector<Vec3f> hsvMap;
  vector<Vec3f> cieMap;
  vector<Vec3f> labMap;
  vector<Vec3f> hclabMap;
  vector<Vec3f> luvMap;

  void onCreate() {
    keyMode = 1;
    // Load a .jpg file
    const char *filename = "./data/colors2.jpg";

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
        float mapX = map(-1,1,0,imgWidth,(float)i);
        float mapY = map(1,-1,0,imgHeight,(float)j);
         // remap from 0, 255 to 0,1
        float mapR = map(0,1,0,255,(float)pixel.r);
        float mapG = map(0,1,0,255,(float)pixel.g);
        float mapB = map(0,1,0,255,(float)pixel.b);
        mesh.vertex(mapX,mapY);
        Color color = Color(mapR,mapG,mapB);
        mesh.color(color);
        posMap.push_back(Vec3f(mapX,mapY,0));

        // rbg position
        float x = map(-1,1,0,1,mapR);
        float y = map(-1,1,0,1,mapG);
        float z = map(-1,1,0,1,mapB);
        rgbMap.push_back(Vec3f(mapR,mapG,mapB));

        // hsv position
        HSV hsvColor = HSV(color);
        float degrees = map(0,360,0,1,hsvColor.h);
        float radians = degrees * M_PI / 180.0;
        x = hsvColor.s * cos(radians);
        y = hsvColor.s * sin(radians);
        z = map(-1,1,0,1,hsvColor.v);
        hsvMap.push_back(Vec3f(x,y,z));

        // cie position
        //  r< red component in [0, 1]
        //  g< green component in [0, 1]
        //  b< blue component in [0, 1]
        CIE_XYZ cieColor = CIE_XYZ(color);
        x = map(-1,1,0,1,cieColor.x);
        y = map(-1,1,0,1,cieColor.y);
        z = map(-1,1,0,1,cieColor.z);
        cieMap.push_back(Vec3f(x,y,z));

        // lab position
        //  l< Lightness component in [0, 100]
        //  a< red-green axis (red is positive, green is negative)
        //    range in [-85.9293, 97.9631] (8-bit rgb gamut)
        //  b< yellow-blue axis (yellow is positive, blue is
        //    negative) range in [-107.544, 94.2025]
        Lab labColor = Lab(color);
        x = map(-1,1,0,100,labColor.l);
        y = map(-1,1,-85.9293, 97.9631,labColor.a);
        z = map(-1,1,-107.544, 94.2025,labColor.b);
        labMap.push_back(Vec3f(x,y,z));

        // hclab position
        // h< hue component in [0, 1]
        // c< chroma component in [0, 1]
        // l< luminance(ab) component in [0, 1]
        HCLab hcLabColor = HCLab(color);
        x = map(-1,1,0,1,hcLabColor.h);
        y = map(-1,1,0,1,hcLabColor.c);
        z = map(-1,1,0,1,hcLabColor.l);
        Vec3f newPos = Vec3f(x,y,z);
        hclabMap.push_back(newPos);

        // luv position
         //  l< Lightness component in [0, 100]
        //  u< red-green axis in [-82.7886, 174.378] (8-bit rgb gamut)
        //  v< yellow-blue axis in [-133.556, 107.025]
        Luv luvColor = Luv(color);
        x = map(-1,1,0, 100,luvColor.l);
        y = map(-1,1,-82.7886, 174.378,luvColor.u);
        z = map(-1,1,-133.556, 107.025,luvColor.v);
        luvMap.push_back(Vec3f(x,y,z));
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
    t = 0.0;
    switch (k.key()) {
      // For printable keys, we just use its character symbol:
      case '1':
        // Image mode
        keyMode = 1;
        break;
      case '2' :
        // RGB Mode
        keyMode = 2;
        break; 
      case '3' :
        // HSV Mode
        keyMode = 3;
        break; 
      case '4' :
        // CIE Mode
        keyMode = 4;
        break; 
      case '5' :
        // Lab Mode
        keyMode = 5;
        break; 
      case '6' :
        // HCLab Mode
        keyMode = 6;
        break; 
      case '7' :
        // Luv Mode
        keyMode = 7;
        break; 
      case '8' :
        // Loop Animation Mode
        keyMode = 8;
        break; 
      default:
        break;
    }
    return true;
  }

  void onAnimate(double dt_ms) {
    // if (t < 1.0) {
    //   t += dt_ms;
    // } else {
    //   t = 1.0;
    // }
    if (keyMode == 1) {
      auto& vertex = mesh.vertices();
      for (int i = 1; i < vertex.size(); i++) {
        vertex[i].lerp(posMap[i], t);
      }
      
    } else if (keyMode == 2) {
      auto& vertex = mesh.vertices();
      for (int i = 1; i < vertex.size(); i++) {
        vertex[i].lerp(rgbMap[i], t);
      }
    } else if (keyMode == 3) {
      auto& vertex = mesh.vertices();
      for (int i = 1; i < vertex.size(); i++) {
        vertex[i].lerp(hsvMap[i], t);
      }
    } else if (keyMode == 4) {
      auto& vertex = mesh.vertices();
      for (int i = 1; i < vertex.size(); i++) {
        vertex[i].lerp(cieMap[i], t);
      }
    } else if (keyMode == 5) {
      auto& vertex = mesh.vertices();
      for (int i = 1; i < vertex.size(); i++) {
        vertex[i].lerp(labMap[i], t);
      }
    } else if (keyMode == 6) {
      auto& vertex = mesh.vertices();
      for (int i = 1; i < vertex.size(); i++) {
        vertex[i].lerp(hclabMap[i], t);
      }
    } else if (keyMode == 7) {
      auto& vertex = mesh.vertices();
      for (int i = 1; i < vertex.size(); i++) {
        vertex[i].lerp(luvMap[i], t);
      }
    }
    else if (keyMode == 8) {
      auto& vertex = mesh.vertices();
      auto colors = mesh.colors();
      float speed = map(t,0.05,0, 1,colors[0].r);
      vertex[0].lerp(vertex.back(), speed);
      for (int i = 1; i < vertex.size(); i++) {
        float speed = map(t,0.05,0, 1,colors[i].r);
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
