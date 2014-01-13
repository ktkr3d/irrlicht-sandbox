#ifndef LOADER_CORE_H
#define LOADER_CORE_H

// Vector2, Vector3, Vector4...
#include <rigid.h>

#include <vector>
#include <cassert>
#include <cstdlib>
#include <string>
#include <istream>

namespace polymesh {

  struct BufferRead
  {
    const char *buf;
    unsigned int buf_size;
    unsigned int remain;

    BufferRead(const char *_buf, unsigned int _size)
      : buf(_buf), buf_size(_size), remain(_size)
      {}

    size_t operator()(void *dst, size_t size)
    {
      if(size>=remain){
        size=remain;
      }
      const char *p=buf+buf_size-remain;
      std::copy(p, p+size, (char*)dst);
      remain-=size;
      return size;
    }
  };

  struct StreamRead
  {
    std::istream &io;

    StreamRead(std::istream &_io)
      : io(_io)
      {}

    size_t operator()(void *buf, size_t size)
    {
      io.read((char*)buf, size);
      return io.gcount();
    }
  };

  ////////////////////////////////////////////////////////////
  // 参照オンリーの簡易文字列クラス
  ////////////////////////////////////////////////////////////
  class cstr
  {
    const char *begin_;
    const char *end_;

  public:
    cstr()
      : begin_(0), end_(0)
      {}

    cstr(const char *begin, const char *end)
      : begin_(begin), end_(end)
      { }

    bool operator==(const char *rhs)const
    {
      const char *l=begin_;
      for(const char *r=rhs; *r; ++r, ++l){
        if(l==end_){
          return false;
        }
        if(*l!=*r){
          return false;
        }
      }
      return l==end_;
    }

    bool operator!=(const char *rhs)const
    {
      return !(*this==rhs);
    }

    bool include(char c)const
    {
      for(const char *l=begin_; l!=end_; ++l){
        if(*l==c){
          return true;
        }
      }
      return false;
    }

    bool startswith(const char *rhs)
    {
      const char *r=rhs;
      for(const char *l=begin_; l!=end_ && *r!='\0'; ++l, ++r){
        if(*l!=*r){
          return false;
        }
      }
      return true;
    }

    std::string str()const{ return std::string(begin_, end_); }
    const char* begin()const{ return begin_; }
    const char* end()const{ return end_; }
    std::pair<const char*, const char*> range()const{ 
      return std::make_pair(begin_, end_); 
    }

    template<typename IsTrim>
      cstr &trim(IsTrim isTrim){
        while(begin_!=end_ && isTrim(*begin_)){
          begin_++;
        }
        while(end_!=begin_ && isTrim(end_[-1])){
          end_--;
        }
        return *this;
      }
  };
  inline std::ostream &operator<<(std::ostream &os, const cstr &rhs)
  {
    return os << rhs.str();
  }

  ////////////////////////////////////////////////////////////
  // 行指向のテキストフォーマット読み込みクラス
  ////////////////////////////////////////////////////////////

  // line delimeter
  ////////////////////////////////////////////////////////////
  struct DelCRLF
  {
    size_t check(const char *p)
    {
      switch(*p)
      {
      case '\r': // fall through
        return 2;
      case '\n':
        return 1;
      }
      return 0;
    }
  };
  struct DelCR
  {
    size_t check(const char *p)
    {
      switch(*p)
      {
      case '\n':
        return 1;
      }
      return 0;
    }
  };
  // line trim
  ////////////////////////////////////////////////////////////
  struct NoTrim
  {
    bool operator()(const char)const{ return false; }
  };
  struct WhiteSpaceTrim
  {
    bool operator()(char c)const
    {
      switch(c)
      {
      case ' ': // fall though
      case '\t': // fall though
      case '\r': // fall though
      case '\n':
        return true;
      default:
        return false;
      }
    }
  };
  // skip line
  ////////////////////////////////////////////////////////////
  struct NoSkip
  {
    bool operator()(const cstr &str)const{ return false; }
  };
  struct EmptyLineSkip
  {
    bool operator()(const cstr &line)const
    {
      return line.begin()==line.end();
    }
  };

  template<typename READ, class DELIMITER=DelCRLF, 
    class TRIM=WhiteSpaceTrim, class SKIP=EmptyLineSkip>
    class LineReader
    {
      READ read_;
      unsigned int bytes_;
      std::vector<char> buf_;
      size_t begin_;
      size_t end_;
      size_t line_count_;
      bool eof_;
      DELIMITER delimiter_;
      SKIP skip_;
      bool enough_;

      public:
      LineReader(READ read)
        : read_(read), bytes_(0), begin_(0), end_(0), line_count_(0), 
        eof_(false), enough_(false)
      {
        buf_.reserve(4096);
        fill_();
      }

