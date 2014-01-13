#include "pmdloader.h"

namespace polymesh {
  namespace pmd {

    // 38bytes
    template<class READER>
      void
      read(READER &reader, Vertex &v)
      {
        v.pos.x=reader.get(TYPE<float>());
        v.pos.y=reader.get(TYPE<float>());
        v.pos.z=reader.get(TYPE<float>());
        v.normal.x=reader.get(TYPE<float>());
        v.normal.y=reader.get(TYPE<float>());
        v.normal.z=reader.get(TYPE<float>());
        v.uv.x=reader.get(TYPE<float>());
        v.uv.y=reader.get(TYPE<float>());
        v.bone0=reader.get(TYPE<unsigned short>());
        v.bone1=reader.get(TYPE<unsigned short>());
        v.weight0=reader.get(TYPE<unsigned char>());
        v.edge_flag=reader.get(TYPE<unsigned char>());
      }


    // 70bytes
    template<class READER>
      void
      read(READER &reader, Material &m)
      {
        m.diffuse_color.x=reader.get(TYPE<float>());
        m.diffuse_color.y=reader.get(TYPE<float>());
        m.diffuse_color.z=reader.get(TYPE<float>());
        m.alpha=reader.get(TYPE<float>());
        m.specular=reader.get(TYPE<float>());
        m.specular_color.x=reader.get(TYPE<float>());
        m.specular_color.y=reader.get(TYPE<float>());
        m.specular_color.z=reader.get(TYPE<float>());
        m.mirror_color.x=reader.get(TYPE<float>());
        m.mirror_color.y=reader.get(TYPE<float>());
        m.mirror_color.z=reader.get(TYPE<float>());
        m.toon_index=reader.get(TYPE<unsigned char>());
        m.flag=reader.get(TYPE<unsigned char>());
        m.vertex_count=reader.get(TYPE<unsigned int>());
        m.texture=reader.getString(20);
      }

    // 39bytes
    template<class READER>
      void
      read(READER &reader, Bone &b)
      {
        b.name=reader.getString(20);
        b.parent_index=reader.get(TYPE<unsigned short>());
        b.tail_index=reader.get(TYPE<unsigned short>());
        b.type=static_cast<BONE_TYPE>(reader.get(TYPE<unsigned char>()));
        b.ik_index=reader.get(TYPE<unsigned short>());
        b.pos.x=reader.get(TYPE<float>());
        b.pos.y=reader.get(TYPE<float>());
        b.pos.z=reader.get(TYPE<float>());
      }

    template<class READER>
      void
      read(READER &reader, IK &ik)
      {
        // 11bytes
        ik.index=reader.get(TYPE<unsigned short>());
        ik.target=reader.get(TYPE<unsigned short>());
        ik.length=reader.get(TYPE<unsigned char>());
        ik.iterations=reader.get(TYPE<unsigned short>());
        ik.weight=reader.get(TYPE<float>());
        // 2 x length bytes
        for(unsigned short j=0; j<ik.length; ++j){
          ik.children.push_back(reader.get(TYPE<unsigned short>()));
        }
      }

    template<class READER>
      void
      read(READER &reader, Morph &m)
      {
        // 25bytes
        m.name=reader.getString(20);
        m.vertex_count=reader.get(TYPE<unsigned int>());
        m.type=static_cast<MORPH_TYPE>(reader.get(TYPE<unsigned char>()));
        // 12 x vertex_count bytes
        for(unsigned short i=0; i<m.vertex_count; ++i){
          m.indices.push_back(reader.get(TYPE<unsigned int>()));
          m.pos_list.push_back(Vector3());
          m.pos_list.back().x=reader.get(TYPE<float>());
          m.pos_list.back().y=reader.get(TYPE<float>());
          m.pos_list.back().z=reader.get(TYPE<float>());
        }
      }

    template<class READER>
      void
      read(READER &reader, RigidBody &r)
      {
        unsigned int pos=reader.pos();
        r.name=reader.getString(20);
        r.boneIndex=reader.get(TYPE<unsigned short>());
        r.group=reader.get(TYPE<unsigned char>());
        r.target=reader.get(TYPE<unsigned short>());
        r.rigidType=static_cast<RIGID_TYPE>(reader.get(TYPE<unsigned char>()));
        r.w=reader.get(TYPE<float>());
        r.h=reader.get(TYPE<float>());
        r.d=reader.get(TYPE<float>());
        r.position=reader.get(TYPE<Vector3>());
        r.rotation=reader.get(TYPE<Vector3>());
        r.weight=reader.get(TYPE<float>());
        r.posAttenuation=reader.get(TYPE<float>());
        r.rotAttenuation=reader.get(TYPE<float>());
        r.recoile=reader.get(TYPE<float>());
        r.friction=reader.get(TYPE<float>());
        r.processType=static_cast<PROCESS_TYPE>(
            reader.get(TYPE<unsigned char>()));
        assert(reader.pos()-pos==83);
      }

