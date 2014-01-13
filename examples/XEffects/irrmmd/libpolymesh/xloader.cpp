#include "xloader.h"
#include <vector>
#include <iostream>
#include <boost/foreach.hpp>

namespace polymesh {
  namespace x {

    // tokenizer
    struct DELIMITER
    {
      bool operator()(char c)
      {
        switch(c)
        {
        case ' ':
        case ',': // fall through
        case '\r': // fall through
        case '\n': // fall through
          return true;
        default:
          return false;
        }
      }
    };
    typedef LineSplitter<DELIMITER> SPLITTER;

    class Implementation
    {
      std::stack<FramePtr> &currentFrame;
      std::map<std::string, MaterialPtr> &material_map;
      std::map<std::string, FramePtr> &frame_map;
      FramePtr &meshFrame;

      bool &hasVertexColor;
      FramePtr &rootFrame;
      std::vector<MeshPtr> &meshes;
      std::vector<AnimationSetPtr> &animations;
      std::vector<FramePtr> &frames;

    public:
      Implementation(
          std::stack<FramePtr> &_currentFrame,
          std::map<std::string, MaterialPtr> &_material_map,
          std::map<std::string, FramePtr> &_frame_map,
          FramePtr &_meshFrame,

          bool &_hasVertexColor, FramePtr &_rootFrame,
          std::vector<MeshPtr> &_meshes,
          std::vector<AnimationSetPtr> &_animations,
          std::vector<FramePtr> &_frames
          )
        : currentFrame(_currentFrame), material_map(_material_map),
        frame_map(_frame_map),  meshFrame(_meshFrame),
        hasVertexColor(_hasVertexColor), rootFrame(_rootFrame),
        meshes(_meshes), animations(_animations), frames(_frames)
        {}

      template<class READER>
        bool parse(READER reader)
        {
          while(!reader.eof()){
            cstr line=reader.getLine();
            if(line==""){
              continue;
            }
            SPLITTER splitter(line);
            // comment? in MilkShape3D
			splitter.skip("//", "\n");
            cstr key=splitter.get();
            if(key=="template"){
              if(!readTemplate(reader, splitter.get())){
                return false;
              }
            }
            else if(key=="Frame"){
              if(!readFrame(reader, splitter.get())){
                return false;
              }
            }
            else if(key=="AnimationSet"){
              if(!readAnimationSet(reader, splitter.get())){
                return false;
              }
            }
            else if(key=="Header"){
              if(!readChunk(reader)){
                return false;
              }
            }
            else if(key=="Mesh"){
              if(!readMesh(reader, splitter.get())){
                return false;
              }
            }
            else if(key=="Material"){
              if(!readMaterial(reader, splitter.get())){
                return false;
              }
            }
            else{
              std::cout << "unknown key(global): " << key << std::endl;
              readChunk(reader);
            }
          }
          return true;
        }  private:
      template<class READER>
        bool readAnimationSet(READER &reader, const cstr &name)
        {
          AnimationSetPtr animationSet(new AnimationSet);
          animations.push_back(animationSet);
          if(name!="{"){
            animationSet->name=name.str();
          }
          //std::cout << "AnimationSet: " << std::endl;
          while(!reader.eof()){
            cstr line=reader.getLine();
            if(line==""){
              continue;
            }
            if(line=="}"){
              return true;
            }

            SPLITTER splitter(line);
            cstr key=splitter.get();
            if(key=="Animation"){
              if(!readAnimation(reader, animationSet)){
                return false;
              }
            }
            else{
              std::cout << "unknown key(AnimationSet): " << line << std::endl;
              if(!readChunk(reader)){
                return false;
              }
            }
          }
          std::cout << "invalid eof(readAnimationSet)" << std::endl;
          return false;
        }

