#pragma once

//-------------------------------------------------------------------------------------------
#include <string>
#include <memory>
#include "utils/muVector.h"
#include "utils/utMemory.h"


//-------------------------------------------------------------------------------------------
class ISource
{
public:
  enum EType
  {
    tpLIN,
    tpINV,
    tpINV_SQR
  };

  struct SData
  {
    mu::vec2d_type pos;
    double size;
    double mult;
    int r, g, b; 
    EType type;
  };

  static ISource* CreateSource(const std::wstring &sType, const ISource::SData &src);
  virtual ~ISource();

  virtual inline void QueryForce(mu::vec2d_type &force, const mu::vec2d_type &pos, double dist) const = 0;
  virtual inline double GetPotential(double dist) const = 0;

  int GetRed()   const { return m_src.r; };
  int GetGreen() const { return m_src.g; };
  int GetBlue()  const { return m_src.b; };
  double GetSize() const { return m_src.size; };
  EType GetType()  const { return m_src.type; };

  const mu::vec2d_type& GetPos() const { return m_src.pos; };

protected:
  SData m_src;
  ISource(const SData &src);
};

//-------------------------------------------------------------------------------------------
class SourceInvSqr : public ISource
{
  friend class ISource;

public:
  virtual void QueryForce(mu::vec2d_type &force, const mu::vec2d_type &r, double dist) const;
  virtual double GetPotential(double dist) const;

private:
  SourceInvSqr(const ISource::SData &src);
};

//-------------------------------------------------------------------------------------------
class SourceInv : public ISource
{
  friend class ISource;

public:
  virtual void QueryForce(mu::vec2d_type &force, const mu::vec2d_type &r, double dist) const;
  virtual double GetPotential(double dist) const;
  
private:
  SourceInv(const ISource::SData &src);
};

//-------------------------------------------------------------------------------------------
class SourceLin : public ISource
{
  friend class ISource;

public:
  virtual void QueryForce(mu::vec2d_type &force, const mu::vec2d_type &r, double dist) const;
  virtual double GetPotential(double dist) const;

private:
  SourceLin(const ISource::SData &src);
};