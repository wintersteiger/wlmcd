// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _REGISTER_TABLE_H_
#define _REGISTER_TABLE_H_

#include <iostream>
#include <vector>
#include <map>

#include "register.h"

#define STR(S) #S

class RegisterTableBase
{
public:
  RegisterTableBase() = default;
  virtual ~RegisterTableBase() = default;
  virtual void Initialize() = 0;
};

template <typename AT, typename VT>
class DenseBuffer : public std::vector<VT> {
public:
  DenseBuffer() : std::vector<VT>() {}
  DenseBuffer(size_t size, const VT& value) : std::vector<VT>(size, value) {}
  virtual ~DenseBuffer() {}

  using std::vector<VT>::operator=;

  const VT& operator[](const AT& address) const {
    return std::vector<VT>::operator[](address);
  }

  VT& operator[](const AT& address) {
    return std::vector<VT>::operator[](address);
  }
};


template <typename AT, typename VT>
class SparseBuffer : public std::map<AT, VT> {
public:
  SparseBuffer() : std::map<AT, VT>() {}
  virtual ~SparseBuffer() {}

  using std::map<AT, VT>::emplace;

  const VT& operator[](const AT& address) const {
    static VT no_value = VT();
    auto bit = std::map<AT, VT>::find(address);
    if (bit == std::map<AT, VT>::end())
      return no_value;
    return bit->second;
  }

  VT& operator[](const AT& address) {
    static VT no_value = VT();
    auto bit = std::map<AT, VT>::find(address);
    if (bit == std::map<AT, VT>::end())
      return no_value;
    return bit->second;
  }
};

template <typename AT, typename VT, typename DEVICE, typename BT>
class RegisterTableT : public RegisterTableBase
{
public:
  class TRegister : public Register<AT, VT> {
  public:
    TRegister(const std::string &name,
              const std::string &nice_name,
              const AT &address,
              const std::string &description,
              const ReadWriteSpec &rws,
              std::vector<TRegister*> &rs) :
        Register<AT, VT>(name, nice_name, address, description, rws) {
      rs.push_back(this);
    }
  };

  class TVariable : public Variable<VT> {
  public:
    TVariable(const std::string &name,
              const std::string &nice_name,
              const std::string &desc,
              const VT& mask,
              const ReadWriteSpec &rws,
              TRegister &r) :
      Variable<VT>(name, nice_name, desc, mask, rws)
    {
      r.Add(this);
    }
    virtual ~TVariable() {}
  };

  typedef std::vector<TRegister*> Registers;
  typedef typename Registers::const_iterator const_iterator;
  typedef typename Registers::iterator iterator;

protected:
  DEVICE &device;
  Registers registers;
  BT buffer;

public:
  RegisterTableT(DEVICE &device) : RegisterTableBase(), device(device) {}
  virtual ~RegisterTableT() {}
  const_iterator begin() const { return registers.begin(); }
  const_iterator end() const { return registers.end(); }
  size_t size() const { return registers.size(); }
  virtual void Refresh(bool frequent) = 0;
  DEVICE &Device() const { return device; }
  virtual void Initialize() override {
    // Todo: this doesn't work for maps
    AT max_address = 0;
    for (auto r : registers)
      max_address = std::max(max_address, r->Address());
    buffer.resize(max_address + 1);
  }
  virtual const VT& operator()(const TRegister& r) const {
    return buffer[r.Address()];
  }
  virtual void Write(const TRegister &reg, const VT &value) {
    device.Write(reg.Address(), value);
  }
  virtual void Write(const TRegister &reg, const Variable<VT> &var, const VT &value) {
    device.Write(reg.Address(), var.Set((*this)(reg), value));
  }
  virtual TRegister *Find(size_t addr)
  {
    for (auto &r : registers)
      if (r->Address() == addr)
        return r;
    return nullptr;
  }
};

template <typename AT, typename VT, typename DEVICE>
using RegisterTable = RegisterTableT<AT, VT, DEVICE, DenseBuffer<AT, VT>>;

template <typename AT, typename VT, typename DEVICE>
using RegisterTableSparse = RegisterTableT<AT, VT, DEVICE, SparseBuffer<AT, VT>>;

template <typename AT, typename DEVICE>
class RegisterTableSparseVar : public RegisterTableBase
{
public:
  class TRegister : public VRegister<AT> {
  public:
    TRegister(const std::string &name,
              const std::string &nice_name,
              const AT &address,
              const std::string &description,
              const ReadWriteSpec &rws,
              size_t num_bytes,
              std::vector<TRegister*> &rs) :
        VRegister<AT>(name, nice_name, address, description, rws, num_bytes) {
      rs.push_back(this);
    }
  };

  class TVariable : public Variable<std::vector<uint8_t>> {
  public:
    TVariable(const std::string &name,
              const std::string &nice_name,
              const std::string &desc,
              const std::vector<uint8_t>& mask,
              const ReadWriteSpec &rws,
              TRegister &r) :
      Variable<std::vector<uint8_t>>(name, nice_name, desc, mask, rws)
    {
      r.Add(this);
    }
    virtual ~TVariable() {}
  };

