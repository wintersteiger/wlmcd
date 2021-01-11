// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _REGISTER_H_
#define _REGISTER_H_

#include <string>
#include <vector>
#include <stdexcept>

typedef struct {
  bool readable;
  bool writeable;
} ReadWriteSpec;

static const ReadWriteSpec RO = { .readable=true, .writeable=false };
static const ReadWriteSpec RW = { .readable=true, .writeable=true };
static const ReadWriteSpec WO = { .readable=false, .writeable=true };

template <typename VT>
class Variable
{
protected:
  std::string name, nice_name, description;
  const VT mask;
  uint16_t shift;

public:
  Variable(const std::string &name,
           const std::string &nice_name,
           const std::string &description,
           const VT mask) :
    name(name),
    nice_name(nice_name),
    description(description),
    mask(mask)
  {
    if (mask == 0)
      throw std::runtime_error("invalid variable mask");
    shift = 0;
    VT tmp = mask;
    while (tmp % 2 == 0) {
      shift++;
      tmp >>= 1;
    }
  }

  virtual ~Variable() {}

  virtual VT operator()(const VT &rv) const {
    return (rv & mask) >> shift;
  };

  virtual VT Set(const VT &rv, const VT &vv) const {
    return (rv & ~mask) | vv << shift;
  };

  const std::string &Name() const { return name; }
  const std::string &NiceName() const { return name; }
  const std::string &Description() const { return description; }

  virtual bool Readable() const { return RW.readable; }
  virtual bool Writeable() const { return RW.writeable; }
};

template <typename AT, typename VT>
class Register {
public:
  typedef std::vector<Variable<VT>*> Variables;
  typedef typename Variables::const_iterator const_iterator;

protected:
  std::string name, nice_name, description;
  AT address;
  Variables variables;

public:
  Register(const std::string &name,
           const std::string &nice_name,
           const AT &address,
           const std::string &description) :
    name(name),
    nice_name(nice_name),
    description(description),
    address(address)
  {}
  virtual ~Register() {}

  const std::string &Name() const { return name; }
  const std::string &NiceName() const { return nice_name; }
  const AT &Address() const { return address; }
  const std::string &Description() const { return description; }
  virtual bool Readable() const { return true; }
  virtual bool Writeable() const { return false; }
  virtual const VT &operator()(const std::vector<VT> &buf) const = 0;

  void Add(Variable<VT> *v) { variables.push_back(v); }
  const_iterator begin() const { return variables.begin(); }
  const_iterator end() const { return variables.end(); }
  size_t size() const { return variables.size(); }
};

#endif
