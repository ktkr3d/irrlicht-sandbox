#ifndef MMD_PMD_LOADER_H
#define MMD_PMD_LOADER_H

#include "core.h"
#include <iostream>
#include <boost/foreach.hpp>

namespace polymesh {

  namespace pmd {

    using namespace rigid;

    ////////////////////////////////////////////////////////////
    // Vertex
    ////////////////////////////////////////////////////////////
    struct Vertex
    {
      Vector3 pos;
      Vector3 normal;
      Vector2 uv;
      unsigned short bone0;
      unsigned short bone1;
      unsigned char weight0;
      unsigned char edge_flag;
    };
    inline std::ostream &operator<<(std::ostream &os, const Vertex &rhs)
    {
      os
        << "[Vertex"
        << " pos:" << rhs.pos
        << " normal:" << rhs.normal
        << " uv:" << rhs.uv
        << " bone0:" << rhs.bone0
        << " bone1:" << rhs.bone1
        << " weight0:" << (int)rhs.weight0
        << " edge_flag:" << (int)rhs.edge_flag
        << "]"
        ;
      return os;
    }

    ////////////////////////////////////////////////////////////
    // Material
    ////////////////////////////////////////////////////////////
    struct Material
    {
      Vector3 diffuse_color;
      float alpha;
      float specular;
      Vector3 specular_color;
      Vector3 mirror_color;
      unsigned char toon_index;
      unsigned char flag;
      unsigned int vertex_count;
      std::string texture;
    };
    inline std::ostream &operator<<(std::ostream &os,
        const Material &rhs)
    {
      os
        << "[Material"
        << " diffuse:" << rhs.diffuse_color
        << " toon_index:" << (int)rhs.toon_index
        << " flag:" << (int)rhs.flag
        << " vertex_count:" << rhs.vertex_count
        << " texture:" << rhs.texture
        << "]"
        ;
      return os;
    }

    ////////////////////////////////////////////////////////////
    // Bone
    ////////////////////////////////////////////////////////////
    enum BONE_TYPE
    {
      BONE_ROTATE=0,
      BONE_ROTATE_MOVE,
      BONE_IK,
      BONE_UNKNOWN,
      BONE_IK_INFLUENCED,
      BONE_ROTATE_INFLUENCED,
      BONE_IK_CONNECT,
      BONE_INVISIBLE,
      BONE_TWIST,
      BONE_REVOLVE,
    };
    struct Bone
    {
      std::string name;
      unsigned short parent_index;
      unsigned short tail_index;
      BONE_TYPE type;
      unsigned short ik_index;
      Vector3 pos;
    };
    inline std::ostream &operator<<(std::ostream &os,
        const Bone &rhs)
    {
      os
        << "[Bone "
        << '"' << rhs.name << '"'
        << "]"
        ;
      return os;
    }

    ////////////////////////////////////////////////////////////
    // IK
    ////////////////////////////////////////////////////////////
    struct IK
    {
      unsigned short index;
      unsigned short target;
      unsigned char length;
      unsigned short iterations;
      float weight;
      std::vector<unsigned short> children;
    };
    inline std::ostream &operator<<(std::ostream &os, const IK &rhs)
    {
      os
        << "[IK "
        << "]"
        ;
      return os;
    }

    ////////////////////////////////////////////////////////////
    // Morph
    ////////////////////////////////////////////////////////////
    enum MORPH_TYPE
    {
      MORPH_BASE=0,
      MORPH_MAYU,
      MORPH_ME,
      MORPH_LIP,
      MORPH_OTHER,
    };
    struct Morph
    {
      std::string name;
      unsigned int vertex_count;
      unsigned char type;
      std::vector<unsigned int> indices;
      std::vector<Vector3> pos_list;
    };
    inline std::ostream &operator<<(std::ostream &os, const Morph &rhs)
    {
      os
        << "[Morph "
        << '"' << rhs.name << '"'
        << "]"
        ;
      return os;
    }

    ////////////////////////////////////////////////////////////
    // rigid
    ////////////////////////////////////////////////////////////
    enum RIGID_TYPE
    {
      RIGID_SPHERE=0,
      RIGID_BOX,
      RIGID_CAPSULE,
    };
    enum PROCESS_TYPE
    {
      PROCESS_SYNCBONE=0,
      PROCESS_PHYSICS,
      PROCESS_PHYSICS_WITH_SYNCBONE,
    };

    struct RigidBody
    {
      std::string name;
      unsigned short boneIndex;
      unsigned char group;
      unsigned short target;
      RIGID_TYPE rigidType;
      float w;
      float h;
      float d;
      Vector3 position;
      Vector3 rotation;
      float weight;
      float posAttenuation;
      float rotAttenuation;
      float recoile;
      float friction;
      PROCESS_TYPE processType;
    };

    struct Joint
    {
      std::string name;
      unsigned int rigidA;
      unsigned int rigidB;
      Vector3 pos;
      Vector3 rot;
      Vector3 constraintPosA;
      Vector3 constraintPosB;
      Vector3 constraintRotA;
      Vector3 constraintRotB;
      Vector3 springPos;
      Vector3 springRot;
    };

    ////////////////////////////////////////////////////////////
    // Loader
    ////////////////////////////////////////////////////////////
    class Loader
    {
    public:

      float version;
      std::string name;
      std::string comment;
      std::vector<Vertex> vertices;
      std::vector<unsigned short> indices;
      std::vector<Material> materials;
      std::vector<Bone> bones;
      std::vector<IK> ik_list;
      std::vector<Morph> morph_list;
      std::vector<std::pair<unsigned short, unsigned char> > bone_list;
      std::vector<std::string> bone_name_list;
      std::vector<RigidBody> rigids;
      std::vector<Joint> joints;

    public:
      Loader()
        : version(0)
        {}

        bool parse(char *buf, unsigned int size);

    private:
        bool validate_();
    };

    inline std::ostream &operator<<(std::ostream &os, const Loader &rhs)
    {
      os
        << "<PMD " << rhs.name << std::endl
        << rhs.comment << std::endl
        << "[vertices] " << rhs.vertices.size() << std::endl
        << "[indices] " << rhs.indices.size() << std::endl
        << "[materials] " << rhs.materials.size() << std::endl
        ;
      BOOST_FOREACH(const Material &m, rhs.materials){
        os << m << std::endl;
      }

      os
        << "[bones] " << rhs.bones.size() << std::endl
        ;
      BOOST_FOREACH(const Bone &b, rhs.bones){
        os << b << std::endl;
      }

      os
        << "[ik] " << rhs.ik_list.size() << std::endl
        ;
      BOOST_FOREACH(const IK &ik, rhs.ik_list){
        os << ik << std::endl;
      }

      os
        << "[morph] " << rhs.morph_list.size() << std::endl
        ;
      BOOST_FOREACH(const Morph &morph, rhs.morph_list){
        os << morph << std::endl;
      }

      os
        << ">" << std::endl
        ;
      return os;
    }

  }

}

#endif // MMD_PMD_LOADER_H
