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
  ReadWriteSpec rws;

public:
  Variable(const std::string &name,
           const std::string &nice_name,
           const std::string &description,
           const VT &mask,
           const ReadWriteSpec &rws);

  virtual ~Variable() {}

  inline virtual VT operator()(const VT &rv) const;

  inline virtual VT Set(const VT &rv, const VT &vv) const;

  const std::string &Name() const { return name; }
  const std::string &NiceName() const { return nice_name; }
  const std::string &Description() const { return description; }

  virtual bool Readable() const { return rws.readable; }
  virtual bool Writeable() const { return rws.writeable; }
};

#define VAROPPAR(T)                                   \
  template <>                                         \
  inline T Variable<T>::operator()(const T &rv) const \
  {                                                   \
    return (rv & mask) >> shift;                      \
  };

VAROPPAR(uint8_t);
VAROPPAR(uint16_t);
VAROPPAR(uint32_t);
VAROPPAR(uint64_t);

template<>
inline std::vector<uint8_t> Variable<std::vector<uint8_t>>::operator()(const std::vector<uint8_t> &rv) const {
  size_t to_shift = shift;
  std::vector<uint8_t> r;
  for (size_t i = rv.size()-1; i != SIZE_MAX; i--)
  {
    size_t bshft = to_shift > 8 ? 8 : to_shift;
    size_t tmp = (rv[i] & mask[i]) >> bshft;
    if (i != 0 && bshft != 8) {
      tmp |= rv[i-1] & ~mask[i];
    }
    r.push_back(tmp);
    to_shift -= bshft;
  }
  return r;
};

#define VARCONSTRUCTOR(T)                                         \
  template <>                                                     \
  inline Variable<T>::Variable(const std::string &name,           \
                        const std::string &nice_name,             \
                        const std::string &description,           \
                        const T &mask, \
                        const ReadWriteSpec &rws) : \
    name(name),              \
    nice_name(nice_name),     \
    description(description), \
    mask(mask),                \
    rws(rws) \
  {                                                               \
    if (mask == 0)                                                \
      throw std::runtime_error("invalid variable mask");          \
    shift = 0;                                                    \
    T tmp = mask;                                                 \
    while (tmp % 2 == 0)                                          \
    {                                                             \
      shift++;                                                    \
      tmp >>= 1;                                                  \
    }                                                             \
  }

VARCONSTRUCTOR(uint8_t)
VARCONSTRUCTOR(uint16_t)
VARCONSTRUCTOR(uint32_t)
VARCONSTRUCTOR(uint64_t)

template <>
inline Variable<std::vector<uint8_t>>::Variable(
    const std::string &name,
    const std::string &nice_name,
    const std::string &description,
    const std::vector<uint8_t> &mask,
    const ReadWriteSpec &rws) :
    name(name),
    nice_name(nice_name),
    description(description),
    mask(mask),
    rws(rws)
  {
    if (mask.size() == 0)
      throw std::runtime_error("invalid variable mask");
    shift = 0;
    bool seen_start = false;
    for (size_t i=0; i < mask.size(); i++) {
      uint8_t tmp = mask[i];
      if (tmp == 0) {
        if (seen_start)
          break;
        shift += 8;
      }
      else while (tmp % 2 == 0) {
        shift++;
        tmp >>= 1;
        seen_start = true;
      }
    }
  }

#define VARSET(T)                                           \
  template <>                                               \
  inline T Variable<T>::Set(const T &rv, const T &vv) const \
  {                                                         \
    return (rv & ~mask) | vv << shift;                      \
  };

VARSET(uint8_t);
VARSET(uint16_t);
VARSET(uint32_t);
VARSET(uint64_t);

template<>
inline std::vector<uint8_t> Variable<std::vector<uint8_t>>::Set(const std::vector<uint8_t> &rv, const std::vector<uint8_t> &vv) const {
  throw std::runtime_error("NYI");
  return {};
};

template <typename AT>
class RegisterBase {
protected:
  AT address;
  std::string name, nice_name, description;
  ReadWriteSpec rws;

public:
  RegisterBase(const std::string &name,
               const std::string &nice_name,
               const AT &address,
               const std::string &description,
               const ReadWriteSpec &rws) :
    address(address),
    name(name),
    nice_name(nice_name),
    description(description),
    rws(rws)
  {}
  virtual ~RegisterBase() {}
  const AT &Address() const { return address; }
  const std::string &Name() const { return name; }
  const std::string &NiceName() const { return nice_name; }
  const std::string &Description() const { return description; }
  virtual bool Readable() const { return rws.readable; }
  virtual bool Writeable() const { return rws.writeable; }
};

template <typename AT, typename VT>
class Register : public RegisterBase<AT> {
public:
  typedef std::vector<Variable<VT>*> Variables;
  typedef typename Variables::const_iterator const_iterator;

protected:
  Variables variables;

public:
  Register(const std::string &name,
           const std::string &nice_name,
           const AT &address,
           const std::string &description,
           const ReadWriteSpec &rws) :
    RegisterBase<AT>(name, nice_name, address, description, rws)
  {}
  virtual ~Register() {}

  void Add(Variable<VT> *v) { variables.push_back(v); }
  const_iterator begin() const { return variables.begin(); }
  const_iterator end() const { return variables.end(); }
  size_t size() const { return variables.size(); }
  virtual size_t value_size() const { return sizeof(VT); }
};

template <typename AT>
class VRegister : public Register<AT, std::vector<uint8_t>> {
public:
  VRegister(const std::string &name,
            const std::string &nice_name,
            const AT &address,
            const std::string &description,
            const ReadWriteSpec &rws,
            size_t num_bytes = 0) :
    Register<AT, std::vector<uint8_t>>(name, nice_name, address, description, rws),
    num_bytes(num_bytes)
  {}

  using Register<AT, std::vector<uint8_t>>::Name;
  using Register<AT, std::vector<uint8_t>>::NiceName;
  using Register<AT, std::vector<uint8_t>>::Address;
  using Register<AT, std::vector<uint8_t>>::Description;
  using Register<AT, std::vector<uint8_t>>::Readable;
  using Register<AT, std::vector<uint8_t>>::Writeable;
  using Register<AT, std::vector<uint8_t>>::Add;
  using Register<AT, std::vector<uint8_t>>::begin;
  using Register<AT, std::vector<uint8_t>>::end;
  using Register<AT, std::vector<uint8_t>>::size;
  using Register<AT, std::vector<uint8_t>>::value_size;

  virtual size_t value_size() const override { return num_bytes; }

protected:
  size_t num_bytes;
};

#endif