      template<class READER>
        bool readAnimation(READER &reader, AnimationSetPtr animationSet)
        {
          AnimationPtr animation(new Animation);

          //std::cout << "Animation: " << std::endl;
          while(!reader.eof()){
            cstr line=reader.getLine();
            if(line==""){
              continue;
            }
            if(line=="}"){
              if(!animation->frame_name.empty()){
                animationSet->animation_map[animation->frame_name]=animation;
              }
              return true;
            }

            SPLITTER splitter(line);
            cstr key=splitter.get();
            if(key=="AnimationKey"){
              if(!readAnimationKey(reader, animation)){
                return false;
              }
            }
            else if(key=="AnimationOptions"){
              if(!readChunk(reader)){
                return false;
              }
            }
            else if(key=="{"){
              cstr line=reader.getLine();
              SPLITTER splitter(line);
              cstr name=splitter.get();
              for(const char *p=name.begin(); p!=name.end(); ++p){
                if(*p=='}'){
                  name=cstr(name.begin(), p);
                  break;
                }
              }
              animation->frame_name=name.str();
            }
            else{
              std::cout << "unknown key(Animation): " << key << std::endl;
              return false;
            }
          }
          std::cout << "invalid eof(readAnimation)" << std::endl;
          return false;
        }

      template<class READER>
        bool readAnimationKey(READER &reader, AnimationPtr animation)
        {
          int type=atoi(reader.getLine().begin());

          switch(type)
          {
          case 0:
            {
              int key_count=atoi(reader.getLine().begin());
              for(int i=0; i<key_count; ++i){
                int frame=atoi(reader.getLine().begin());
                int channels=atoi(reader.getLine().begin());
                cstr line=reader.getTill('\n');
              }
            }
            break;

          case 2:
            {
              int key_count=atoi(reader.getLine().begin());
              for(int i=0; i<key_count; ++i){
                int frame=atoi(reader.getLine().begin());
                int channels=atoi(reader.getLine().begin());
                cstr line=reader.getTill('\n');
              }
            }
            break;

          case 4: // matrix
            {
              int key_count=atoi(reader.getLine().begin());
              for(int i=0; i<key_count; ++i){
                int frame=atoi(reader.getLine().begin());
                int channels=atoi(reader.getLine().begin());
                cstr line=reader.getTill('\n');
                SPLITTER splitter(line);
                float m00=splitter.getFloat();
                float m01=splitter.getFloat();
                float m02=splitter.getFloat();
                float m03=splitter.getFloat();
                float m10=splitter.getFloat();
                float m11=splitter.getFloat();
                float m12=splitter.getFloat();
                float m13=splitter.getFloat();
                float m20=splitter.getFloat();
                float m21=splitter.getFloat();
                float m22=splitter.getFloat();
                float m23=splitter.getFloat();
                float m30=splitter.getFloat();
                float m31=splitter.getFloat();
                float m32=splitter.getFloat();
                float m33=splitter.getFloat();
                animation->push(frame, Matrix4(
                      m00, m01, m02, m03,
                      m10, m11, m12, m13,
                      m20, m21, m22, m23,
                      m30, m31, m32, m33
                      ));
              }
            }
            break;

          default:
            std::cout << "unknown animation type: " << type << std::endl;
            return false;
          }

          cstr line=reader.getLine();
          if(line!="}"){
            std::cout 
              << "invalid close(readAnimationKey)" 
              << line
              << std::endl;
            return false;
          }

          return true;
        }