      unsigned int getBytes(){ return bytes_; }
      unsigned int getLineCount(){ return line_count_; }
      bool eof(){ 
        if(!eof_){
          return false;
        }
        if(end_>=buf_.size()){
          return true;
        }
        if(end_==buf_.size()-1 && delimiter_.check(&buf_[0]+end_)){
          return true;
        }
        return false;
      }

      cstr getTill(char till)
      {
        begin_=end_;
        do {
          for(; end_!=buf_.size(); ++end_){
            if(buf_[end_]==till){
              cstr line=cstr(&buf_[0]+begin_, &buf_[0]+end_).trim(TRIM());
              ++end_;
              enough_=true;
              return line;
            }
          }
          shift_();
          fill_();
        }while(!eof());
        return cstr();
      }

      cstr getTill(char till, char orTill)
      {
        begin_=end_;
        do {
          for(; end_!=buf_.size(); ++end_){
            if(buf_[end_]==till || buf_[end_]==orTill){
              cstr line=cstr(&buf_[0]+begin_, &buf_[0]+end_).trim(TRIM());
              ++end_;
              enough_=true;
              return line;
            }
          }
          shift_();
          fill_();
        }while(!eof());
        return cstr();
      }

      cstr getLine()
      {
        cstr line;
        do {
          next_();
          if(!eof_ && end_>=buf_.size()){
            shift_();
            fill_();
            findEnd_();
          }
          assert(begin_<=end_);
          assert(begin_<=buf_.size());
          if(!buf_.empty()){
            line=cstr(&buf_[0]+begin_, &buf_[0]+end_).trim(TRIM());
          }
        }while(skip_(line) && !eof());

        line_count_++;
        return line;
      }

      private:
      void shift_()
      {
        assert(begin_<=buf_.size());
        buf_.erase(buf_.begin(), buf_.begin()+begin_);
        begin_=0;
      }

      void fill_()
      {
        size_t remain=buf_.size();
        if(!enough_){
          buf_.resize(buf_.capacity()*2);
        }
        else{
          buf_.resize(buf_.capacity());
        }
        enough_=false;
        size_t size=read_(&buf_[0]+remain, buf_.size()-remain);
        buf_.resize(remain+size);
        bytes_+=size;
        if(size==0){
          eof_=true;
        }
        end_=begin_;
      }

      void next_()
      {
        begin_=end_;
        findEnd_();
      }

      void findEnd_()
      {
        for(size_t i=begin_; i<buf_.size(); ++i){
          size_t advance=delimiter_.check(&buf_[i]);
          if(advance){
            end_=i+advance;
            enough_=true;
            return;
          }
        }
        end_=buf_.size();
      }
    };

  ////////////////////////////////////////////////////////////
  // BinaryReader::get<>用型指定
  // 構造体のポインタキャストによる代入に使う。
  // この場合、構造体のアライメントが無くなるように
  //
  // VCでは
  // #ifdef _MSC_VER
  // #pragma pack( push, 1 )
  // #endif
  // #ifdef _MSC_VER
  // #pragma pack( pop )
  // #endif
  //
  // gccでは
  // #ifdef __GNUG__
  // __attribute__((packed))
  // #endif
  //
  // とし、
  // BOOST_STATIC_ASSERT(sizeof(Morph)==23);
  // で構造体が意図したサイズになっているか確認すること
  //
  // reader.get(TYPE<Morph, 23>());
  // のように使う。
  // x86では使ってもよい？
  //
  ////////////////////////////////////////////////////////////
  template<typename T, size_t S=sizeof(T)>
    struct TYPE
    {
      typedef T type;
      enum { size=S };
    };

  ////////////////////////////////////////////////////////////
  // バイナリフォーマット読み込みクラス
  ////////////////////////////////////////////////////////////
  template<typename READ>
    class BinaryReader
    {
      READ read_;
      std::vector<char> buf_;
      size_t buf_pos_;
      size_t pos_;
      bool eof_;
      size_t read_size_;
      typedef std::pair<char*, char*> CharRange;

    private: 
      // non copyable
      BinaryReader (const BinaryReader &);
      BinaryReader & operator = (const BinaryReader &);

    public:
      BinaryReader(READ read)
        : read_(read), buf_(4096), buf_pos_(0), 
        pos_(0), eof_(false), read_size_(0)
        {
          fill_();
        }

      size_t pos(){ return pos_; }
      bool eof(){ 
        return eof_ && buf_pos_ >= buf_.size();
      }

      std::string getString(size_t size)
      {
        CharRange range=get_(size);
        char *end=range.first;
        for(; end!=range.second; ++end){
          if(*end=='\r' || *end=='\0' || *end=='\n'){
            break;
          }
        }
        return std::string(range.first, end);
      }

