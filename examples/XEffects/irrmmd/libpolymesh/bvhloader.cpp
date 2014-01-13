#include "bvhloader.h"

namespace polymesh {
  namespace bvh {

    class Impl
    {
      std::vector<JointPtr> stack_;
      Loader &l;

    public:
      Impl(Loader &loader)
        : l(loader)
        {}

      template<class READ>
        bool parse(READ &read)
        {
          LineReader<READ> reader(read);

          ////////////////////////////////////////////////////////////
          // HIERARCHY
          ////////////////////////////////////////////////////////////
          if(reader.getLine()!="HIERARCHY"){
            //std::cout << "invalid signature" << std::endl;
            return false;
          }

          cstr line=reader.getLine();
          SPLITTER splitter(line);
          if(splitter.get()!="ROOT"){
            return false;
          }
          if(!parseJoint(reader, splitter.get().str())){
            return false;
          }

          std::cout << l.channels.size() << " channels" << std::endl;

          ////////////////////////////////////////////////////////////
          // MOTION
          ////////////////////////////////////////////////////////////
          if(reader.getLine()!="MOTION"){
            return false;
          }

          // Frames
          line=reader.getLine();
          splitter=SPLITTER(line);
          if(splitter.get()!="Frames:"){
            return false;
          }
          int frames=splitter.getInt();

          // Frame Time
          line=reader.getLine();
          splitter=SPLITTER(line);
          if(splitter.get()!="Frame"){
            return false;
          }
          if(splitter.get()!="Time:"){
            return false;
          }
          l.frameTime=splitter.getFloat();

          while(!reader.eof()){
            line=reader.getLine();
            if(line==""){
              break;
            }
            SPLITTER splitter(line);
            for(size_t i=0; ; ++i){
              cstr value=splitter.get();
              if(value==""){
                break;
              }
              l.channels[i]->values.push_back(
                  static_cast<float>(std::atof(value.str().c_str())));
            }
          }

          if(l.channels.back()->values.size()!=frames){
            std::cout << l.channels.back()->values.size() << "frames" << std::endl;
            return false;
          }

          return true;
        }

    private:
      template<class READER>
        bool parseJoint(READER &reader, const std::string &name)
        {
          JointPtr joint(new Joint);
          joint->name=name;
          l.joint_map.insert(std::make_pair(name, joint));
          if(stack_.empty()){
            l.root=joint;
          }
          else{
            stack_.back()->children.push_back(joint);
          }
          stack_.push_back(joint);

          cstr line=reader.getLine();
          if(line!="{"){
            std::cout << "expected { but " << line << std::endl;
            return false;
          }
          // offset
          line=reader.getLine();
          SPLITTER splitter(line);
          if(splitter.get()!="OFFSET"){
            return false;
          }
          joint->pos.x=splitter.getFloat();
          joint->pos.y=splitter.getFloat();
          joint->pos.z=splitter.getFloat();

          // channel
          line=reader.getLine();
          splitter=SPLITTER(line);
          if(splitter.get()!="CHANNELS"){
            return false;
          }
          int channel_count=splitter.getInt();
          for(int i=0; i<channel_count; ++i){
            cstr channel=splitter.get();
            if(channel=="Xposition"){
              joint->channels.push_back(IChannelPtr(new ChannelPositionX));
              l.channels.push_back(IChannelPtr(new ChannelPositionX));
            }
            else if(channel=="Yposition"){
              joint->channels.push_back(IChannelPtr(new ChannelPositionY));
              l.channels.push_back(IChannelPtr(new ChannelPositionY));
            }
            else if(channel=="Zposition"){
              joint->channels.push_back(IChannelPtr(new ChannelPositionZ));
              l.channels.push_back(IChannelPtr(new ChannelPositionZ));
            }
            else if(channel=="Xrotation"){
              joint->channels.push_back(IChannelPtr(new ChannelRotationX));
              l.channels.push_back(IChannelPtr(new ChannelRotationX));
            }
            else if(channel=="Yrotation"){
              joint->channels.push_back(IChannelPtr(new ChannelRotationY));
              l.channels.push_back(IChannelPtr(new ChannelRotationY));
            }
            else if(channel=="Zrotation"){
              joint->channels.push_back(IChannelPtr(new ChannelRotationZ));
              l.channels.push_back(IChannelPtr(new ChannelRotationZ));
            }
            else{
              std::cout << "unknown channel: " << channel << std::endl;
              assert(false);
            }
          }

          //
          while(true){
            line=reader.getLine();
            SPLITTER splitter(line);
            cstr key=splitter.get();
            if(key=="}"){
              stack_.pop_back();
              return true;
            }
            else if(key=="JOINT"){
              if(!parseJoint(reader, splitter.get().str())){
                return false;
              }
            }
            else if(key=="End"){
              ////////////////////
              // End Site
              ////////////////////
              cstr line=reader.getLine();
              if(line!="{"){
                std::cout << "expected '{' but: " << line << std::endl;
                return false;
              }
              // body
              line=reader.getLine();
              // close
              line=reader.getLine();
              if(line!="}"){
                std::cout << "expected '}' but: " << line << std::endl;
                return false;
              }
            }
          }
          return false;
        }
    };

    bool 
      Loader::parse(char *buf, unsigned int size)
      {
        BufferRead read(buf, size);
        Impl impl(*this);
        return impl.parse(read);
      }

  }
}