      template<class READER>
        bool readMesh(READER &reader, const cstr &name)
        {
          MeshPtr mesh(new Mesh);
          if(name!="{"){
            mesh->name=name.str();
          }
          meshes.push_back(mesh);
          currentFrame.top()->mesh=mesh;

          ////////////////////
          // vertices
          ////////////////////
          {
            cstr line=reader.getLine();
            // skip empty line
            while(!reader.eof()){
              if(line!=""){
                break;
              }
              line=reader.getLine();
            }

            SPLITTER splitter(line);
            int count=splitter.getInt();
            for(int i=0; i<count; ++i){
              // x
              double x=atof(reader.getLine().begin());
              // y
              double y=atof(reader.getLine().begin());
              // z
              double z=atof(reader.getLine().begin());
              mesh->vertices.push_back(Vertex(
				  static_cast<float>(x), 
				  static_cast<float>(y), 
				  static_cast<float>(z)));
            }
          }

          ////////////////////
          // faces
          ////////////////////
          {
            cstr line=reader.getLine();
            // skip empty line
            while(!reader.eof()){
              if(line!=""){
                break;
              }
              line=reader.getLine();
            }

            SPLITTER splitter(line);
            int count=splitter.getInt();
            for(int i=0; i<count; ++i){
              int face_vertex_count=atoi(reader.getLine().begin());
              cstr line=reader.getLine();
              SPLITTER splitter(line);
              switch(face_vertex_count)
              {
              case 3:
                {
                  int i0=splitter.getInt();
                  int i1=splitter.getInt();
                  int i2=splitter.getInt();
                  mesh->faces.push_back(Face(i0, i1, i2));
                }
                break;
              case 4:
                {
                  int i0=splitter.getInt();
                  int i1=splitter.getInt();
                  int i2=splitter.getInt();
                  int i3=splitter.getInt();
                  mesh->faces.push_back(Face(i0, i1, i2, i3));
                }
                break;
              default:
                assert(false);
                break;
              };
            }
          }

          ////////////////////
          // chunks
          ////////////////////
          while(!reader.eof()){
            cstr line=reader.getLine();
            if(line==""){
              continue;
            }
            if(line=="}"){
              return true;
            }

            SPLITTER splitter(line);
            cstr key=splitter.get();
            if(key=="MeshNormals"){
              if(!readChunk(reader)){
                return false;
              }
            }
            else if(key=="MeshMaterialList"){
              if(!readMeshMaterialList(reader, mesh)){
                return false;
              }
            }
            else if(key=="MeshTextureCoords"){
              if(!readMeshTextureCoords(reader, mesh)){
                return false;
              }
            }
            else if(key=="MeshVertexColors"){
              if(!readMeshVertexColors(reader, mesh)){
                return false;
              }
            }
            else if(key=="VertexDuplicationIndices"){
              if(!readChunk(reader)){
                return false;
              }
            }
            else if(key=="XSkinMeshHeader"){
              if(!readChunk(reader)){
                return false;
              }
            }
            else if(key=="XSkinMeshHeader"){
              if(!readChunk(reader)){
                return false;
              }
            }
            else if(key=="SkinWeights"){
              if(!readSkinWeights(reader, mesh)){
                return false;
              }
            }
            else{
              std::cout 
                << '(' << reader.getLineCount() << ')'
                << "unknown key(Mesh): " << key << std::endl;
              if(!readChunk(reader)){
                return false;
              }
            }
          }
          std::cout << "invalid eof(readMesh)" << std::endl;
          return false;
        }

      template<class READER>
        bool readSkinWeights(READER &reader, MeshPtr mesh)
        {
          // weight map
          std::string frame_name;
          {
            cstr line=reader.getLine();        
            SPLITTER splitter(line);
            frame_name=splitter.getQuated().str();
          }
          int count=atoi(reader.getLine().begin());

          SkinWeightPtr skinWeight(new SkinWeight);
          // vertex indices
          skinWeight->indices.resize(count);
          for(int i=0; i<count; ++i){
            skinWeight->indices[i]=atoi(reader.getTill(',', ';').begin());
          }
          // skinning weight
          skinWeight->weights.resize(count);
          for(int i=0; i<count; ++i){
            skinWeight->weights[i]=static_cast<float>(atof(reader.getTill(',', ';').begin()));
          }

          {
            // offset matrix
            cstr line=reader.getLine();        
            SPLITTER splitter(line);
            float m00=splitter.getFloat();
            float m01=splitter.getFloat();
            float m02=splitter.getFloat();
            float m03=splitter.getFloat();
            float m10=splitter.getFloat();
            float m11=splitter.getFloat();
            float m12=splitter.getFloat();
            float m13=splitter.getFloat();
            float m20=splitter.getFloat();
            float m21=splitter.getFloat();
            float m22=splitter.getFloat();
            float m23=splitter.getFloat();
            float m30=splitter.getFloat();
            float m31=splitter.getFloat();
            float m32=splitter.getFloat();
            float m33=splitter.getFloat();
            skinWeight->offset=Matrix4(
                m00, m01, m02, m03,
                m10, m11, m12, m13,
                m20, m21, m22, m23,
                m30, m31, m32, m33
                );
          }

          mesh->weight_map[frame_name]=skinWeight;


          return reader.getLine()=="}";
        }