      void getBytes(char *buf, size_t size)
      {
        CharRange range=get_(size);
        if(buf){
          std::copy(range.first, range.second, buf);
        }
      }

      template<typename T>
        typename T::type
        get(T type)
        {
          std::pair<char*, char*> range=get_(T::size);
          return *((typename T::type*)range.first);
        }

    private:
      CharRange get_(size_t size){
        assert(size<=buf_.size());
        if(buf_pos_+size>=buf_.size()){
          fill_();
        }
        size_t pos=buf_pos_;
        buf_pos_+=size;
        pos_+=size;
        return std::make_pair(&buf_[0]+pos, &buf_[0]+(pos+size));
      }

      void fill_()
      {
        size_t read_pos=0;
        if(buf_pos_>0){
          // 前に詰める
          buf_.erase(buf_.begin(), buf_.begin()+buf_pos_);
          buf_pos_=0;
          read_pos=buf_.size();
          buf_.resize(buf_.capacity());
        }
        size_t read_size=read_(&buf_[read_pos], buf_.size()-read_pos);
        read_size_+=read_size;
        if(read_pos+read_size<buf_.size()){
          /*
          std::fill(
              buf_.begin()+(read_pos+read_size), buf_.end(), 0);
              */
          buf_.resize(read_pos+read_size);            
        }
        if(read_size==0){
          eof_=true;
        }
      }
    };

  ////////////////////////////////////////////////////////////
  // トークン分解器
  ////////////////////////////////////////////////////////////
  template<typename DELIMETER>
    class LineSplitter
    {
      const char *begin_;
      const char *end_;
      DELIMETER is_delimiter_;

    public:
      LineSplitter(const cstr &str)
        : begin_(str.begin()), end_(str.end())
        {
        }

      void skip(const std::string &begin, const std::string &end)
      {
        for(size_t i=0; i<begin.size(); ++i){
          if(begin[i]!=begin_[i]){
            return;
          }
        }
        size_t size=end_-begin_;
        for(size_t i=begin.size(); i<size-1; ++i){
          if(begin_[i]==end[0]){
            bool found=true;
            for(size_t j=1; j<end.size(); ++j){
              if(begin_[i+j]!=end[j]){
                found=false;
                break;
              }
            }
            if(found){
              begin_+=i+end.size()+1;
              break;
            }
          }
        }
      }

      cstr get()
      {
        for(; begin_!=end_ && is_delimiter_(*begin_); ++begin_){
          // skip delimeter
          ;
        }
        assert(begin_<=end_);

        if(begin_==end_){
          return cstr(begin_, end_);
        }

        const char *p=begin_;
        for(; p!=end_; ++p){
          // find token end
          if(is_delimiter_(*p)){
            break;
          }
        }

        cstr token=cstr(begin_, p);
        begin_=p;
        return token;
      }

      cstr getQuated()
      {
        for(; begin_!=end_; ++begin_){
          if(*begin_=='"'){
            break;
          }
        }
        begin_++;
        assert(begin_<=end_);

        const char *c=begin_+1;
        for(; c!=end_; ++c){
          if(*c=='"'){
            break;
          }
        }

        cstr token=cstr(begin_, c);
        begin_=c+1;
        return token;
      }

      int getInt()
      {
        return std::atoi(get().str().c_str());
      }

      int getHex()
      {
        int num;
        cstr token=get();
        for(const char *p=token.begin(); p!=token.end(); ++p){
          switch(*p)
          {
          case '0': num=num*16; break;
          case '1': num=num*16+1; break;
          case '2': num=num*16+2; break;
          case '3': num=num*16+3; break;
          case '4': num=num*16+4; break;
          case '5': num=num*16+5; break;
          case '6': num=num*16+6; break;
          case '7': num=num*16+7; break;
          case '8': num=num*16+8; break;
          case '9': num=num*16+9; break;
          case 'a': num=num*16+10; break;
          case 'b': num=num*16+11; break;
          case 'c': num=num*16+12; break;
          case 'd': num=num*16+13; break;
          case 'e': num=num*16+14; break;
          case 'f': num=num*16+15; break;
          default: return num;
          }
        }
        return num;
      }

      float getFloat()
      {
        return static_cast<float>(std::atof(get().str().c_str()));
      }

      rigid::Vector2 getVector2()
      {
        float x=getFloat();
        float y=getFloat();
        return rigid::Vector2(x, y);
      }

      rigid::Vector3 getVector3()
      {
        float x=getFloat();
        float y=getFloat();
        float z=getFloat();
        return rigid::Vector3(x, y, z);
      }

      rigid::Vector4 getVector4()
      {
        float x=getFloat();
        float y=getFloat();
        float z=getFloat();
        float w=getFloat();
        return rigid::Vector4(x, y, z, w);
      }
    };


}

#endif // LOADER_H
