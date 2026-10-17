// Wavefront OBJ parsing -- see include/ObjParser.h for why this is its own
// translation unit. Student work; everything around it is given.

#include "ObjParser.h"

#include <cstdlib>
#include <sstream>

namespace {

// Pulls the three fields out of one "a/b/c" corner. Any of b and c may be
// absent: "a", "a/b", "a//c" and "a/b/c" are all legal OBJ.
//
// Indices are returned already converted to zero-based, or -1 when the field
// was not present. OBJ counts from 1, and OBJ also allows NEGATIVE indices
// meaning "counting back from the end of the list so far", which is why the
// caller passes the current counts in.
void parseCorner(const std::string& field,
                 size_t vCount, size_t vtCount, size_t vnCount,
                 int& vi, int& ti, int& ni) {
    vi = ti = ni = -1;

    const size_t s1 = field.find('/');
    const size_t s2 = (s1 == std::string::npos) ? std::string::npos
                                                : field.find('/', s1 + 1);

    const std::string a = field.substr(0, s1);
    const std::string b = (s1 == std::string::npos) ? std::string()
                        : field.substr(s1 + 1,
                              (s2 == std::string::npos) ? std::string::npos
                                                        : s2 - s1 - 1);
    const std::string c = (s2 == std::string::npos) ? std::string()
                                                    : field.substr(s2 + 1);

    struct Resolve {
        static int one(const std::string& text, size_t count) {
            if (text.empty()) return -1;
            const long raw = std::atol(text.c_str());
            if (raw > 0)  return static_cast<int>(raw - 1);
            if (raw < 0)  return static_cast<int>(static_cast<long>(count) + raw);
            return -1;    // index 0 is not legal in OBJ
        }
    };

    vi = Resolve::one(a, vCount);
    ti = Resolve::one(b, vtCount);
    ni = Resolve::one(c, vnCount);
}

} // namespace

bool parseObj(std::istream& in, ObjMesh& out) {
    // TODO(objparser): parse v, vn, vt and f lines into the ObjMesh
    // Read the stream a line at a time and dispatch on the first token.
    // f corners are "a/b/c" with b and c each optional; indices are
    // 1-based and may be negative (counting back from the end).
    // Resolve each vt index to a coordinate here and push one per corner,
    // so faces keep their stride of 2 (vertex, normal).
    //(void)in;
    out = ObjMesh();
    std::string line;
    std::vector<glm::vec2> texTable;

    while (std::getline(in, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;
        if (token == "v") {
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            out.vertices.push_back(vertex);
        } else if (token == "vn") {
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            out.normals.push_back(normal);
        } else if (token == "vt") {
            glm::vec2 texCoord;
            iss >> texCoord.x >> texCoord.y;
            texTable.push_back(texCoord);
            out.texCoords.push_back(texCoord);
            //out.hasTexCoords = true;
        } else if (token == "f") {
            std::vector<std::string> fields;
            std::string field;
            while (iss >> field) {
                fields.push_back(field);
            }

            std::vector<int> face;
            std::vector<glm::vec2> uvs;
            bool sawuv = false;

            for (int k=0; k < 3 && k < fields.size(); ++k) {
                int vi, ti, ni;
                parseCorner(fields[k], out.vertices.size(), texTable.size(), out.normals.size(), vi, ti, ni);
                face.push_back(vi);
                face.push_back(ni);

                glm::vec2 uv(0.0f, 0.0f);
                if (ti >= 0 && static_cast<size_t>(ti) < texTable.size()) {
                    uv = texTable[ti];
                    sawuv = true;
                }
                uvs.push_back(uv);
            }

            if (face.size() == 6) {  // three (vertex, normal) pairs
                const glm::vec3& p0 = out.vertices[face[0]];
                const glm::vec3& p1 = out.vertices[face[2]];
                const glm::vec3& p2 = out.vertices[face[4]];

                glm::vec3 normal = glm::cross(p1 - p0, p2 - p0);
                const float length = glm::length(normal);
                if (length > 1e-8f) {
                    normal /= length;
                } else {
                    normal = glm::vec3(0.0f, 0.0f, 1.0f);
                }

                const int generatedNormalIndex = static_cast<int>(out.normals.size());
                out.normals.push_back(normal);

                for (int corner = 0; corner < 3; ++corner) {
                    if (face[corner * 2 + 1] < 0) {
                        face[corner * 2 + 1] = generatedNormalIndex;
                    }
                }
            }
            
            out.faces.push_back(face);
            out.texCoords.insert(out.texCoords.end(), uvs.begin(), uvs.end());
            if (sawuv) {
                out.hasTexCoords = true;
            }

        }
    }

    return !out.vertices.empty() && !out.faces.empty();
}
