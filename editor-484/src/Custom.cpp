#include "Custom.h"
#include "TexCoords.h"

#include <cmath>

namespace {
// sign(w) * |w|^e. Raising a negative base to a fractional power is NaN, so
// the sign is stripped and reapplied.
float signedPow(float base, float exponent) {
    float magnitude = std::pow(std::fabs(base), exponent);
    return (base < 0.0f) ? -magnitude : magnitude;
}
} // namespace

Custom::Custom(float x, float y, float z, float uniformScale, int colorIndex, int id,
               float scaleX, float scaleY, float scaleZ, bool useUniformScaling)
    : Shape(x, y, z, uniformScale, colorIndex, id, scaleX, scaleY, scaleZ, useUniformScaling), VAO(0), VBO(0), EBO(0) {
    shapeType = customShapeName;

    setupCustom();      // Prepare OpenGL buffers

}

Custom::~Custom() {
    // Cleanup OpenGL resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Custom::setupCustom() {
    std::vector<float> vertexData;
    std::vector<unsigned int> indexData;

    // Grid resolution, GIVEN and deliberately outside the marked region below:
    // the texture-coordinate code further down builds its grid from these, so
    // they have to survive when the geometry is stripped. If your own shape is
    // not built from a (u, v) grid, ignore them -- the UV code will simply not
    // line up until Assignment 5, which is expected.
    const int uSteps = 64;
    const int vSteps = 48;

    // TODO(geometry): a shape of your own choosing, at least twelve faces
    // Build the shape: fill `vertices`, `faces`, and `normals` (directly or
    // by calling calculateNormals()). See ASSIGNMENTS.md, A2, for the
    // conventions -- roughly one unit across, centred on the origin,
    // counter-clockwise winding seen from outside, every face index a valid
    // index into `vertices`.
    //
    // This one is yours to design: at least twelve faces, and something you
    // can explain. Name it by editing customShapeName in src/Globals.cpp.
    //
    // What is here is a placeholder square so the editor runs and the Insert
    // menu does something visible. Read src/Torus.cpp first -- it is the
    // worked example of a procedural shape.
    vertices = {
        // Front
        { 0.5f,  1.5f,  0.5f},  // 0
        { 0.5f,  0.5f,  0.5f},  // 1
        { 1.5f,  0.5f,  0.5f},  // 2
        { 1.5f, -0.5f,  0.5f},  // 3
        { 0.5f, -0.5f,  0.5f},  // 4
        { 0.5f, -1.5f,  0.5f},  // 5
        {-0.5f, -1.5f,  0.5f},  // 6
        {-0.5f, -0.5f,  0.5f},  // 7
        {-1.5f, -0.5f,  0.5f},  // 8
        {-1.5f,  0.5f,  0.5f},  // 9
        {-0.5f,  0.5f,  0.5f},  // 10
        {-0.5f,  1.5f,  0.5f},  // 11

        // Back
        { 0.5f,  1.5f, -0.5f},  // 12
        { 0.5f,  0.5f, -0.5f},  // 13
        { 1.5f,  0.5f, -0.5f},  // 14
        { 1.5f, -0.5f, -0.5f},  // 15
        { 0.5f, -0.5f, -0.5f},  // 16
        { 0.5f, -1.5f, -0.5f},  // 17
        {-0.5f, -1.5f, -0.5f},  // 18
        {-0.5f, -0.5f, -0.5f},  // 19
        {-1.5f, -0.5f, -0.5f},  // 20
        {-1.5f,  0.5f, -0.5f},  // 21
        {-0.5f,  0.5f, -0.5f},  // 22
        {-0.5f,  1.5f, -0.5f}   // 23
    };
        
    normals = {
        { 0.0f,  0.0f,  1.0f},  // Front
        { 0.0f,  0.0f, -1.0f},  // Back

        { 1.0f,  0.0f,  0.0f},  // Right
        {-1.0f,  0.0f,  0.0f},  // Left

        { 0.0f,  1.0f,  0.0f},  // Up
        { 0.0f, -1.0f,  0.0f}   // Down
    };

    faces = {
        // Front (+Z)
        { 0, 10,  1},  // face 00 - Front
        { 0, 11, 10},  // face 01 - Front
        {10,  8,  7},  // face 02 - Front
        {10,  9,  8},  // face 03 - Front
        { 1,  7,  4},  // face 04 - Front
        { 1, 10,  7},  // face 05 - Front
        { 2,  4,  3},  // face 06 - Front
        { 2,  1,  4},  // face 07 - Front
        { 4,  6,  5},  // face 08 - Front
        { 4,  7,  6},  // face 09 - Front

        // Back (-Z)
        {13, 22, 12},  // face 10 - Back
        {22, 23, 12},  // face 11 - Back
        {19, 20, 22},  // face 12 - Back
        {20, 21, 22},  // face 13 - Back
        {16, 19, 13},  // face 14 - Back
        {19, 22, 13},  // face 15 - Back
        {15, 16, 14},  // face 16 - Back
        {16, 13, 14},  // face 17 - Back
        {17, 18, 16},  // face 18 - Back
        {18, 19, 16},  // face 19 - Back

        // Right (+X)
        { 0,  1, 13},  // face 20
        { 0, 13, 12},  // face 21

        { 2,  3, 15},  // face 22
        { 2, 15, 14},  // face 23

        { 4,  5, 17},  // face 24
        { 4, 17, 16},  // face 25

        // Left (-X)
        { 6,  7, 19},  // face 26
        { 6, 19, 18},  // face 27

        { 8,  9, 21},  // face 28
        { 8, 21, 20},  // face 29

        {10, 11, 23},  // face 30
        {10, 23, 22},  // face 31

        // Up (+Y)
        { 1,  2, 14},  // face 32
        { 1, 14, 13},  // face 33

        { 9, 10, 22},  // face 34
        { 9, 22, 21},  // face 35

        {11,  0, 12},  // face 36
        {11, 12, 23},  // face 37

        // Down (-Y)
        { 3,  4, 16},  // face 38
        { 3, 16, 15},  // face 39

        { 5,  6, 18},  // face 40
        { 5, 18, 17},  // face 41

        { 7,  8, 20},  // face 42
        { 7, 20, 19}   // face 43
    };

    // The shape's own (u, v) grid parameters, the same way the sphere uses
    // its own. flipU for the same handedness reason as Sphere: a generator that
    // sweeps u anticlockwise seen from above mirrors the image without it.
    const std::vector<glm::vec2> gridUVs =
        TexCoords::parametricGrid(uSteps, vSteps, true, false);
    texCoords.clear();

    // Build vertex data and index data for OpenGL, using the PER-VERTEX normal
    // so the surface is smooth rather than faceted.
    //
    // SKIPS ANY FACE THAT INDEXES OUTSIDE `vertices` OR `normals`. This shape
    // is the one the student designs, so its topology is whatever they made it
    // -- and an off-by-one in their face indices is a normal thing to hit
    // halfway through. Without this guard that mistake is not a wrong-looking
    // shape, it is an out-of-bounds read: undefined behaviour that usually
    // lands in heap slack and renders garbage, and occasionally crashes on
    // someone else's machine. The handout warns students about exactly this
    // ("an out-of-range index reads memory that is not yours"), so the editor
    // must not do it to them.
    //
    // Skipping rather than clamping is deliberate: a clamped face draws a
    // plausible-looking triangle and hides the bug, while a missing one is
    // visible and geometry_test says so in as many words.
    size_t emitted = 0;
    for (size_t i = 0; i < faces.size(); ++i) {

        bool usable = (faces[i].size() == 3);
        for (size_t j = 0; usable && j < 3; ++j) {
            const int vi = faces[i][j];
            if (vi < 0 ||
                vi >= static_cast<int>(vertices.size())) {
                usable = false;
            }
        }
        if (!usable) continue;

        glm::vec3 color = (colorIndex == 31)
            ? glm::vec3(customColor[0], customColor[1], customColor[2])
            : glm::vec3(colorPresets[colorIndex].color[0],
                        colorPresets[colorIndex].color[1],
                        colorPresets[colorIndex].color[2]);

        for (int j = 0; j < 3; ++j) {
            int vertexIndex = faces[i][j];
            const glm::vec3& position = vertices[vertexIndex];
            int normalIndex = i < 10 ? 0 :
                            i < 20 ? 1 :
                            i < 26 ? 2 :
                            i < 32 ? 3 :
                            i < 38 ? 4 : 5;

            const glm::vec3& normal = normals[normalIndex];

            vertexData.insert(vertexData.end(), {position.x, position.y, position.z});
            vertexData.insert(vertexData.end(), {normal.x, normal.y, normal.z});
            vertexData.insert(vertexData.end(), {color.r, color.g, color.b});

            // Expanded per corner: indexData below is sequential, so
            // attribute 3 must match the interleaved buffer's length.
            texCoords.push_back(vertexIndex < static_cast<int>(gridUVs.size())
                                ? gridUVs[static_cast<size_t>(vertexIndex)]
                                : glm::vec2(0.0f));
        }

        // Counts EMITTED faces, not the loop index: a skipped face must not
        // leave a gap in the sequential index buffer.
        indexData.insert(indexData.end(), {static_cast<unsigned int>(emitted * 3),
                                            static_cast<unsigned int>(emitted * 3 + 1),
                                            static_cast<unsigned int>(emitted * 3 + 2)});
        ++emitted;
    }

    // Create and bind VAO, VBO, and EBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(unsigned int), indexData.data(), GL_STATIC_DRAW);

    // Configure vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0); // Position
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float))); // Normal
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float))); // Color
    glEnableVertexAttribArray(2);

    uploadTexCoords();

    glBindVertexArray(0); // Unbind VAO
    
}

void Custom::draw(GLuint shaderProgram) {

    // Use the shader program
    glUseProgram(shaderProgram);
    
    // Enable lighting for the cube
    GLint lightingLoc = glGetUniformLocation(shaderProgram, "useLighting");
    if (lightingLoc != -1) {
        glUniform1i(lightingLoc, 1); // Enable lighting for the cube
    }

    // Apply transformations and pass to the shader
    applyTransform(shaderProgram);

    // Pass material properties
    GLint colorLoc = glGetUniformLocation(shaderProgram, "material.color");
    if (colorLoc != -1) {
        glUniform3fv(colorLoc, 1, (colorIndex == 31) ? customColor : colorPresets[colorIndex].color);
    }

    // Render the cube
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(faces.size() * 3), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Disable lighting after drawing the cube (for axis rendering)
    if (lightingLoc != -1) {
        glUniform1i(lightingLoc, 0);
    }
}