      template<class READER>
        bool readMeshVertexColors(READER &reader, MeshPtr mesh)
        {
          cstr line=reader.getLine();
          SPLITTER splitter(line);
          size_t vertex_count=splitter.getInt();
          assert(mesh->vertices.size()==vertex_count);
          for(size_t i=0; i<vertex_count; ++i){
            cstr line=reader.getLine();
            splitter=SPLITTER(line);
            float r=splitter.getFloat();
            float g=splitter.getFloat();
            float b=splitter.getFloat();
            float a=splitter.getFloat();
            mesh->vertices[i].vertex_color=Vector4(r, g, b, a);
          }
          if(reader.getLine()!="}"){
            std::cout << "invalid close(readMeshVertexColors)" << std::endl;
            return false;
          }
          return true;
        }

      template<class READER>
        bool readMeshTextureCoords(READER &reader, MeshPtr mesh)
        {
          size_t vertex_count=atoi(reader.getLine().begin());
          assert(mesh->vertices.size()==vertex_count);
          for(size_t i=0; i<vertex_count; ++i){
            double u=atof(reader.getLine().begin());
            double v=atof(reader.getLine().begin());
            mesh->vertices[i].uv=Vector2(
				static_cast<float>(u), 
				static_cast<float>(v));
          }
          if(reader.getLine()!="}"){
            std::cout << "invalid close(readMeshTextureCoords)" << std::endl;
            return false;
          }
          return true;
        }

      template<class READER>
        bool readMeshMaterialList(READER &reader, MeshPtr mesh)
        {
          size_t material_count=atoi(reader.getLine().begin());
          size_t face_count=atoi(reader.getLine().begin());

          //if(face_count==mesh->faces.size()){
          for(size_t i=0; i<face_count; ++i){
            mesh->faces[i].material_index=
              atoi(reader.getTill(',', ';').begin());
          }
          //}
          //std::cout << material_count << ',' << face_count << std::endl;

          // read materials
          while(!reader.eof()){
            cstr line=reader.getLine();
            if(line==""){
              continue;
            }
            if(line=="}"){
              return true;
            }

            SPLITTER splitter(line);
            cstr key=splitter.get();
            if(key=="Material"){
              if(!readMaterial(reader, mesh)){
                return false;
              }
            }
            else if(key=="{"){
              // ?
              mesh->materials.push_back(material_map[splitter.get().str()]);
            }
            else{
              std::cout << "unknown key(MeshMaterialList): " << line << std::endl;
              if(!readChunk(reader)){
                return false;
              }
            }
          }
          std::cout << "invalid eof(readMeshMaterialList)" << std::endl;
          return false;
        }

      template<class READER>
        bool readMaterial(READER &reader, const cstr &name)
        {
          MaterialPtr material(new Material);
          material_map[name.str()]=material;

          return readMaterial(reader, material);
        }

      template<class READER>
        bool readMaterial(READER &reader, MeshPtr mesh)
        {
          MaterialPtr material(new Material);
          mesh->materials.push_back(material);

          return readMaterial(reader, material);
        }

