#include "solvers/TableBallisticSolver.hpp"
#include "CommonTypes.hpp"

#include <string>
#include <fstream>
#define _USE_MATH_DEFINES
#include <math.h>

// --------------

class TableBallisticSolver : public IBallisticSolver
{
public:
	TableBallisticSolver(const std::string& tableFileName) {
		load(tableFileName);
	}

	void computeAmmoDrop(
		float& outFlightTime, float& outHorizontalDistance,
		const float attackSpeed,
		const float ammo_zPos,
		const AmmoParams& ammoParams) override;

private:
	bool load(const std::string& path);

	struct Result {
        float t;
        float hDist;
    };

    size_t index(int iz, int iv, int im, int id, int il) const;
    const Result& at(int iz, int iv, int im, int id, int il) const;

	Result lerp(const Result& a, const Result& b, float t) const;

	struct Interp {
		int lo;
		float frac;
	};
	
	Interp findInterp(float val, const std::vector<float>& axis) const;

	TableBallisticSolver::Result lookup(
		float Z0, float V0, float m, float d,  float l) const;

    std::vector<float> axisZ0;
    std::vector<float> axisV0;
    std::vector<float> axisM;
    std::vector<float> axisD;
    std::vector<float> axisL;
  
    std::vector<Result> data;
};

// ---------------

void TableBallisticSolver::computeAmmoDrop(
	float& outFlightTime, float& outHorizontalDistance,
	const float attackSpeed,
	const float ammo_zPos,
	const AmmoParams& ammoParams)
{
	const Result result = lookup(
    	ammo_zPos, attackSpeed,
		ammoParams.mass, ammoParams.drag,  ammoParams.lift);

	outFlightTime = result.t;
	outHorizontalDistance = result.hDist;
}

bool TableBallisticSolver::load(const std::string& path) {
	std::ifstream file(path);
	if (!file.is_open()) return false;

	int nZ, nV, nM, nD, nL;
	file >> nZ >> nV >> nM >> nD >> nL;

	axisZ0.resize(nZ);
	for (auto& v : axisZ0) file >> v;
	
	axisV0.resize(nV);
	for (auto& v : axisV0) file >> v;
	
	axisM.resize(nM);
	for (auto& v : axisM)  file >> v;
	
	axisD.resize(nD);
	for (auto& v : axisD)  file >> v;
	
	axisL.resize(nL);
	for (auto& v : axisL)  file >> v;

	const size_t total = (size_t)(nZ * nV * nM * nD * nL);
	data.resize(total);

	for (size_t i = 0; i < total; i++)
		file >> data[i].t >> data[i].hDist;

	return file.good();
}

size_t TableBallisticSolver::index(int iz, int iv, int im, int id, int il) const {
	return ((((size_t)iz * axisV0.size() + iv)
							* axisM.size()  + im)
							* axisD.size()  + id)
							* axisL.size()  + il;
}

const TableBallisticSolver::Result& TableBallisticSolver::at(
	int iz, int iv, int im, int id, int il) const
{
	return data[index(iz, iv, im, id, il)];
}

TableBallisticSolver::Result TableBallisticSolver::lerp(const Result& a, const Result& b, float t) const
{
	return {
		a.t     + (b.t     - a.t)     * t,
		a.hDist + (b.hDist - a.hDist) * t
	};
}

TableBallisticSolver::Interp TableBallisticSolver::findInterp(float val, const std::vector<float>& axis) const
{
	if (val <= axis.front()) return {0, 0.0f};
	if (val >= axis.back())
		return {(int)axis.size()-2, 1.0f};

	auto it = std::lower_bound(
		axis.begin(), axis.end(), val);
	int i = (int)(it - axis.begin()) - 1;
	if (i < 0) i = 0;

	float frac = (val - axis[i])
			/ (axis[i+1] - axis[i]);
	return {i, frac};
}

TableBallisticSolver::Result TableBallisticSolver::lookup(
    float Z0, float V0, float m, float d,  float l) const
{
    Interp iz = findInterp(Z0, axisZ0);
    Interp iv = findInterp(V0, axisV0);
    Interp im = findInterp(m,  axisM);
    Interp id = findInterp(d,  axisD);
    Interp il = findInterp(l,  axisL);
 
    // 2^5 = 32 вершини гіперкуба
    // Згортаємо: 32 → 16 → 8 → 4 → 2 → 1
 
    // l: 32 → 16
    Result v[16];
    for (int a = 0; a < 2; a++)
     for (int b = 0; b < 2; b++)
      for (int c = 0; c < 2; c++)
       for (int e = 0; e < 2; e++) {
           auto& lo = at(iz.lo+a, iv.lo+b,
                         im.lo+c, id.lo+e, il.lo);
           auto& hi = at(iz.lo+a, iv.lo+b,
                         im.lo+c, id.lo+e, il.lo+1);
           v[a*8+b*4+c*2+e] = lerp(lo, hi, il.frac);
       }
 
    // d: 16 → 8
    Result w[8];
    for (int a = 0; a < 2; a++)
     for (int b = 0; b < 2; b++)
      for (int c = 0; c < 2; c++)
       w[a*4+b*2+c] = lerp(v[a*8+b*4+c*2],
                            v[a*8+b*4+c*2+1],
                            id.frac);
 
    // m: 8 → 4
    Result u[4];
    for (int a = 0; a < 2; a++)
     for (int b = 0; b < 2; b++)
      u[a*2+b] = lerp(w[a*4+b*2],
                       w[a*4+b*2+1], im.frac);
 
    // V0: 4 → 2
    Result s[2];
    for (int a = 0; a < 2; a++)
        s[a] = lerp(u[a*2], u[a*2+1], iv.frac);
 
    // Z0: 2 → 1
    return lerp(s[0], s[1], iz.frac);
}

// ---------------

std::unique_ptr<IBallisticSolver> createTableBallisticSolver(const std::string& tableFileName) {
	return std::make_unique<TableBallisticSolver>(tableFileName);
}
