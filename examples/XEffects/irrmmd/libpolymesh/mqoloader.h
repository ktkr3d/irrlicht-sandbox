#ifndef MQO_LOADER_H
#define MQO_LOADER_H

#include "core.h"
#include <algorithm>

namespace polymesh {
  namespace mqo {

    typedef rigid::Vector2 Vector2;
    typedef rigid::Vector3 Vector3;
    typedef rigid::Vector4 Vector4;

    ////////////////////////////////////////////////////////////
    // rgba(flaot)<->rgba(unsigned char)
    ////////////////////////////////////////////////////////////
    struct RGBA
    {
      unsigned char r;
      unsigned char g;
      unsigned char b;
      unsigned char a;
      RGBA()
        :r(255), g(255), b(255), a(255)
        {}
      RGBA(unsigned char _r, 
          unsigned char _g, 
          unsigned char _b, 
          unsigned char _a)
        :r(_r), g(_g), b(_b), a(_a)
        {}
      RGBA(const Vector4 &v)
        :
        r(static_cast<int>(255.0f*v.x)), 
        g(static_cast<int>(255.0f*v.y)), 
        b(static_cast<int>(255.0f*v.z)), 
        a(static_cast<int>(255.0f*v.w))
        {}
      RGBA(unsigned int dword)
      {
        //unsigned char* rgba=(unsigned char*)&dword;
        a=dword & 0x000000FF;
        b=dword & 0x0000FF00 >> 8;
        g=dword & 0x00FF0000 >> 16;
        r=dword & 0xFF000000 >> 24;
      }

      Vector4 toVector4()
      {
        float factor=1.0f/255.0f;
        return Vector4( r*factor, g*factor, b*factor, a*factor);
      }
    };
    inline std::ostream &operator<<(std::ostream &os, const RGBA &rhs)
    {
      return os
        << '[' 
        << (int)rhs.r 
        << ',' << (int)rhs.g 
        << ',' << (int)rhs.b 
        << ',' << (int)rhs.a 
        << ']';
    }

    ////////////////////////////////////////////////////////////
    // 読み取りデータの入れ物
    ////////////////////////////////////////////////////////////
    struct Scene
    {
      Vector3 pos;
      Vector3 lookat;
      float head;
      float pitch;
      int ortho;
      float zoom2;
      Vector3 ambient;
      Scene()
        : head(0), pitch(0), ortho(false), zoom2(2)
        {}
    };
    inline std::ostream &operator<<(std::ostream &os, const Scene &rhs)
    {
      os
        << "<Scene"
        << " pos: " << rhs.pos
        << ", lookat: " << rhs.lookat
        << ", head: " << rhs.head
        << ", pitch: " << rhs.pitch
        << ", ortho: " << rhs.ortho
        << ", zoom2: " << rhs.zoom2
        << ", ambient: " << rhs.ambient
        << ">"
        ;
      return os;
    }

    struct Material
    {
      std::string name;
      int shader;
      RGBA color;
      float diffuse;
      float ambient;
      float emmit;
      float specular;
      float power;
      std::string texture;
      std::string alphamap;
      std::string bumpmap;
      int vcol;

      Material()
        : shader(0), diffuse(1), ambient(0), emmit(0), specular(0), power(0),
        vcol(0)
        {}
    };
    inline std::ostream &operator<<(std::ostream &os, const Material &rhs)
    {
      os
        << "<Material "
        << '"' << rhs.name << '"'
        << " shader:" << rhs.shader
        << ", color:" << rhs.color
        << ", diffuse:" << rhs.diffuse
        << ", ambient:" << rhs.ambient
        << ", emmit:" << rhs.emmit
        << ", specular:" << rhs.specular
        << ", power:" << rhs.power
        << ", texture:\"" << rhs.texture << '"'
        << ", alphamap:\"" << rhs.alphamap << '"'
        << ", bumpmap:\"" << rhs.bumpmap << '"'
        << ">"
        ;
      return os;
    }

