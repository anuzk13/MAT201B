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
#include "al/math/al_Random.hpp"

using namespace al;
using namespace std;

class MyApp : public App {
public:
  vector<Vec2f> forceVectors; // meters/second
  VAOMesh mesh;
  vector<Vec3f> velocity;
  vector<Vec3f> acceleration;

  vector<float> victimValues;

  int imgWidth, imgHeight;

  Vec3f randomVec3f(float scale) {
    return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()) * scale;
  }

  void onCreate() {
    // Load a .jpg file
    const char *mapImage = "./data/displacement.png";
    const char *victimsImage = "./data/dots_topography.png";

    auto mapData = Image(mapImage);
    auto victimsData = Image(victimsImage);

    if (mapData.array().size() == 0) {
      cout << "failed to load image" << endl;
      exit(1);
    }
    cout << "loaded image size: " << mapData.width() << ", "
         << mapData.height() << endl;

    // Asumes both images have the same size
    imgWidth = mapData.width();
    imgHeight = mapData.height();

    for (int j = 0; j < imgHeight; ++j) {
      for (int i = 0; i < imgWidth; ++i) {
        auto pixelV = victimsData.at(i, j);
        auto pixel = mapData.at(i, j);
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

        float rV = map(0,1,0,255,(float)pixelV.r);
        float gV = map(0,1,0,255,(float)pixelV.g);
        float bV = map(0,1,0,255,(float)pixelV.b);
        Color colorV = Color(rV,gV,bV);
        HSV hsvColorv = HSV(colorV);
        float valueV = map(0,0.2,0,1,hsvColorv.v);
        victimValues.push_back(valueV);

        mesh.vertex(x,y,z);
        mesh.color(color);

        velocity.push_back(0);
        acceleration.push_back(0);
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

    g.meshColor();
    // Mesh fieldMesh(Mesh::LINES);
    // for (int j = 0; j < field.resY; ++j) {
    //   for (int i = 0; i < field.resX; ++i) {
    //     Vec2f orientation = field.vectors[i*field.resX + j];
    //     float originX = map(-1.f,1.f,0,field.resX,i);
    //     float originY = map(-1.f,1.f,0,field.resY,j);
    //     Vec3f originPoint = Vec3f(originX, originY, 0.f);
    //     Vec3f endPoint = Vec3f(originX + orientation.x,
    //                             originY + orientation.y,  0.f);

    //     Color color = Color(1,0,0);

    //     // here we're rendering a point based on the vector field
    //     fieldMesh.vertex(originPoint);
    //     fieldMesh.color(color);
    //     fieldMesh.vertex(endPoint);
    //     fieldMesh.color(color);
    //   }
    // }
    // g.draw(fieldMesh);
  }

  bool onKeyDown(const Keyboard &k) {
    return true;
  }

  void onAnimate(double dt_ms) {
    auto& vertex = mesh.vertices();
    vertex[0].lerp(vertex[vertex.size()-1], 0.1);
    for (int i = 1; i < vertex.size(); i++) {
      vertex[i].lerp(vertex[i-1], 0.1);
    }
    mesh.update();
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
