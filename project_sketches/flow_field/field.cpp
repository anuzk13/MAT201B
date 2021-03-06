// Flow field visualization
// 2022-01-20

#include "al/app/al_App.hpp"
#include "al/io/al_CSVReader.hpp"

using namespace al;


typedef struct {
  int id;
  double y,x,dy,dx,dy_norm,dx_norm,norm,norm_norm;
} FlowPoint;


struct AlloApp : App {

  CSVReader reader;
  std::vector<FlowPoint> rows;
  Mesh fieldMesh;
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
    reader.readFile("data/data_smooth_2.csv");

    rows = reader.copyToStruct<FlowPoint>();

    fieldMesh = Mesh(Mesh::LINES);
    float scale = 1;
    for (int i = 0; i < rows.size(); ++i) {
        
        float originX = map(-1.5f,1.5f,0,1201,rows[i].x);
        float originY = map(1.5f,-1.5f,0,1783,rows[i].y);
        Vec3f originPoint = Vec3f(originX, originY, 0.f);
        float endX = map(-1.5f,1.5f,0,1201,rows[i].x + rows[i].dx_norm * scale);
        float endY = map(1.5f,-1.5f,0,1783,rows[i].y + rows[i].dy_norm * scale);
        Vec3f endPoint = Vec3f(endX, endY,  0.f);

        Color color = HSV(rows[i].norm_norm, 1.0f, 1.0f); 

        // here we're rendering a point based on the vector field
        fieldMesh.vertex(originPoint);
        fieldMesh.color(color);
        fieldMesh.vertex(endPoint);
        fieldMesh.color(color);
    }
  }

  void onCreate() override {
    nav().pos(0,0,10);
  }


  void onAnimate(double dt) override {
  }

  bool onKeyDown(const Keyboard &k) override {
    return true;
  }

  float map (float min_d, float max_d, float min_o, float max_o, float x) {
    return (max_d-min_d)*(x - min_o) / (max_o - min_o) + min_d ;
  }

  void onDraw(Graphics &g) override {
    g.clear(0.3);
     // use the color stored in the mesh
    g.meshColor();
    g.draw(fieldMesh);
  }
};

int main() {
  AlloApp app;
  app.configureAudio(48000, 512, 2, 0);
  app.start();
}