      template<class READER>
        bool readMaterial(READER &reader, MaterialPtr material)
        {
          // rgba
          {
            material->rgba.x=static_cast<float>(atof(reader.getLine().begin()));
            material->rgba.y=static_cast<float>(atof(reader.getLine().begin()));
            material->rgba.z=static_cast<float>(atof(reader.getLine().begin()));
            material->rgba.w=static_cast<float>(atof(reader.getLine().begin()));
            //reader.getLine();
          }

          // power
          {
            material->power=static_cast<float>(atof(reader.getLine().begin()));
          }

          // specular
          {
            material->specular.x=
              static_cast<float>(atof(reader.getLine().begin()));
            material->specular.y=
              static_cast<float>(atof(reader.getLine().begin()));
            material->specular.z=
              static_cast<float>(atof(reader.getLine().begin()));
            //reader.getLine();
          }

          // emissive
          {
            material->emissive.x=
              static_cast<float>(atof(reader.getLine().begin()));
            material->emissive.y=
              static_cast<float>(atof(reader.getLine().begin()));
            material->emissive.z=
              static_cast<float>(atof(reader.getLine().begin()));
            //reader.getLine();
          }

          {
            cstr line=reader.getLine();
            while(!reader.eof()){
              // skip empty line
              if(line!=""){
                break;
              }
              line=reader.getLine();
            }

            SPLITTER splitter(line);
            cstr key=splitter.get();
            if(key=="TextureFilename"){
              // texture
              SPLITTER splitter(reader.getLine());
              material->texture=splitter.getQuated().str();
              // close
              if(reader.getLine()!="}"){
                std::cout 
                  << '(' << reader.getLineCount() << ')'
                  << "invalid close(TextureFilename)" << std::endl;
                return false;
              }
              if(reader.getLine()!="}"){
                std::cout 
                  << '(' << reader.getLineCount() << ')'
                  << "invalid close(readMaterial)" << std::endl;
                return false;
              }
              return true;
            }
            else if(key=="}"){
              return true;
            }

            std::cout 
              << '(' << reader.getLineCount() << ')'
              << "unknown key(readMaterial):" << key << std::endl;
            return false;
          }
        }

      template<class READER>
        bool readFrame(READER &reader, const cstr &name)
        {
          std::cout << "Frame: " << name << std::endl;
          size_t id=frames.size();
          FramePtr frame(new Frame(id));
          if(name!="{"){
            // no name frame for store mesh
            frame->name=name.str();

            if(id==0){
              // first frame
              rootFrame=frame;
              // pop dummy root
              currentFrame.pop();
            }
            else{
              currentFrame.top()->addChild(frame);
            }
            frames.push_back(frame);
            frame_map[frame->name]=frame;
          }
          else{
            meshFrame=frame;
          }
          currentFrame.push(frame);

          while(!reader.eof()){
            cstr line=reader.getLine();
            if(line==""){
              continue;
            }
            if(line=="}"){
				//std::cout << "close:" << currentFrame.top()->name << std::endl;
              currentFrame.pop();
              return true;
            }

            SPLITTER splitter(line);
            cstr key=splitter.get();
            if(key=="FrameTransformMatrix"){
              if(!readChunk(reader)){
                return false;
              }
            }
            else if(key=="Frame"){
              if(!readFrame(reader, splitter.get())){
                return false;
              }
            }
            else if(key=="Mesh"){
              if(!readMesh(reader, splitter.get())){
                return false;
              }
            }
            else{
              std::cout 
                << '(' << reader.getLineCount() << ')'
                << "unknown key(Frame" << frame->name << "): " 
                << key 
                << std::endl
                ;
              return false;
            }
          }
          std::cout << "invalid eof(readFrame: " 
            << frame->name << ")" << std::endl;
          return false;
        }

      template<class READER>
        bool readTemplate(READER &reader, const cstr &name)
        {
          //std::cout << "template: " << name << std::endl;
          while(!reader.eof()){
            cstr line=reader.getLine();
            if(line=="}"){
              return true;
            }
          }
          std::cout << "invalid eof(readTemplate)" << std::endl;
          return false;
        }

