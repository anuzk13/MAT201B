// Karl Yerkes
// MAT201B
// 2022-01-04
// minimal app, ready for adapting..
//

#include "al/app/al_App.hpp"
#include "al/io/al_CSVReader.hpp"

using namespace al;
using namespace std;

struct MyApp : App {
// outside is 422 inside is 1000 each
  std::vector<std::vector<float>> data;

  void onCreate() override {
    CSVReader reader;
    reader.addType(CSVReader::REAL);
    reader.readFile("data/Y_voltage_force_flatten_transpose.csv");
    float data[422][1000];
    std::vector<double> column0 = reader.getColumn(0);
    for (int i = 0; i < 422; i++) {
        auto v = data[i];
        for (int j = 0; j < 1000; j++) {
            v[j] = column0[j+i*1000];
        }
    }
  }

  void onAnimate(double dt) override {
    
  }

  void onDraw(Graphics& g) override {
    g.clear(1, 0, 0);
    //g.clear(0.25);
    //
  }

  bool onKeyDown(const Keyboard& k) override {
    //
    for (int i = 0; i < 422; i++) {
        auto v = data[i];
        for (int j = 0; j < 1000; j++) {
            cout<< "the array matrix is:"<< v[j] <<endl;
        }
    }
    return false;
  }
  
  void onSound(AudioIOData& io) override {
    //
  }
};

int main() {
  MyApp app;
  app.start();
}