    struct Face
    {
      unsigned int index_count;
      unsigned int indices[4];
      unsigned int material_index;
      Vector2 uv[4];
      RGBA color[4];
      Face()
        : index_count(0), material_index(0)
        {
          indices[0]=0;
          indices[1]=0;
          indices[2]=0;
          indices[3]=0;
          uv[0]=Vector2();
          uv[1]=Vector2();
          uv[2]=Vector2();
          uv[3]=Vector2();
        }
    };
    inline std::ostream &operator<<(std::ostream &os, const Face &rhs)
    {
      switch(rhs.index_count)
      {
      case 2:
        os
          << "<Edge "
          << "indices:{" << rhs.indices[0] << ',' << rhs.indices[1] << "}"
          << ", material_index: " << rhs.material_index
          << ", uv:{" << rhs.uv[0] << ',' << rhs.uv[1] << "}"
          << ", color:{" << rhs.color[0] << ',' << rhs.color[1] << "}"
          << ">"
          ;
        break;

      case 3:
        os
          << "<Triangle "
          << "indices:{" << rhs.indices[0] 
          << ',' << rhs.indices[1] 
          << ',' << rhs.indices[2] << "}"
          << ", material_index: " << rhs.material_index
          << ", uv:{" << rhs.uv[0] << ',' << rhs.uv[1] << ',' << rhs.uv[2] << "}"
          << ", color:{" 
          << rhs.color[0] << ',' << rhs.color[1] << ',' <<  rhs.color[2] << "}"
          << ">"
          ;
        break;

      case 4:
        os
          << "<Rectangle "
          << "indices:{" << rhs.indices[0] 
          << ',' << rhs.indices[1] 
          << ',' << rhs.indices[2] 
          << ',' << rhs.indices[3] << "}"
          << ", material_index: " << rhs.material_index
          << ", uv:{" << rhs.uv[0] << ',' << rhs.uv[1] 
          << ',' <<  rhs.uv[2] << ',' << rhs.uv[3] << "}"
          << ", color:{" << rhs.color[0] 
          << ',' << rhs.color[1] 
          << ',' <<  rhs.color[2] 
          << ',' <<  rhs.color[3] << "}"
          << ">"
          ;
        break;
      default:
        assert(false);
      }

      return os;
    }

    struct Object
    {
      std::string name;
      int depth;
      int folding;
      Vector3 scale;
      Vector3 rotation;
      Vector3 translation;
      int visible;
      int locking;
      int shading;
      float smoothing;
      Vector3 color;
      int color_type;

      std::vector<Vector3> vertices;
      std::vector<Face> faces;
    };
    inline std::ostream &operator<<(std::ostream &os, const Object &rhs)
    {
      os
        << "<Object "
        << '"' << rhs.name << '"'
        << " vertices:" << rhs.vertices.size()
        << ", faces:" << rhs.faces.size()
        << ">"
        ;
      return os;
    }

    ////////////////////////////////////////////////////////////
    // MQOパーサ
    ////////////////////////////////////////////////////////////

    // Tokenizer
    struct DELIMITER
    {
      bool operator()(char c)
      {
        switch(c)
        {
        case ' ':
        case '(':
        case ')':
          return true;
        default:
          return false;
        }
      }
    };
    typedef LineSplitter<DELIMITER> SPLITTER;

    class Loader
    {
    public:
      Scene scene;
      std::vector<Material> materials;
      std::vector<Object> objects;

      Loader() {}
      bool parse(char *buf, unsigned int size);
    };
    inline std::ostream &operator<<(std::ostream &os, const Loader &rhs)
    {
      os << "<MQO " << std::endl;
      os << rhs.scene << std::endl;
      os << "[Materials] " << rhs.materials.size() << std::endl;
      for(size_t i=0; i<rhs.materials.size(); ++i){
        os << rhs.materials[i] << std::endl;
      }
      os << "[Objects] " << rhs.objects.size() << std::endl;
      for(size_t i=0; i<rhs.objects.size(); ++i){
        os << rhs.objects[i] << std::endl;
      }
      os << ">";
      return os;
    }

  } // namespace
} // namespace

#endif // MQO_LOADER_H
