#ifndef X_LOADER_H
#define X_LOADER_H

#include "core.h"
#include <boost/shared_ptr.hpp>
#include <map>
#include <stack>

namespace polymesh {
  namespace x {

    typedef rigid::Vector2 Vector2;
    typedef rigid::Vector3 Vector3;
    typedef rigid::Vector4 Vector4;
    typedef rigid::Matrix4 Matrix4;

    struct Material
    {
      Vector4 rgba;
      float power;
      Vector3 specular;
      Vector3 emissive;
      std::string texture;
	  Material()
		  : power(0)
	  {}
    };
    typedef boost::shared_ptr<Material> MaterialPtr;

    struct Face
    {
      unsigned int num;
      unsigned int indices[4];
      unsigned int material_index;

      Face(unsigned int i0, unsigned int i1, unsigned int i2)
        : num(3), material_index(0)
        {
          indices[0]=i0;
          indices[1]=i1;
          indices[2]=i2;
          indices[3]=-1;
        }
      Face(unsigned int i0, unsigned int i1, unsigned int i2, unsigned int i3)
        : num(4), material_index(0)
        {
          indices[0]=i0;
          indices[1]=i1;
          indices[2]=i2;
          indices[3]=i3;
        }
    };
    inline std::ostream& operator<<(std::ostream &os, const Face &rhs)
    {
      switch(rhs.num)
      {
      case 3:
        os << "<Triangle>";
        break;
      case 4:
        os << "<Quadrangle>";
        break;
      default:
        os << "<Face[" << rhs.num << "]>";
        break;
      }
      return os;
    }

    struct Vertex
    {
      Vector3 pos;
      Vector2 uv;
      Vector3 normal;
      Vector4 vertex_color;
      // skinning
      unsigned short skinningIndex[4];
      float skinningWeight[4];
      char weightCount;

      Vertex(float x, float y, float z)
        : pos(x, y, z), weightCount(0)
        {
          skinningIndex[0]=0xFFFF;
          skinningIndex[1]=0xFFFF;
          skinningIndex[2]=0xFFFF;
          skinningIndex[3]=0xFFFF;
          skinningWeight[0]=0;
          skinningWeight[1]=0;
          skinningWeight[2]=0;
          skinningWeight[3]=0;
        }

      void assignWeight(unsigned int frame_index, float weight)
      {
        skinningIndex[static_cast<int>(weightCount)]=frame_index;
        skinningWeight[static_cast<int>(weightCount)]=weight;
        ++weightCount;
        assert(weightCount<=4);
      }
    };

    struct SkinWeight
    {
      std::vector<unsigned int> indices;
      std::vector<float> weights;
      Matrix4 offset;
    };
    typedef boost::shared_ptr<SkinWeight> SkinWeightPtr;

    struct Mesh
    {
      std::string name;
      std::vector<Vertex> vertices;
      std::vector<Face> faces;
      std::vector<MaterialPtr> materials;
      std::map<std::string, SkinWeightPtr> weight_map;
    };
    typedef boost::shared_ptr<Mesh> MeshPtr;
    inline std::ostream &operator<<(std::ostream &os, const Mesh&rhs)
    {
      os
        << "<Mesh "
        << '"' << rhs.name << '"'
        << ", " << rhs.vertices.size() << "vertices"
        << ", " << rhs.faces.size() << "faces"
        << ", " << rhs.materials.size() << "materials"
        << ">"
        ;
      return os;
    }

    struct Frame;
    typedef boost::shared_ptr<Frame> FramePtr;
    struct Frame
    {
      unsigned int id;
      unsigned short parent_index;
      std::string name;
      std::vector<FramePtr> children;
      MeshPtr mesh;

      Matrix4 offset;

      Frame(unsigned int _id)
        : id(_id), parent_index(0xFFFF)
        { }

      void addChild(FramePtr frame)
      {
        frame->parent_index=id;
        children.push_back(frame);
      }
    };
    inline std::ostream &operator<<(std::ostream &os, const Frame &rhs)
    {
      os
        << "<Frame "
        << '"' << rhs.name << '"'
        << ">"
        ;
      return os;
    }

    struct Animation
    {
      std::string frame_name;
      std::vector<unsigned int> frames;
      std::vector<Matrix4> matrices;
      void push(int frame, const Matrix4 &matrix)
      {
        frames.push_back(frame);
        matrices.push_back(matrix);
      }
    };
    typedef boost::shared_ptr<Animation> AnimationPtr;
    inline std::ostream& operator<<(std::ostream &os, const Animation  &rhs)
    {
      os
        << "<Animation "
        << '"' << rhs.frame_name << '"'
        << " " << rhs.frames.size() << "frames"
        ;
      return os;
    }

    struct AnimationSet
    {
      std::string name;
      typedef std::map<std::string, AnimationPtr> AnimationMap;
      AnimationMap animation_map;
    };
    typedef boost::shared_ptr<AnimationSet> AnimationSetPtr;
    inline std::ostream& operator<<(std::ostream &os, const AnimationSet &rhs)
    {
      os
        << "<AnimationSet "
        << '"' << rhs.name << '"'
        << " " << rhs.animation_map.size() << "bones"
        ;
      /*
      for(AnimationSet::AnimationMap::const_iterator it=rhs.animation_map.begin();
          it!=rhs.animation_map.end();
          ++it){
        os << *it->second << std::endl;
      }
      */
      os
        << ">"
        ;
      return os;
    }

    class Loader
    {
      std::stack<FramePtr> currentFrame;
      std::map<std::string, MaterialPtr> material_map;
      std::map<std::string, FramePtr> frame_map;
      FramePtr meshFrame;

    public:
      int gnu_dev_major;
      int gnu_dev_minor;
      int major;
      int minor;
      std::string type;
      int float_size;

      int max_skin_index;
      float min_skin_weight;

      bool hasVertexColor;
      FramePtr rootFrame;
      std::vector<MeshPtr> meshes;
      std::vector<AnimationSetPtr> animations;
      std::vector<FramePtr> frames;

      Loader()
        : major(0), minor(0), hasVertexColor(false)
        {
          // dummy root
          rootFrame=FramePtr(new Frame(0));
          currentFrame.push(rootFrame);
        }

      bool parse(char *buf, unsigned int size);

    private:
      void postProcess(FramePtr frame);
    };
    inline std::ostream &operator<<(std::ostream &os, const Loader &rhs)
    {
      os << "<X "
        << rhs.major << '.' << rhs.minor << ' ' << rhs.type << std::endl;

      os << "[meshes] " << rhs.meshes.size() << std::endl;
      for(size_t i=0; i<rhs.meshes.size(); ++i){
        os << *rhs.meshes[i] << std::endl;
      }
      os << "[frames] " << rhs.frames.size() << std::endl;
      /*
         for(size_t i=0; i<rhs.frames.size(); ++i){
         os << *rhs.frames[i] << std::endl;
         }
         */
      os << "[animations] " << rhs.animations.size() << std::endl;
      for(size_t i=0; i<rhs.animations.size(); ++i){
        os << *rhs.animations[i] << std::endl;
      }

      os << ">";
      return os;
    }
  }
}

#endif // X_LOADER_H