  typedef std::vector<TRegister*> Registers;
  typedef typename Registers::const_iterator const_iterator;
  typedef typename Registers::iterator iterator;

protected:
  DEVICE &device;
  Registers registers;
  SparseBuffer<AT, std::vector<uint8_t>> buffer;

public:
  RegisterTableSparseVar(DEVICE &device) : RegisterTableBase(), device(device) {}
  virtual ~RegisterTableSparseVar() {}
  const_iterator begin() const { return registers.begin(); }
  const_iterator end() const { return registers.end(); }
  size_t size() const { return registers.size(); }
  virtual void Refresh(bool frequent) = 0;
  DEVICE &Device() const { return device; }
  virtual void Initialize() override {
    for (auto r : registers)
      buffer.emplace(r->Address(), std::vector<uint8_t>(r->value_size(), 0));
  }
  virtual const std::vector<uint8_t>& operator()(const TRegister& r) const {
    return buffer[r.Address()];
  }
  virtual void Write(const TRegister &reg, const std::vector<uint8_t> &value) {
    device.Write(reg.Address(), value);
  }
  virtual void Write(const TRegister &reg, const Variable<std::vector<uint8_t>> &var, const std::vector<uint8_t> &value) {
    device.Write(reg.Address(), var.Set((*this)(reg), value));
  }
  virtual TRegister *Find(size_t addr)
  {
    for (auto &r : registers)
      if (r->Address() == addr)
        return r;
    return nullptr;
  }
};


#define REGDECL(AT, VT, N, NN, A, RW, D, V)                     \
  TRegister _r##N = TRegister(STR(N), NN, A, D, RW, registers); \
  VT N() const { return (*this)(_r##N); }                       \
  V

#define VREGDECL(AT, SZ, N, NN, A, RW, D, V)                        \
  TRegister _r##N = TRegister(STR(N), NN, A, D, RW, SZ, registers); \
  const std::vector<uint8_t> &N() const { return (*this)(_r##N); }  \
  V

#define VARDECL(AT, VT, R, N, NN, M, RW, D)                 \
  TVariable _v##N = TVariable(STR(N), NN, D, M, RW, _r##R); \
  VT N() const { return _v##N(R()); }


#define REGISTER_TABLE_SET_TABLE(D, N, AT, VT, R)  \
  class N##RT : public ::RegisterTable<AT, VT, D>  \
  {                                                \
  public:                                          \
    N##RT(D &d) : ::RegisterTable<AT, VT, D>(d) {} \
    virtual ~N##RT() {}                            \
    virtual void Refresh(bool frequent);           \
    R;                                             \
  };                                               \
  N##RT N;

#define REGISTER_TABLE(D, N, AT, VT, R)          \
  class D::N : public ::RegisterTable<AT, VT, D> \
  {                                              \
  public:                                        \
    N(D &d) : ::RegisterTable<AT, VT, D>(d) {}   \
    virtual ~N() {}                              \
    virtual void Refresh(bool frequent);         \
    virtual void Read(std::istream &is);         \
    virtual void Write(std::ostream &os);        \
    using ::RegisterTable<AT, VT, D>::Write;     \
    using ::RegisterTable<AT, VT, D>::Find;      \
    R;                                           \
  };

#define REGISTER_TABLE_SPARSE(D, N, AT, VT, R)         \
  class D::N : public ::RegisterTableSparse<AT, VT, D> \
  {                                                    \
  public:                                              \
    N(D &d) : ::RegisterTableSparse<AT, VT, D>(d) {}   \
    virtual ~N() {}                                    \
    virtual void Refresh(bool frequent);               \
    virtual void Read(std::istream &is);               \
    virtual void Write(std::ostream &os);              \
    using ::RegisterTable<AT, VT, D>::Write;           \
    using ::RegisterTable<AT, VT, D>::Find;            \
    R;                                                 \
  };

#define REGISTER_TABLE_SPARSE_VAR(D, N, AT, R)                 \
  class D::N : public ::RegisterTableSparseVar<AT, D>          \
  {                                                            \
  public:                                                      \
    N(D &d) : ::RegisterTableSparseVar<AT, D>(d) {}            \
    virtual ~N() {}                                            \
    virtual void Refresh(bool frequent);                       \
    virtual void Read(std::istream &is);                       \
    virtual void Write(std::ostream &os);                      \
    using ::RegisterTableSparseVar<AT, D>::Write; \
    using ::RegisterTableSparseVar<AT, D>::Find;  \
    R;                                                         \
  };

#define REGISTER_TABLE_SET_TABLE_W(D, N, AT, VT, R) \
  class N##RT : public ::RegisterTable<AT, VT, D>   \
  {                                                 \
  public:                                           \
    N##RT(D &d) : ::RegisterTable<AT, VT, D>(d) {}  \
    virtual ~N##RT() {}                             \
    virtual void Refresh(bool frequent);            \
    virtual void Read(std::istream &is);            \
    virtual void Write(std::ostream &os);           \
    using ::RegisterTable<AT, VT, D>::Write;        \
    using ::RegisterTable<AT, VT, D>::Find;         \
    R;                                              \
  };                                                \
  N##RT N;

#define REGISTER_TABLE_SET(D, S, N, T)    \
  class S::N                              \
  {                                       \
  protected:                              \
    D &device;                            \
                                          \
  public:                                 \
    N(D &device);                         \
    virtual ~N();                         \
    T;                                    \
    void Refresh(bool frequent);          \
    virtual void Read(std::istream &is);  \
    virtual void Write(std::ostream &os); \
  };

#endif
