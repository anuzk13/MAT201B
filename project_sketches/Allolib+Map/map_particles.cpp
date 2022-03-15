/*
  This example shows how to use Image, Array and Texture to read a .jpg file,
display it as an OpenGL texture and print the pixel values on the command line.
Notice that while the intput image has only 4 pixels, the rendered texture is
smooth.  This is because interpolation is done on the GPU.

  Karl Yerkes and Matt Wright (2011/10/10)
*/


#include "al/app/al_App.hpp"
#include "al/graphics/al_Image.hpp"
#include "al/io/al_CSVReader.hpp"
#include "al/app/al_GUIDomain.hpp"

using namespace al;
using namespace std;

typedef struct {
  double y,x,dy,dx,dy_norm,dx_norm,norm,norm_norm;
} FlowPoint;

class MyApp : public App {
public:

  // Parameter maxSpeed{"/maxSpeed", "", 0.02, 0.01, 0.6};
  // // sensile to this conditions
  // Parameter timeStep{"/timeStep", "", 0.01, 0.01, 0.6};
  Parameter maxSpeed{"/maxSpeed", "", 0.02, 0.01, 0.6};
  Parameter timeStep{"/timeStep", "", 0.01, 0.01, 0.6};
  Parameter spreadFactor{"/spreadFactor", "", 0.5, -10, 10};
  ParameterBool showField{"/showField", "", 0.0};

  // victim' data
  std::vector<FlowPoint> rows;
  Mesh fieldMesh;
  vector<Vec3f> victimsForces;

  // displacement map data
  VAOMesh mesh;
  vector<Vec3f> velocity;
  vector<Vec3f> acceleration;

  int imgWidth, imgHeight;
  int fieldWidth = 1201;
  int fieldHeight = 1783;

  double angle{0};

  // image size
  // 1201 x 1782

  void onInit() override {

    // Gui params
    auto GUIdomain = GUIDomain::enableGUI(defaultWindowDomain());
    auto& gui = GUIdomain->newGUI();
    gui.add(timeStep); // add parameter to GUI
    gui.add(maxSpeed);
    gui.add(showField);
    gui.add(spreadFactor);
  
    // read CSV info
    CSVReader reader;
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
  }

  void onCreate() {
    // create visualization of victim's data field
    fieldMesh = Mesh(Mesh::LINES);
    for (int i = 0; i < rows.size(); ++i) {
      if (abs(rows[i].dx_norm > 0) || abs(rows[i].dy_norm) > 0) {
        float originX = map(-1.f,1.f,0,fieldWidth,rows[i].x);
        float originY = map(1.f,-1.f,0,fieldHeight,rows[i].y);
        Vec3f originPoint = Vec3f(originX, originY, 0.f);
        float endX = map(-1.f,1.f,0,fieldWidth,rows[i].x + rows[i].dx_norm);
        float endY = map(1.f,-1.f,0,fieldHeight,rows[i].y + rows[i].dy_norm);
        Vec3f endPoint = Vec3f(endX, endY,  0.f);

        Color color = HSV(rows[i].norm_norm, 1.0f, 1.0f);
        // Color color = HSV(0.0f, 1.0f, 1.0f); 

        // here we're rendering a point based on the vector field
        fieldMesh.vertex(originPoint);
        fieldMesh.color(color);
        fieldMesh.vertex(endPoint);
        fieldMesh.color(color);
        Vec3f diff = (originPoint - endPoint).normalize();
        float zDir = max(abs(rows[i].dx_norm),abs(rows[i].dy_norm));
        // victimsForces.push_back(Vec3f(diff.x, diff.y, zDir));
        victimsForces.push_back(diff);
      } else {
        victimsForces.push_back(Vec3f(0,0,0));
      }
    }

    // read map data
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
        if (pixel.a) {
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

    }
    // Generate the geometry onto which to display the texture
    mesh.primitive(Mesh::POINTS);
    nav().pullBack(6);
    auto& vertex = mesh.vertices();
  }

  void onDraw(Graphics &g) {
    g.rotate(angle, Vec3d(0, 1, 0));
    //color of bakcgrund
    g.clear(0);
    // size of point primitive
    g.pointSize(1);
    // show color of the vertex
    g.meshColor();
     // draw the mesh
    g.draw(mesh);
    // g.draw(wire);
    g.pointSize(8);
    g.meshColor();

    if (showField.get() == 1.0f) {
      g.draw(fieldMesh);
    }
  
  }

  bool onKeyDown(const Keyboard &k) {
    return true;
  }

  void onAnimate(double dt_ms) {

    double dt = timeStep / 100;
    auto& vertex = mesh.vertices();

    // angle += timeStep * 100;

    // vector field
    for (int i = 0; i < vertex.size(); i++) {
      if (abs(vertex[i].x) <= 1.0f &&  abs(vertex[i].y) <= 1.0f) {
        // cout << "looking at vertex" << i << endl;
        tuple<int, Vec3f> fieldVector = getFieldVector(vertex[i]);
        Vec3f direction = get<1>(fieldVector);
        if (direction.mag() > 0) {
          Vec3f steer = (direction - velocity[i])* maxSpeed;
          acceleration[i] += steer;
        } else if (velocity[i].mag() > 0) {
          victimsForces[get<0>(fieldVector)] = velocity[i].normalize() * spreadFactor;
          Color color = HSV(victimsForces[get<0>(fieldVector)].mag(), 1.0f, 1.0f);
          // Color color = HSV(0.0f, 1.0f, 1.0f); 
          fieldMesh.vertex(Vec3f(vertex[i].x, vertex[i].y,0));
          fieldMesh.color(color);
          fieldMesh.vertex(Vec3f(vertex[i].x, vertex[i].y,0) + velocity[i].normalize()/1000);
          fieldMesh.color(color);
        }
      }
    }
   
    for (int i = 0; i < velocity.size(); i++) {
      // "semi-implicit" Euler integration
      velocity[i] += acceleration[i] * dt;
      vertex[i] += velocity[i] * dt;
    }

    // clear all accelerations (IMPORTANT!!)
    for (auto &a : acceleration) a.zero();
    mesh.update();

  }

  // mind_d max_d min max destination
  // min_o max_o 
  float map (float min_d, float max_d, float min_o, float max_o, float x) {
    return (max_d-min_d)*(x - min_o) / (max_o - min_o) + min_d ;
  }

   // particle pos is assumed to be given in the screen space coords
  tuple<int, Vec3f> getFieldVector(const Vec3f& particlePos) {
    int x_index = floor(map(0, fieldWidth - 1, -1.f, 1.f, particlePos.x));
    int y_index = floor(map(fieldHeight - 1, 0, -1.f, 1.f, particlePos.y));
    int index = y_index * fieldWidth + x_index;
    return make_tuple(index, victimsForces[index]);
  }

};


int main() {
  MyApp app;
  app.dimensions(3800 , 2100);
  app.title("imageTexture");
  app.start();
}
