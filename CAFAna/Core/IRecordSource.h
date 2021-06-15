#pragma once

#include "CAFAna/Core/IRecordSink.h"

namespace ana
{
  class IValueSource;

  template<class RecT> class _IRecordSource
  {
  public:
    virtual ~_IRecordSource() {}

    // TODO make all of these classes non-copyable

    virtual void Register(_IRecordSink<RecT>* sink)
    {
      fSinks.push_back(sink);
    }

    IValueSource& GetVar(const _Var<RecT>& var);

    template<class SpillT> IValueSource& GetCut(const _Cut<RecT, SpillT>& cut);

    IValueSource& operator[](const _Var<RecT>& var)
    {
      return GetVar(var);
    }

  protected:
    std::vector<_IRecordSink<RecT>*> fSinks;
  };
};


#include "CAFAna/Core/VarApplier.h"
#include "CAFAna/Core/CutApplier.h"

namespace ana
{
  template<class RecT> IValueSource&
  _IRecordSource<RecT>::GetVar(const _Var<RecT>& var)
  {
     // TODO look up in a map by ID
     _VarApplier<RecT>* ret = new _VarApplier<RecT>(var);
     Register(ret);
     return *ret;
  }

  template<class RecT> template<class SpillT> IValueSource&
  _IRecordSource<RecT>::GetCut(const _Cut<RecT, SpillT>& cut)
  {
     // TODO look up in a map by ID
     _CutApplier<RecT, SpillT>* ret = new _CutApplier<RecT, SpillT>(cut);
     Register(ret);
     return *ret;
  }
}