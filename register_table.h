// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _REGISTER_TABLE_H_
#define _REGISTER_TABLE_H_

#include <iostream>

#include "register.h"

template <typename AT, typename VT, typename DEVICE>
class RegisterTable
{
public:
  typedef std::vector<Register<AT, VT>*> Registers;
  typedef typename Registers::const_iterator const_iterator;
  typedef typename Registers::iterator iterator;

protected:
  DEVICE &device;
  Registers registers;
  std::vector<VT> buffer;

public:
  RegisterTable(DEVICE &device) : device(device) {}
  virtual ~RegisterTable() {}
  const std::vector<VT> &Buffer() const { return buffer; }
  const_iterator begin() const { return registers.begin(); }
  const_iterator end() const { return registers.end(); }
  size_t size() const { return registers.size(); }
  virtual void Refresh(bool frequent) = 0;
  DEVICE &Device() const { return device; }
  void Initialize() {
    AT max_address = 0;
    for (auto r : registers)
      max_address = std::max(max_address, r->Address());
    buffer.resize(max_address + 1, 0);
  }
};

#define REGDECL(AT, VT, N, NN, A, RW, D, V)                                    \
  class N##Register : public Register<AT, VT> {                                \
  public:                                                                      \
    N##Register(std::vector<Register<AT, VT> *> &rs)                           \
        : Register<AT, VT>(STR(N), NN, A, D) {                                 \
      rs.push_back(this);                                                      \
    }                                                                          \
    virtual ~N##Register() {}                                                  \
    virtual const VT &operator()(const std::vector<VT> &buf) const {           \
      return buf[Address()];                                                   \
    }                                                                          \
    virtual bool Readable() const { return RW.readable; }                      \
    virtual bool Writeable() const { return RW.writeable; }                    \
  };                                                                           \
  N##Register _r##N = N##Register(registers);                                  \
  VT N() const { return buffer[A]; }                                           \
  V

#define STR(S) #S

#define VARDECL(AT, VT, R, N, NN, M, RW, D)                                    \
  class N##Variable : public Variable<VT> {                                    \
  public:                                                                      \
    N##Variable(R##Register &r) : Variable<VT>(STR(N), NN, D, M) {             \
      r.Add(this);                                                             \
    }                                                                          \
    virtual ~N##Variable() {}                                                  \
    virtual bool Readable() const { return RW.readable; }                      \
    virtual bool Writeable() const { return RW.writeable; }                    \
  };                                                                           \
  N##Variable _v##N = N##Variable(_r##R);                                      \
  VT N() const { return _v##N(R()); }

#define REGISTER_TABLE(D, N, AT, VT, R)                                        \
  class D::N : public ::RegisterTable<AT, VT, D> {                             \
  public:                                                                      \
    N(D &d) : ::RegisterTable<AT, VT, D>(d) {}                                 \
    virtual ~N() {}                                                            \
    virtual void Refresh(bool frequent);                                       \
    R;                                                                         \
  };

#define REGISTER_TABLE_SET_TABLE(D, N, AT, VT, R)                              \
  class N##RT : public ::RegisterTable<AT, VT, D> {                            \
  public:                                                                      \
    N##RT(D &d) : ::RegisterTable<AT, VT, D>(d) {}                             \
    virtual ~N##RT() {}                                                        \
    virtual void Refresh(bool frequent);                                       \
    R;                                                                         \
  };                                                                           \
  N##RT N;

#define REGISTER_TABLE_W(D, N, AT, VT, R)                                    \
  class D::N : public ::RegisterTable<AT, VT, D>                             \
  {                                                                          \
  public:                                                                    \
    N(D &d) : ::RegisterTable<AT, VT, D>(d) {}                               \
    virtual ~N() {}                                                          \
    virtual void Refresh(bool frequent);                                     \
    virtual void Read(std::istream &is);                                     \
    virtual void Write(std::ostream &os);                                    \
    virtual void Write(const Register<AT, VT> &reg, const VT &value) {       \
      device.Write(reg, value);                                              \
    }                                                                        \
    virtual void Write(const Register<AT, VT> &reg, const Variable<VT> &var, \
                       const VT &value) {                                    \
      device.Write(reg, var.Set(reg(buffer), value));                        \
    }                                                                        \
    R;                                                                       \
  };

#define REGISTER_TABLE_SET_TABLE_W(D, N, AT, VT, R)                            \
  class N##RT : public ::RegisterTable<AT, VT, D> {                            \
  public:                                                                      \
    N##RT(D &d) : ::RegisterTable<AT, VT, D>(d) {}                             \
    virtual ~N##RT() {}                                                        \
    virtual void Refresh(bool frequent);                                       \
    virtual void Read(std::istream &is);                                       \
    virtual void Write(std::ostream &os);                                      \
    virtual void Write(const Register<AT, VT> &reg, const VT &value) {         \
      device.Write(reg, value);                                                \
    }                                                                          \
    virtual void Write(const Register<AT, VT> &reg, const Variable<VT> &var,   \
                       const VT &value) {                                      \
      device.Write(reg, var.Set(reg(buffer), value));                          \
    }                                                                          \
    R;                                                                         \
  };                                                                           \
  N##RT N;

#define REGISTER_TABLE_SET(D, S, N, T)                                         \
  class S::N {                                                                 \
  protected:                                                                   \
    D &device;                                                                 \
  public:                                                                      \
    N(D &device);                                                              \
    virtual ~N();                                                              \
    T;                                                                         \
    void Refresh(bool frequent);                                               \
    virtual void Read(std::istream &is);                                       \
    virtual void Write(std::ostream &os);                                      \
  };

#endif
