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

typedef struct {
  int id;
  double y,x,dy,dx,dy_norm,dx_norm,norm,norm_norm;
} FlowPoint;

class MyApp : public App {
public:

  CSVReader reader;
  std::vector<FlowPoint> rows;
  Mesh fieldMesh;
  vector<Vec3f> victimsForces;

  VAOMesh mesh;
  vector<Vec3f> velocity;
  vector<Vec3f> acceleration;

  int imgWidth, imgHeight;
  int fieldWidth = 1201;
  int fieldHeight = 1783;
  float maxSpeedVF = 5.0;

  void onInit() override {
    //  
    reader.addType(CSVReader::INT64);
    reader.addType(CSVReader::REAL);
    reader.addType(CSVReader::REAL);
    reader.addType(CSVReader::REAL);
    reader.addType(CSVReader::REAL);
    reader.addType(CSVReader::REAL);
    reader.addType(CSVReader::REAL);
    reader.addType(CSVReader::REAL);
    reader.addType(CSVReader::REAL);
    reader.readFile("data/victims_data.csv");

    rows = reader.copyToStruct<FlowPoint>();

    fieldMesh = Mesh(Mesh::LINES);
    float scale = 1;
    for (int i = 0; i < rows.size(); ++i) {
        
        float originX = map(-1.f,1.f,0,fieldWidth,rows[i].x);
        float originY = map(1.f,-1.f,0,fieldHeight,rows[i].y);
        Vec3f originPoint = Vec3f(originX, originY, 0.f);
        float endX = map(-1.f,1.f,0,fieldWidth,rows[i].x + rows[i].dx_norm * scale);
        float endY = map(1.f,-1.f,0,fieldHeight,rows[i].y + rows[i].dy_norm * scale);
        Vec3f endPoint = Vec3f(endX, endY,  0.f);

        Color color = HSV(rows[i].norm_norm, 1.0f, 1.0f); 

        // here we're rendering a point based on the vector field
        fieldMesh.vertex(originPoint);
        fieldMesh.color(color);
        fieldMesh.vertex(endPoint);
        fieldMesh.color(color);
        victimsForces.push_back((endPoint - originPoint).normalize());
    }
  }

  void onCreate() {
    // Load a .jpg file
    const char *mapImage = "./data/displacement.png";

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

    g.draw(fieldMesh);
  
  }

  bool onKeyDown(const Keyboard &k) {
    return true;
  }

  void onAnimate(double dt_ms) {
    auto& vertex = mesh.vertices();

    // vector field
    for (int i = 0; i < vertex.size(); i++) {
      if (abs(vertex[i].x) <= 1.0f &&  abs(vertex[i].y) <= 1.0f) {
        Vec3f steer = getFieldVector(vertex[i]) * maxSpeedVF - velocity[i];
        // This line causes the program to crash but I am not sure why
        // acceleration[i] += steer;
      }
    }
    // vertex[0].lerp(vertex[vertex.size()-1], 0.1);
    // for (int i = 1; i < vertex.size(); i++) {
    //   vertex[i].lerp(vertex[i-1], 0.1);
    // }
    for (int i = 0; i < velocity.size(); i++) {
      // "semi-implicit" Euler integration
      velocity[i] += acceleration[i] * dt_ms;
      vertex[i] += velocity[i] * dt_ms;
    }

    // clear all accelerations (IMPORTANT!!)
    for (auto &a : acceleration) a.zero();
    mesh.update();
  }

  float map (float min_d, float max_d, float min_o, float max_o, float x) {
    return (max_d-min_d)*(x - min_o) / (max_o - min_o) + min_d ;
  }

   // particle pos is assumed to be given in the screen space coords
  Vec3f getFieldVector(Vec2f particlePos) {
    float x_index = map(0, fieldWidth -1, -1.f, 1.f, particlePos.x);
    float y_index = map(fieldHeight -1 , 0, -1.f, 1.f, particlePos.y);
    return victimsForces[floor(y_index) * fieldHeight + floor(x_index)];
  }

};


int main() {
  MyApp app;
  app.dimensions(600, 400);
  app.title("imageTexture");
  app.start();
}
