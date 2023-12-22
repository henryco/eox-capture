//
// Created by henryco on 12/21/23.
//

#include "voxels.h"

namespace eox::ogl {

    void Voxels::init() {
        {
            // TODO VAO/VBO/EBO magic
        }
        shader.init();
    }

    void Voxels::render() {
        // TODO render
    }

    Voxels::~Voxels() {
        cleanup();
    }

    void Voxels::cleanup() {

    }

} // eox