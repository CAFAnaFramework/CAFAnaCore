#include "CAFAna/Core/VarBase.h"

#include "CAFAna/Core/DepMan.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>

namespace ana
{
  //----------------------------------------------------------------------
  VarBase::VarBase(const std::function<VoidVarFunc_t>& func, int id)
    : fFunc(func), fID((id >= 0) ? id : fgNextID++)
  {
    DepMan<VarBase>::Instance().RegisterConstruction(this);
  }

  //----------------------------------------------------------------------
  VarBase::~VarBase()
  {
    DepMan<VarBase>::Instance().RegisterDestruction(this);
  }

  //----------------------------------------------------------------------
  VarBase::VarBase(const VarBase& v)
    : fFunc(0), fID(-1)
  {
    if(&v == this) return;

    if(v.fFunc){
      fFunc = v.fFunc;
      fID = v.fID;

      DepMan<VarBase>::Instance().RegisterConstruction(this);
    }
    else{
      // If we are copying from a Var with NULL func, that is probably because
      // it is all zero because it hasn't been statically constructed
      // yet. Register our interest of getting constructed in turn once it is.
      DepMan<VarBase>::Instance().RegisterDependency(&v, this);
    }
  }

  //----------------------------------------------------------------------
  VarBase& VarBase::operator=(const VarBase& v)
  {
    if(&v == this) return *this;

    if(v.fFunc){
      fFunc = v.fFunc;
      fID = v.fID;

      DepMan<VarBase>::Instance().RegisterConstruction(this);
    }
    else{
      fFunc = 0;
      fID = -1;

      // If we are copying from a Var with NULL func, that is probably because
      // it is all zero because it hasn't been statically constructed
      // yet. Register our interest of getting constructed in turn once it is.
      DepMan<VarBase>::Instance().RegisterDependency(&v, this);
    }

    return *this;
  }


  //----------------------------------------------------------------------
  // Extract the value (trivial) and print a good error message if it's NaN
  double ValHelper(const VarBase& v,
                   const std::string& op,
                   double c, const void* rec)
  {
    const double val = v(rec);
    if(std::isnan(val)){
      std::cout << "Warning: Cut compares NaN " << op << " " << c << std::endl;
    }
    return val;
  }

  void ValHelper(const VarBase& va, const VarBase& vb,
                 const std::string& op,
                 const void* rec,
                 double& a, double& b)
  {
    a = va(rec);
    b = vb(rec);
    if(std::isnan(a) || std::isnan(b)){
      std::cout << "Warning: Cut compares " << a << " " << op << " " << b << std::endl;
    }
  }


