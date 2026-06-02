#include "solvers/SimpleBallisticSolver.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

// --------------

class SimpleBallisticSolver : public IBallisticSolver
{
public:
	void computeAmmoDrop(
		float& outFlightTime, float& outHorizontalDistance,
		const float attackSpeed,
		const float ammo_zPos,
		const AmmoParams& ammoParams) override;
};

// ---------------

void SimpleBallisticSolver::computeAmmoDrop(
	float& outFlightTime, float& outHorizontalDistance,
	const float attackSpeed,
	const float ammo_zPos,
	const AmmoParams& ammoParams)
{
	constexpr float g{ 9.81f };

	const float ammo_m = ammoParams.mass;
	const float ammo_d = ammoParams.drag;
	const float ammo_l = ammoParams.lift;

	//--- Flight time ---

	float ammo_fligthTime;
	{
		const float a = ammo_d * g * ammo_m - 2 * powf(ammo_d, 2) * ammo_l * attackSpeed;

		const float b = -3 * g * powf(ammo_m, 2) + 3 * ammo_d * ammo_l * ammo_m * attackSpeed;

		const float c = 6 * powf(ammo_m, 2) * ammo_zPos;

		//Solving

		const float p = -powf(b, 2) / (3 * powf(a, 2));

		const float q = 2 * powf(b, 3) / (27 * powf(a, 3)) + c / a;

		const float angle = acos(3 * q / (2 * p) * sqrt(-3 / p));

		ammo_fligthTime = 2 * sqrt(-p / 3) * cosf((angle + 4 * static_cast<float>(M_PI)) / 3) - b / (3 * a);
	}

	//--- Horizontal flight distance ---

	float h;
	{
		const float h_component1 = attackSpeed * ammo_fligthTime;

		const float h_component2 = powf(ammo_fligthTime, 2) * ammo_d * attackSpeed / (2 * ammo_m);

		const float h_component3 = powf(ammo_fligthTime, 3) * (6 * ammo_d * g * ammo_l * ammo_m - 6 * powf(ammo_d, 2) * (powf(ammo_l, 2) - 1) * attackSpeed) / (36 * powf(ammo_m, 2));

		const float h_component4 = powf(ammo_fligthTime, 4) * (-6 * powf(ammo_d, 2) * g * ammo_l * (1 + powf(ammo_l, 2) + powf(ammo_l, 4)) * ammo_m + 3 * powf(ammo_d, 3) * powf(ammo_l, 2) * (1 + powf(ammo_l, 2)) * attackSpeed + 6 * powf(ammo_d, 3) * powf(ammo_l, 4) * (1 + powf(ammo_l, 2)) * attackSpeed);

		const float h_component5 = powf(ammo_fligthTime, 5) * (3 * powf(ammo_d, 3) * g * powf(ammo_l, 3) * ammo_m - 3 * powf(ammo_d, 4) * powf(ammo_l, 2) * (1 + powf(ammo_l, 2)) * ammo_fligthTime) / (36 * (1 + powf(ammo_l, 2)) * powf(ammo_m, 4));

		h = h_component1 - h_component2 + h_component3 + h_component4 + h_component5;
	}

	outFlightTime = ammo_fligthTime;
	outHorizontalDistance = h;
}

// ---------------

std::unique_ptr<IBallisticSolver> createSimpleBallisticSolver() {
	return std::make_unique<SimpleBallisticSolver>();
}