    template<class READER>
      void
      read(READER &reader, Joint &j)
      {
        unsigned int base_pos=reader.pos();
        j.name=reader.getString(20);
        j.rigidA=reader.get(TYPE<unsigned int>());
        j.rigidB=reader.get(TYPE<unsigned int>());
        j.pos=reader.get(TYPE<Vector3>());
        j.rot=reader.get(TYPE<Vector3>());
        j.constraintPosA=reader.get(TYPE<Vector3>());
        j.constraintPosB=reader.get(TYPE<Vector3>());
        j.constraintRotA=reader.get(TYPE<Vector3>());
        j.constraintRotB=reader.get(TYPE<Vector3>());
        j.springPos=reader.get(TYPE<Vector3>());
        j.springRot=reader.get(TYPE<Vector3>());
        assert(reader.pos()-base_pos==124);
      }

    class Impl
    {
      Loader &l;
    public:
      Impl(Loader &loader)
        : l(loader)
        {}

      template<typename READ>
        bool parse(READ read)
        {
          BinaryReader<READ> reader(read);
          if(!parseHeader(reader)){
            return false;
          }
          if(!parseVertices(reader)){
            return false;
          }
          if(!parseIndices(reader)){
            return false;
          }
          if(!parseMaterials(reader)){
            return false;
          }
          if(!parseBones(reader)){
            return false;
          }
          if(!parseIK(reader)){
            return false;
          }
          if(!parseMorph(reader)){
            return false;
          }
          if(!parseFaceList(reader)){
            return false;
          }
          if(!parseBoneNameList(reader)){
            return false;
          }
          if(!parseBoneList(reader)){
            return false;
          }
          if(reader.eof()){
            return true;
          }

          ////////////////////////////////////////////////////////////
          // extended data
          ////////////////////////////////////////////////////////////
#ifdef DEBUG
          std::cout << "has extended data" << std::endl;
#endif
          // english
          ////////////////////////////////////////////////////////////
          if(reader.get(TYPE<unsigned char>())){
            if(!parseEnglishName(reader)){
              return false;
            }
            if(!parseEnglishBone(reader)){
              return false;
            }
            if(!parseEnglishMorph(reader)){
              return false;
            }
            if(!parseEnglishBoneList(reader)){
              return false;
            }
          }
          if(reader.eof()){
            return true;
          }

          // toone texture
          ////////////////////////////////////////////////////////////
          if(!parseToonTextures(reader)){
            return false;
          }
          if(reader.eof()){
            return true;
          }

          // physics
          ////////////////////////////////////////////////////////////
          if(!parseRigid(reader)){
            return false;
          }
          if(!parseJoint(reader)){
            return false;
          }

          // end
          assert(reader.eof());

          return true;
        }

    private:
      template<typename READER>
        bool parseJoint(READER &reader)
        {
          unsigned int count=reader.get(TYPE<unsigned int>());
#ifdef DEBUG
          std::cout << "constraint: " << count << std::endl;
#endif
          for(unsigned int i=0; i<count; ++i){
            //reader.getBytes(NULL, 124);
            l.joints.push_back(Joint());
            read(reader, l.joints.back());
          }
          return true;
        }

      template<typename READER>
        bool parseRigid(READER &reader)
        {
          unsigned int count=reader.get(TYPE<unsigned int>());
#ifdef DEBUG
          std::cout << "rigidBody: " << count << std::endl;
#endif
          for(unsigned int i=0; i<count; ++i){
            //reader.getBytes(NULL, 83);
            l.rigids.push_back(RigidBody());
            read(reader, l.rigids.back());
          }
          return true;
        }

      template<typename READER>
        bool parseToonTextures(READER &reader)
        {
          for(size_t i=0; i<10; ++i){
            reader.getString(100);
          }
          return true;
        }

      template<typename READER>
        bool parseEnglishBoneList(READER &reader)
        {
          for(size_t i=0; i<l.bone_name_list.size(); ++i){
            std::string english=reader.getString(50);
#ifdef DEBUG
            std::cout << english << std::endl;
#endif
          }
          return true;
        }

