/*
  This example shows how to use Image, Array and Texture to read a .jpg file,
display it as an OpenGL texture and print the pixel values on the command line.
Notice that while the intput image has only 4 pixels, the rendered texture is
smooth.  This is because interpolation is done on the GPU.

  Karl Yerkes and Matt Wright (2011/10/10)
*/


#include "al/app/al_App.hpp"
#include "al/math/al_Functions.hpp"
#include "al/graphics/al_Image.hpp"
#include "al/math/al_Random.hpp"
#include "al/io/al_CSVReader.hpp"

using namespace al;
using namespace std;

class MyApp : public App {
public:

  float timeStep = 0.1;
  float maxSpeedVF = 0.01;
  float showField = 0.0;

  VAOMesh mesh;
  vector<Vec3f> velocity;
  vector<Vec3f> acceleration;

  int imgWidth, imgHeight;

  int fieldWidth = 400;
  int fieldHeight = 300;

  void onInit() override {

  }

  void onCreate() {
    // Load a .jpg file
    // const char *mapImage = "./data/displacement.png";
    const char *mapImage = "./data/displacement_sm.png";

    auto mapData = Image(mapImage);

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
        auto pixel = mapData.at(i, j);
         // remap from 0, width to -1,1
        float x = map(-1.f,1.f,0,imgWidth,(float)i);
        float y = map(1.f,-1.f,0,imgHeight,(float)j);
         // remap from 0, 255 to 0,1
        float r = map(0,1,0,255,(float)pixel.r);
        float g = map(0,1,0,255,(float)pixel.g);
        float b = map(0,1,0,255,(float)pixel.b);
        Color color = Color(r,g,b);
        HSV hsvColor = HSV(color);
        float z = map(0,0.2,0,1,hsvColor.v);

        mesh.vertex(x,y,z);
        mesh.color(color);

        velocity.push_back(0);
        acceleration.push_back(0);
      }

    }
    // Generate the geometry onto which to display the texture
    mesh.primitive(Mesh::POINTS);
    nav().pullBack(4);
    auto& vertex = mesh.vertices();
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
  
  }

  bool onKeyDown(const Keyboard &k) {
    return true;
  }

  void onAnimate(double dt_ms) {

    double dt = timeStep;
    auto& vertex = mesh.vertices();

    for (int i = 0; i < velocity.size(); i++) {
      // "semi-implicit" Euler integration
      velocity[i] += acceleration[i] * dt;
      vertex[i] += velocity[i] * dt;
    }

    // clear all accelerations (IMPORTANT!!)
    for (auto &a : acceleration) a.zero();
    mesh.update();

  }

  float map (float min_d, float max_d, float min_o, float max_o, float x) {
    return (max_d-min_d)*(x - min_o) / (max_o - min_o) + min_d ;
  }

};

int main() {
  MyApp app;
  app.title("imageTexture");
  app.start();
}
