#ifndef BVH_LOADER_H
#define BVH_LOADER_H

#include "core.h"
#include <map>
#include <boost/shared_ptr.hpp>
#include <iostream>

namespace polymesh {
  namespace bvh {

    typedef rigid::Vector3 Vector3;

    enum CHANNEL_TYPE
    {
      CHANNEL_POSITION_X,
      CHANNEL_POSITION_Y,
      CHANNEL_POSITION_Z,
      CHANNEL_ROTATION_X,
      CHANNEL_ROTATION_Y,
      CHANNEL_ROTATION_Z,
    };

    struct IChannel
    {
      std::vector<float> values;
    };
    typedef boost::shared_ptr<IChannel> IChannelPtr;

    struct ChannelPositionX : public IChannel
    {
    };
    struct ChannelPositionY : public IChannel
    {
    };
    struct ChannelPositionZ : public IChannel
    {
    };
    struct ChannelRotationX : public IChannel
    {
    };
    struct ChannelRotationY : public IChannel
    {
    };
    struct ChannelRotationZ : public IChannel
    {
    };

    struct Joint
    {
      std::string name;
      Vector3 pos;
      std::vector<IChannelPtr> channels;
      std::vector<boost::shared_ptr<Joint> > children;
    };
    typedef boost::shared_ptr<Joint> JointPtr;
    inline std::ostream& operator<<(std::ostream &os, const Joint &rhs)
    {
      os << "[Joint " << rhs.name << " " << rhs.pos << "]";
      return os;
    }

    struct Delimeter
    {
      bool operator()(char c)
      {
        switch(c)
        {
        case ' ':
        case '\t':
          return true;
        default:
          return false;
        }
      }
    };
    typedef LineSplitter<Delimeter> SPLITTER;

    class Loader
    {
    public:
      JointPtr root;
      std::map<std::string, JointPtr> joint_map;
      std::vector<IChannelPtr> channels;
      float frameTime;

      Loader()
        : frameTime(0)
        {}

      bool parse(char *buf, unsigned int size);
    };

    inline std::ostream& operator<<(std::ostream &os, const Loader &rhs)
    {
      os
        << "<BVH "
        << rhs.joint_map.size() << "joints" << std::endl
        ;
      for(std::map<std::string, JointPtr>::const_iterator it=rhs.joint_map.begin();
          it!=rhs.joint_map.end();
          ++it){
        std::cout << *(it->second) << std::endl;
      }
      os
        << rhs.channels.size() << "channels"
        << ", " << rhs.channels.back()->values.size() << "frames"
        << ", " << rhs.frameTime << "sec/frame"
        << ">"
        ;
      return os;
    }
  }
}

#endif // BVH_LOADER_H
