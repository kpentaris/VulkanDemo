//
// Created by PentaKon on 8/11/2020.
//

#include "Shaders.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

bool readShaderFile(const std::string &filename, std::vector<char> &fileContent) {
  std::ifstream shaderFile(filename, std::ios::ate | std::ios::binary);

  if(!shaderFile.is_open()) {
    std::cerr << "Failed to open shader file " << filename << std::endl;
    return false;
  }
  size_t fileSize = static_cast<size_t>(shaderFile.tellg());
  fileContent.resize(fileSize);
  shaderFile.seekg(0);
  shaderFile.read(fileContent.data(), fileSize);
  shaderFile.close();

  return true;
}
