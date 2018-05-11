#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <cstdio>
#include <vector>

using namespace std;
using namespace glm;

class RigidBody {
	// force / torque function format
public:
	typedef vec3(*Function) (
		double, // time of application
		vec3,
		quat, // position
		vec3,
		vec3,
		mat3,
		vec3,
		vec3
		);
	RigidBody(double m, mat3 inertia, Function force, Function torque)
	{
		this->m = m;
		this->m_inertia = inertia;
		this->m_force = force;
		this->m_torque = torque;
	}
	// Runge - Kutta fourth order differential equation solver
	void Update(double t, double dt) {
		double halfdt = 0.5 * dt, sixthdt = dt / 6.0;
		double tphalfdt = t + halfdt, tpdt = t + dt;
		vec3 XN, PN, LN, VN, WN;
		quat QN;
		mat3 RN;
		// A1 = G(t,S0), B1 = S0 + (dt / 2) * A1
		vec3 A1DXDT = m_V;
		quat A1DQDT = 0.5 * m_W * m_Q;
		vec3 A1DPDT = m_force(t, m_X, m_Q, m_P, m_L, m_R, m_V, m_W);
		vec3 A1DLDT = m_torque(t, m_X, m_Q, m_P, m_L, m_R, m_V, m_W);
		XN = m_X + halfdt * A1DXDT;
		QN.x = m_Q.x + halfdt * A1DQDT.x;
		QN.y = m_Q.y + halfdt * A1DQDT.y;
		QN.z = m_Q.z + halfdt * A1DQDT.z;
		QN.w = m_Q.w + halfdt * A1DQDT.w;
		PN = m_P + halfdt * A1DPDT;
		LN = m_L + halfdt * A1DLDT;
		Convert(QN, PN, LN, RN, VN, WN);
		// A2 = G(t + dt / 2,B1), B2 = S0 + (dt / 2) * A2
		vec3 A2DXDT = VN;
		quat A2DQDT = 0.5 * WN * QN;
		vec3 A2DPDT = m_force(tphalfdt, XN, QN, PN, LN, RN, VN, WN);
		vec3 A2DLDT = m_torque(tphalfdt, XN, QN, PN, LN, RN, VN, WN);
		XN = m_X + halfdt * A2DXDT;
		QN.x = m_Q.x + halfdt * A2DQDT.x;
		QN.y = m_Q.y + halfdt * A2DQDT.y;
		QN.z = m_Q.z + halfdt * A2DQDT.z;
		QN.w = m_Q.w + halfdt * A2DQDT.w;
		PN = m_P + halfdt * A2DPDT;
		LN = m_L + halfdt * A2DLDT;
		Convert(QN, PN, LN, RN, VN, WN);
		// A3 = G(t + dt / 2,B2), B3 = S0 + dt * A3
		vec3 A3DXDT = VN;
		quat A3DQDT = 0.5 * WN * QN;
		vec3 A3DPDT = m_force(tphalfdt, XN, QN, PN, LN, RN, VN, WN);
		vec3 A3DLDT = m_torque(tphalfdt, XN, QN, PN, LN, RN, VN, WN);
		XN = m_X + dt * A3DXDT;
		QN.x = m_Q.x + dt * A3DQDT.x;
		QN.y = m_Q.y + dt * A3DQDT.y;
		QN.z = m_Q.z + dt * A3DQDT.z;
		QN.w = m_Q.w + dt * A3DQDT.w;
		PN = m_P + dt * A3DPDT;
		LN = m_L + dt * A3DLDT;
		Convert(QN, PN, LN, RN, VN, WN);
		vec3 A4DXDT = VN;
		quat A4DQDT = 0.5 * WN * QN;
		vec3 A4DPDT = m_force(tpdt, XN, QN, PN, LN, RN, VN, WN);
		vec3 A4DLDT = m_torque(tpdt, XN, QN, PN, LN, RN, VN, WN);
		m_X = m_X + sixthdt * (A1DXDT + 2.0*(A2DXDT + A3DXDT) + A4DXDT);
		m_Q.x = m_Q.x + sixthdt * (A1DQDT.x + 2.0*(A2DQDT.x + A3DQDT.x) + A4DQDT.x);
		m_Q.y = m_Q.y + sixthdt * (A1DQDT.y + 2.0*(A2DQDT.y + A3DQDT.y) + A4DQDT.y);
		m_Q.z = m_Q.z + sixthdt * (A1DQDT.z + 2.0*(A2DQDT.z + A3DQDT.z) + A4DQDT.z);
		m_Q.w = m_Q.w + sixthdt * (A1DQDT.w + 2.0*(A2DQDT.w + A3DQDT.w) + A4DQDT.w);
		m_P = m_P + sixthdt * (A1DPDT + 2.0*(A2DPDT + A3DPDT) + A4DPDT);
		m_L = m_L + sixthdt * (A1DLDT + 2.0*(A2DLDT + A3DLDT) + A4DLDT);
		Convert(m_Q, m_P, m_L, m_R, m_V, m_W);
	}
	void Initialize(double m, mat3 inertia, Function force, Function torque)
	{
		this->m = m;
		this->m_inertia = inertia;
		this->m_force = force;
		this->m_torque = torque;
	}
protected:
	// convert (Q,P,L) to (R,V,W)
	void Convert(quat Q, vec3 P, vec3 L, mat3 & R, vec3 & V, vec3 & W) const {
		R = mat3_cast(Q);
		V = m_invMass * P;
		W = R * m_invInertia * transpose(R)*L;
	}
	// constant quantities
	double m_mass, m_invMass,m;
	mat3 m_inertia, m_invInertia;
	// state variables
	vec3 m_X; // position
	quat m_Q; // orientation
	vec3 m_P; // linear momentum
	vec3 m_L; // angular momentum
			  // derived state variables
	mat3 m_R; // orientation matrix
	vec3 m_V; // linear velocity vector
	vec3 m_W; // angular velocity
			  // force and torque functions
	Function m_force; Function m_torque;
};