      template<typename READER>
        bool parseEnglishMorph(READER &reader)
        {
          int count=l.morph_list.size()-1;
          for(int i=0; i<count; ++i){
            std::string english_morph_name=reader.getString(20);
          }
          return true;
        }

      template<typename READER>
        bool parseEnglishBone(READER &reader)
        {
          for(size_t i=0; i<l.bones.size(); ++i){
            std::string english_bone_name=reader.getString(20);
          }
          return true;
        }

      template<typename READER>
        bool parseEnglishName(READER &reader)
        {
          std::string english_mdoel_name=reader.getString(20);
          std::string english_comment=reader.getString(256);
          return true;
        }

      template<typename READER>
        bool parseBoneList(READER &reader)
        {
          unsigned int count=reader.get(TYPE<unsigned int>());
          for(unsigned int i=0; i<count; ++i){
            unsigned short bone=reader.get(TYPE<unsigned short>());
            unsigned char disp=reader.get(TYPE<unsigned char>());
            l.bone_list.push_back(std::make_pair(bone, disp));
          }
          return true;
        }

      template<typename READER>
        bool parseBoneNameList(READER &reader)
        {
          unsigned int count=reader.get(TYPE<unsigned char>());
          for(unsigned int i=0; i<count; ++i){
            l.bone_name_list.push_back(reader.getString(50));
          }
          return true;
        }

      template<typename READER>
        bool parseFaceList(READER &reader)
        {
          unsigned int count=reader.get(TYPE<unsigned char>());
          for(unsigned int i=0; i<count; ++i){
            reader.get(TYPE<unsigned short>());
          }
          return true;
        }

      template<typename READER>
        bool parseMorph(READER &reader)
        {
          unsigned int count=reader.get(TYPE<unsigned short>());
          for(unsigned int i=0; i<count; ++i){
            l.morph_list.push_back(Morph());
            read(reader, l.morph_list.back());
          }
          return true;
        }

      template<typename READER>
        bool parseIK(READER &reader)
        {
          unsigned int count=reader.get(TYPE<unsigned short>());
          for(unsigned int i=0; i<count; ++i){
            l.ik_list.push_back(IK());
            read(reader, l.ik_list.back());
          }
          return true;
        }

      template<typename READER>
        bool parseBones(READER &reader)
        {
          unsigned int count=reader.get(TYPE<unsigned short>());
          for(unsigned int i=0; i<count; ++i){
            l.bones.push_back(Bone());
            read(reader, l.bones.back());
          }
          return true;
        }

      template<typename READER>
        bool parseMaterials(READER &reader)
        {
          unsigned int count=reader.get(TYPE<unsigned int>());
          for(unsigned int i=0; i<count; ++i){
            l.materials.push_back(Material());
            read(reader, l.materials.back());
          }
          return true;
        }

      template<typename READER>
        bool parseIndices(READER &reader)
        {
          unsigned int count=reader.get(TYPE<unsigned int>());
          for(unsigned int i=0; i<count; ++i){
            l.indices.push_back(reader.get(TYPE<unsigned short>()));
          }
          return true;
        }

      template<typename READER>
        bool parseVertices(READER &reader)
        {
          unsigned int count=reader.get(TYPE<unsigned int>());
          for(unsigned int i=0; i<count; ++i){
            l.vertices.push_back(Vertex());
            read(reader, l.vertices.back());
          }
          return true;
        }

      template<typename READER>
        bool parseHeader(READER &reader)
        {
          if(reader.getString(3)!="Pmd"){
            //std::cout << "invalid pmd" << std::endl;
            return false;
          }
          l.version=reader.get(TYPE<float>());
          if(l.version!=1.0){
            std::cout << "invalid vesion: " << l.version <<std::endl;
            return false;
          }
          l.name=reader.getString(20);
          l.comment=reader.getString(256);

          return true;
        }

    };

    bool 
      Loader::parse(char *buf, unsigned int size)
      {
        Impl impl(*this);
        BufferRead reader(buf, size);
        if(!impl.parse(reader)){
          return false;
        }
        if(!validate_()){
          return false;
        }
        return true;
      }

    bool 
      Loader::validate_()
      {
        if(!morph_list.empty()){
          // validate morph
          assert(morph_list[0].type==MORPH_BASE);
          // check base
          Morph &base=morph_list[0];
          for(size_t i=0; i<base.vertex_count; ++i){
            assert(vertices[base.indices[i]].pos==base.pos_list[i]);
          }
          // check each face
          for(size_t i=1; i<morph_list.size(); ++i){
            Morph &m=morph_list[i];
            assert(m.type!=MORPH_BASE);
          }
        }
        return true;
      }

  } // namespace
} // namespace
