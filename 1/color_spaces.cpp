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
#include "al/graphics/al_Image.hpp"

using namespace al;
using namespace std;

class MyApp : public App {
public:
  Texture texture;
  Mesh mesh;


  void onCreate() {
    // Load a .jpg file
    //
    const char *filename = "./data/hubble.jpg";

    auto imageData = Image(filename);

    if (imageData.array().size() == 0) {
      cout << "failed to load image" << endl;
    }
    cout << "loaded image size: " << imageData.width() << ", "
         << imageData.height() << endl;

    texture.create2D(imageData.width(), imageData.height());
    texture.submit(imageData.array().data(), GL_RGBA, GL_UNSIGNED_BYTE);

    texture.filter(Texture::LINEAR);

    // Generate the geometry onto which to display the texture
    mesh.primitive(Mesh::TRIANGLE_STRIP);
    mesh.vertex(-1, 1);
    mesh.vertex(-1, -1);
    mesh.vertex(1, 1);
    mesh.vertex(1, -1);

    // Add texture coordinates
    mesh.texCoord(0, 1);
    mesh.texCoord(0, 0);
    mesh.texCoord(1, 1);
    mesh.texCoord(1, 0);

    nav().pullBack(4);
  }

  void onDraw(Graphics &g) {
    g.clear(0.2f);

     // We must tell the GPU to use the texture when rendering primitives
    texture.bind();
    g.texture(); // Use texture for mesh coloring
    g.draw(mesh);
    texture.unbind();
  }
};

int main() {
  MyApp app;
  app.dimensions(600, 400);
  app.title("imageTexture");
  app.start();
}