      template<class READER>
        bool readChunk(READER &reader)
        {
          int level=1;
          while(!reader.eof()){
            cstr line=reader.getLine();
            if(line==""){
              continue;
            }
            else if(line=="}"){
              level--;
              if(level==0){
                return true;
              }
            }
            else if(line.include('{')){
              level+=1;
            }
          }
          std::cout << "invalid eof(readChunk)" << std::endl;
          return false;
        }
    };

    struct XDelimiter
    {
      size_t check(const char *p)
      {
        switch(*p)
        {
        case '}': // fall through
        case '{': // fall through
        case ';':
          {
            ++p;
            size_t advance=1;
            while(*p==' '){
              ++p;
              ++advance;
            }
            if(p[0]=='\r' && p[1]=='\n'){
              return advance+2;
            }
            else if(p[0]=='\n'){
              return advance+1;
            }
            else{
              return advance;
            }
          }
        }
        return 0;
      }
    };

    struct XTrim
    {
      bool operator()(char c)const
      {
        switch(c)
        {
        case ' ': // fall though
        case '\t': // fall though
        case '\r': // fall though
        case '\n': // fall though
        case ';': // fall though
        case ',':
          return true;
        default:
          return false;
        }
      }
    };

    bool 
      Loader::parse(char *buf, unsigned int size)
      {
        LineReader<BufferRead, XDelimiter, XTrim> 
          reader(BufferRead(buf, size));

        // parse first line
        cstr line=reader.getTill('\n');

        SPLITTER splitter(line);
        if(splitter.get()!="xof"){
#ifdef DEBUG
          std::cout << "invalid magic number" << std::endl;
#endif
          return false;
        }
        cstr major_minor_type=splitter.get();
        major=std::atoi(
            std::string(
              major_minor_type.begin(), 
              major_minor_type.begin()+2).c_str());
        minor=std::atoi(
            std::string(
              major_minor_type.begin()+2, 
              major_minor_type.begin()+4).c_str());
        type=std::string(
            major_minor_type.begin()+4, major_minor_type.end());
        if(type!="txt"){
#ifdef DEBUG
          std::cout << "unknown type: " << type << std::endl;
#endif
          return false;
        }
        float_size=std::atoi(splitter.get().str().c_str());
#ifdef DEBUG
        std::cout << major << '.' << minor << ' ' << type << std::endl;
#endif

        Implementation impl(currentFrame, material_map, frame_map, meshFrame,
            hasVertexColor, rootFrame, meshes, animations, frames);
        if(!impl.parse(reader)){
          return false;
        }

        // post process
        // expand skinweight
        max_skin_index=0;
        min_skin_weight=1;
        BOOST_FOREACH(FramePtr frame, frames){
          postProcess(frame);
        }
        if(meshFrame){
          postProcess(meshFrame);
        }

#ifdef DEBUG
        std::cout 
          << "max_skin_index: " << max_skin_index << std::endl
          << "min_skin_weight: " << min_skin_weight << std::endl
          ;
#endif

        return true;
      }

    void 
      Loader::postProcess(FramePtr frame){
        if(!frame->mesh){
          return;
        }
        MeshPtr mesh=frame->mesh;
        for(std::map<std::string, SkinWeightPtr>::iterator 
            it=mesh->weight_map.begin();
            it!=mesh->weight_map.end();
            ++it)
        {
          FramePtr frame=frame_map[it->first];
          frame->offset=it->second->offset;
          assert(frame);
          for(size_t i=0; i<it->second->indices.size(); ++i){
            Vertex &v=mesh->vertices[it->second->indices[i]];
            v.assignWeight(frame->id, it->second->weights[i]);
            max_skin_index=std::max(
                static_cast<int>(v.weightCount), max_skin_index);
            min_skin_weight=std::min(it->second->weights[i], min_skin_weight);
          }
        }

        // update orphan vertex
        BOOST_FOREACH(Vertex &v, mesh->vertices){
          if(v.weightCount==0){
            //std::cout << "orphan" << std::endl;
            v.skinningIndex[0]=0;
            v.skinningWeight[0]=1.0f;
          }
        }

        mesh->weight_map.clear();
      }

  }
}