  // These comparison operators are extremely rote. Use a macro

#define COMPARISON(OP, OPNAME)                                          \
  /* Comparison of a Var with a constant */                             \
  CutBase VarBase::                                                     \
  operator OP(double c) const                                           \
  {                                                                     \
    struct OPNAME                                                       \
    {                                                                   \
      VarBase v; double c;                                              \
      bool operator()(const void* rec)                                  \
      {                                                                 \
        return ValHelper(v, #OP, c, rec) OP c;                          \
      }                                                                 \
    };                                                                  \
                                                                        \
    return CutBase(OPNAME{*this, c}, 0, 0, -1);                         \
  }                                                                     \
                                                                        \
  /* Comparison of a Var with another Var */                            \
  CutBase VarBase::                                                     \
  operator OP(const VarBase& v) const                                   \
  {                                                                     \
    struct OPNAME                                                       \
    {                                                                   \
      VarBase a, b;                                                     \
      bool operator()(const void* rec)                                  \
      {                                                                 \
        double va, vb;                                                  \
        ValHelper(a, b, #OP, rec, va, vb);                              \
        return va OP vb;                                                \
      }                                                                 \
    };                                                                  \
                                                                        \
    return CutBase(OPNAME{*this, v}, 0, 0, -1);                         \
  }                                                                     \
  void dummy() // trick to require a semicolon

  COMPARISON(>,  Greater);
  COMPARISON(>=, GreaterEquals);
  COMPARISON(<,  Less);
  COMPARISON(<=, LessEquals);
  COMPARISON(==, Equals);
  COMPARISON(!=, NotEquals);

  int VarBase::fgNextID = 0;

  //----------------------------------------------------------------------
  Var2DMapper::Var2DMapper(const Binning& binsa, const Binning& binsb)
    : fBinsA(binsa), fBinsB(binsb)
  {
  }

  double Var2DMapper::Map(double va, double vb) const
  {
    // Since there are no overflow/underflow bins, check the range
    if(va < fBinsA.Min() || vb < fBinsB.Min()) return -1;
    if(va > fBinsA.Max() || vb > fBinsB.Max()) return fBinsA.NBins() * fBinsB.NBins();

    // FindBin uses root convention, first bin is bin 1, bin 0 is underflow
    const int ia = fBinsA.FindBin(va) - 1;
    const int ib = fBinsB.FindBin(vb) - 1;

    const int i = ia*fBinsB.NBins()+ib;

    return i+.5;
  }

  std::vector<double> Var2DMapper::Map(const std::vector<double>& vas,
                                       const std::vector<double>& vbs) const
  {
    assert(vas.size() == vbs.size());
    std::vector<double> ret;
    ret.reserve(vas.size());
    for(unsigned int i = 0; i < vas.size(); ++i)
      ret.push_back(Map(vas[i], vbs[i]));
    return ret;
  }

  /// Helper for 2D VarBase constructor
  class Var2DFunc : protected Var2DMapper
  {
  public:
    Var2DFunc(const VarBase& a, const Binning& binsa,
              const VarBase& b, const Binning& binsb)
      : Var2DMapper(binsa, binsb), fA(a), fB(b)
    {
    }

    double operator()(const void* rec) const
    {
      return Map(fA(rec), fB(rec));
    }

  protected:
    const VarBase fA, fB;
  };


  Var3DMapper::Var3DMapper(const Binning& binsa, const Binning& binsb, const Binning& binsc)
    : fBinsA(binsa), fBinsB(binsb), fBinsC(binsc)
  {
  }

  double Var3DMapper::Map(double va, double vb, double vc) const
  {
    /// Since there are no overflow/underflow bins, check the range
    if(va < fBinsA.Min() || vb < fBinsB.Min() || vc < fBinsC.Min()){
      return -1.0;
    }

    if(va > fBinsA.Max() || vb > fBinsB.Max() || vc > fBinsC.Max()){
      return fBinsA.NBins() * fBinsB.NBins() * fBinsC.NBins();
    }

    const int ia = fBinsA.FindBin(va) - 1;
    const int ib = fBinsB.FindBin(vb) - 1;
    const int ic = fBinsC.FindBin(vc) - 1;
    const int i = ia*fBinsB.NBins()*fBinsC.NBins() + ib*fBinsC.NBins() + ic;

    return i+.5;
  }

  std::vector<double> Var3DMapper::Map(const std::vector<double>& vas,
                                       const std::vector<double>& vbs,
                                       const std::vector<double>& vcs) const
  {
    assert(vas.size() == vbs.size() && vbs.size() == vcs.size());
    std::vector<double> ret;
    ret.reserve(vas.size());
    for(unsigned int i = 0; i < vas.size(); ++i)
      ret.push_back(Map(vas[i], vbs[i], vcs[i]));
    return ret;
  }

  /// Helper for 3D VarBase constructor
  class Var3DFunc : protected Var3DMapper
  {
  public:
    Var3DFunc(const VarBase& a, const Binning& binsa,
              const VarBase& b, const Binning& binsb,
              const VarBase& c, const Binning& binsc)
      : Var3DMapper(binsa, binsb, binsc), fA(a), fB(b),	fC(c)
    {
    }

    double operator()(const void* rec) const
    {
      return Map(fA(rec), fB(rec), fC(rec));
    }

  protected:
    const VarBase fA, fB, fC;
  };

  //----------------------------------------------------------------------
  VarBase::VarBase(const VarBase& a, const Binning& binsa,
                   const VarBase& b, const Binning& binsb)
    : VarBase(Var2DFunc(a, binsa, b, binsb))
  {
  }

  //----------------------------------------------------------------------
  VarBase::VarBase(const VarBase& a, const Binning& binsa,
                   const VarBase& b, const Binning& binsb,
                   const VarBase& c, const Binning& binsc)
    : VarBase(Var3DFunc(a, binsa, b, binsb, c, binsc))
  {
  }

  //----------------------------------------------------------------------
  VarBase VarBase::operator*(const VarBase& v) const
  {
    static std::map<std::pair<int, int>, int> ids;
    const std::pair<int, int> key(ID(), v.ID());

    struct Times
    {
      const VarBase a, b;
      double operator()(const void* rec) const {return a(rec) * b(rec);}
    };

    if(ids.count(key) == 0) ids[key] = fgNextID++;
    return VarBase(Times{*this, v});
  }

  //----------------------------------------------------------------------
  VarBase VarBase::operator/(const VarBase& v) const
  {
    static std::map<std::pair<int, int>, int> ids;
    const std::pair<int, int> key(ID(), v.ID());

    struct Divide
    {
      const VarBase a, b;
      double operator()(const void* rec) const
      {
        const double denom = b(rec);
        if(denom != 0)
          return a(rec) / denom;
        else
          return 0.0;
      }
    };

    if(ids.count(key) == 0) ids[key] = fgNextID++;
    return VarBase(Divide{*this, v}, ids[key]);
  }

  //----------------------------------------------------------------------
  VarBase VarBase::operator+(const VarBase& v) const
  {
    static std::map<std::pair<int, int>, int> ids;
    const std::pair<int, int> key(ID(), v.ID());

    struct Plus
    {
      const VarBase a, b;
      double operator()(const void* rec) const {return a(rec) + b(rec);}
    };

    if(ids.count(key) == 0) ids[key] = fgNextID++;
    return VarBase(Plus{*this, v}, ids[key]);
  }

  //----------------------------------------------------------------------
  VarBase VarBase::operator-(const VarBase& v) const
  {
    static std::map<std::pair<int, int>, int> ids;
    const std::pair<int, int> key(ID(), v.ID());

    struct Minus
    {
      const VarBase a, b;
      double operator()(const void* rec) const {return a(rec) - b(rec);}
    };

    if(ids.count(key) == 0) ids[key] = fgNextID++;
    return VarBase(Minus{*this, v}, ids[key]);
  }
} // namespace
