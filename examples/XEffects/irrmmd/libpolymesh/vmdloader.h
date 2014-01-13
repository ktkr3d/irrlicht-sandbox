#ifndef MMD_VMD_LOADER_H
#define MMD_VMD_LOADER_H

#include "core.h"
#include <map>
#include <boost/shared_ptr.hpp>

namespace polymesh {

  namespace vmd {

    typedef rigid::Vector3 Vector3;
    typedef rigid::Quaternion Quaternion;

    ////////////////////////////////////////////////////////////
    // Bone
    ////////////////////////////////////////////////////////////
    struct Bone
    {
      std::string name;
      unsigned int frame;
      Vector3 pos;
      Quaternion q;
      char cInterpolationX[16];
      char cInterpolationY[16];
      char cInterpolationZ[16];
      char cInterpolationRot[16];

      template<class READER>
        void
        read(READER &reader)
        {
          name=reader.getString(15);
          frame=reader.get(TYPE<unsigned int>());
          pos=reader.get(TYPE<Vector3>());
          q=reader.get(TYPE<Quaternion>());
          reader.getBytes(cInterpolationX, 16);
          reader.getBytes(cInterpolationY, 16);
          reader.getBytes(cInterpolationZ, 16);
          reader.getBytes(cInterpolationRot, 16);
        }

      friend bool operator<(const Bone &rhs1, const Bone &rhs2){ return rhs1.frame<rhs2.frame; }
    };
    inline std::ostream& operator<<(std::ostream &os, const Bone &rhs)
    {
      os
        << "<Bone "
        << '"' << rhs.name << '"'
        << " " << rhs.frame << rhs.pos << rhs.q
        << ">"
        ;
      return os;
    }

    struct BoneBuffer
    {
      typedef boost::shared_ptr<BoneBuffer> Ptr;
      std::vector<Bone> bones;

      void push(const Bone &b){ bones.push_back(b); }
    };

    ////////////////////////////////////////////////////////////
    // Morph
    ////////////////////////////////////////////////////////////
    struct Morph
    {
      std::string name;
      unsigned int frame;
      float influence;

      template<class READER>
        void
        read(READER &reader)
        {
          name=reader.getString(15);
          frame=reader.get(TYPE<unsigned int>());
          influence=reader.get(TYPE<float>());
        }

      friend bool operator<(const Morph &rhs1, const Morph &rhs2){ return rhs1.frame<rhs2.frame; }
    };

    struct MorphBuffer
    {
      typedef boost::shared_ptr<MorphBuffer> Ptr;
      std::vector<Morph> morphs;

      void push(const Morph &m){ morphs.push_back(m); }
    };

    ////////////////////////////////////////////////////////////
    // Loader
    ////////////////////////////////////////////////////////////
    class Loader
    {
      public:
        std::string version;
        std::string name;
        typedef std::map<std::string, BoneBuffer::Ptr> BoneMap;
        BoneMap boneMap;
        typedef std::map<std::string, MorphBuffer::Ptr> MorphMap;
        MorphMap morphMap;

        bool parse(char *buf, unsigned int size);
    };

    inline std::ostream& operator<<(std::ostream &os, const Loader &rhs)
    {
      os
        << "<VMD " << rhs.name << std::endl
        << "[bones] " << rhs.boneMap.size() << std::endl
        << "[morphs] " << rhs.morphMap.size() << std::endl
        << ">"
        ;
      return os;
    }

  }

}

#endif // VMD_LOADER_H
