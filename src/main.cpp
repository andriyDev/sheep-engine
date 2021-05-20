
#include <stdio.h>

#include <memory>
#include <string>

#include "resource.h"

class TextTest {
 public:
  using detail_type = std::string;

  static std::shared_ptr<TextTest> Load(const std::string& text) {
    return std::shared_ptr<TextTest>(new TextTest{text});
  }

  std::string value;
};

int main() {
  ResourceLoader loader;
  loader.Add<TextTest>("a", "Text A");
  loader.Add<TextTest>("b", "Text B");
  loader.Add<TextTest>("c", "Text C");

  printf("a = %s\n", loader.Load<TextTest>("a")->value.c_str());
  printf("b = %s\n", loader.Load<TextTest>("b")->value.c_str());
  printf("c = %s\n", loader.Load<TextTest>("c")->value.c_str());
  return 0;
}
