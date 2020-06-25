#include "stdafx.h"
#include "Source.h"

#include <cmath>
#include <sstream>

//--- Utility classes -----------------------------------------------------------------------
#include "utils/muGeneric.h"
#include "utils/utWideExceptions.h"

//-------------------------------------------------------------------------------------------
//
//
// Interface for different source types
//
//
//-------------------------------------------------------------------------------------------

ISource* ISource::CreateSource(const std::wstring& sType, const ISource::SData& src)
{
	if (sType == _T("LINEAR"))
	{
		return new SourceLin(src);
	}
	else if (sType == _T("INV"))
	{
		return new SourceInv(src);
	}
	else if (sType == _T("INV_SQR"))
	{
		return new SourceInvSqr(src);
	}
	else if (sType == _T("INV_QRT"))
	{
		return new SourceInvQrt(src);
	}
	else
	{
		std::wstringstream msg;
		msg << _T("Invalid source type identifier: \"") << sType << _T("\n");
		msg << _T("Valid values are \"LINEAR\" or \"INV\" or \"INV_SQUARE\".");
		throw utils::wruntime_error(msg.str());
	}
}

//-------------------------------------------------------------------------------------------
ISource::ISource(const ISource::SData& src)
	:m_src(src)
{}

//-------------------------------------------------------------------------------------------
ISource::~ISource()
{}

//-------------------------------------------------------------------------------------------
//
//
// Sources with F ~ 1/d²
//
//
//-------------------------------------------------------------------------------------------

SourceInvSqr::SourceInvSqr(const ISource::SData& src)
	:ISource(src)
{
	m_src.type = tpINV_SQR;
}

//-------------------------------------------------------------------------------------------
void SourceInvSqr::QueryForce(mu::vec2d_type& force, const mu::vec2d_type& r, double dist) const
{
	force[0] = (m_src.mult / mu::qubic(dist)) * r[0];
	force[1] = (m_src.mult / mu::qubic(dist)) * r[1];
}

//-------------------------------------------------------------------------------------------
double SourceInvSqr::GetPotential(double dist) const
{
	return m_src.mult / dist;  // ok
}

//-------------------------------------------------------------------------------------------
//
//
// Sources with F ~ 1/d³
//
//
//-------------------------------------------------------------------------------------------

SourceInvQrt::SourceInvQrt(const ISource::SData& src)
	:ISource(src)
{
	m_src.type = tpINV_QRT;
}

//-------------------------------------------------------------------------------------------
void SourceInvQrt::QueryForce(mu::vec2d_type& force, const mu::vec2d_type& r, double dist) const
{
	force[0] = (m_src.mult / mu::pow4(dist)) * r[0];
	force[1] = (m_src.mult / mu::pow4(dist)) * r[1];
}

//-------------------------------------------------------------------------------------------
double SourceInvQrt::GetPotential(double dist) const
{
	return m_src.mult / dist;  // ok
}

//-------------------------------------------------------------------------------------------
//
//
// Sources with F ~ 1/d
//
//
//-------------------------------------------------------------------------------------------

SourceInv::SourceInv(const ISource::SData& src)
	:ISource(src)
{
	m_src.type = tpINV;
}

//-------------------------------------------------------------------------------------------
void SourceInv::QueryForce(mu::vec2d_type& force, const mu::vec2d_type& r, double dist) const
{
	//----------------------------------------------------------
	// Calculate magnet Forces:
	//                _
	//      _         r
	//  m * a = k * -----
	//               |r²|
	//
	// Calculate distance pendulum to magnet i, The simulation is somewhat 3D
	// with the 3rd dimension introduced by the m_fHeight which is the height
	// of the pendulum plane above the magnet plane.
	force[0] = (m_src.mult / mu::sqr(dist)) * r[0];
	force[1] = (m_src.mult / mu::sqr(dist)) * r[1];
}

//-------------------------------------------------------------------------------------------
double SourceInv::GetPotential(double dist) const
{
	return m_src.mult * std::log(dist);   // ok
}

//-------------------------------------------------------------------------------------------
//
//
// Sources with F ~ d
//
//
//-------------------------------------------------------------------------------------------

SourceLin::SourceLin(const ISource::SData& src)
	:ISource(src)
{
	m_src.type = tpLIN;
}

//-------------------------------------------------------------------------------------------
void SourceLin::QueryForce(mu::vec2d_type& force, const mu::vec2d_type& r, double /*dist*/) const
{
	//----------------------------------------------------------
	// Calculate spring like sources (simulate pendulum)
	//                          _
	//         _                r         _   
	//     m * a = - k * |r| * --- = -k * r
	//                         |r|
	//  _  
	//  a  - Acceleration of the pendulum
	//  k  - elestic constant
	//  m  - Pendulum Mass (equals one here)
	//  _
	//  r  - Vector from Mountpoint to Pendulum mass 
	// |r| - distance Pendulum to Mountpoint
	force[0] = m_src.mult * r[0];
	force[1] = m_src.mult * r[1];  // ok
}

//-------------------------------------------------------------------------------------------
double SourceLin::GetPotential(double dist) const
{
	// spring potential:  U = 0.5 * k * x^2
	return 0.5 * m_src.mult * mu::sqr(dist);  // ok
}
