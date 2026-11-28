
/*
 *   Source [main.c]
 */

#include "rose/rose.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define CLEAR_COLOR ROSE_COLOR_RGBA(70, 20, 80, 255)

#define LABOUR_SUPPLY_INCREMENT 0.1
#define PROPORTIONAL_INCREMENT 0.01

typedef enum Commodity {
	COMMODITY_FRUIT,
	COMMODITY_BREAD,
	COMMODITY_FISH,
	COMMODITY_COFFEE,
	COMMODITY_TEA,
	COMMODITY_TOBACCO,
	COMMODITY_WINE,
	COMMODITY_COUNT,
} Commodity;

typedef struct Constants {
	double utility_base     [COMMODITY_COUNT];
	double utility_dropoff  [COMMODITY_COUNT];
	double production_scale [COMMODITY_COUNT];
	double disutility_scale;
	double disutility_asymptote;
} Constants;

typedef struct Market {
	double labour_supply;
	double labour_income;
	double labour_wage_rate;
	
	double labour_proportional_allocation [COMMODITY_COUNT];
	double labour_real_allocation         [COMMODITY_COUNT];
	
	double budget_proportional_allocation [COMMODITY_COUNT];
	double budget_real_allocation         [COMMODITY_COUNT];
	
	double domestic_production  [COMMODITY_COUNT];
	double domestic_consumption [COMMODITY_COUNT];
	double domestic_prices      [COMMODITY_COUNT];

	double marginal_utility [COMMODITY_COUNT];
	double marginal_utility_spend;
	double marginal_disutility_labour;
	double marginal_disutility_earn;

	Constants constants;
} Market;

static Market market = {

	.labour_supply = 8.0,
	.labour_income = 1000.0,
	.labour_wage_rate = 0.0,

	.labour_proportional_allocation = {
		[COMMODITY_FRUIT]   = 1.0 / (double)COMMODITY_COUNT,
		[COMMODITY_BREAD]   = 1.0 / (double)COMMODITY_COUNT,
		[COMMODITY_FISH]    = 1.0 / (double)COMMODITY_COUNT,
		[COMMODITY_COFFEE]  = 1.0 / (double)COMMODITY_COUNT,
		[COMMODITY_TEA]     = 1.0 / (double)COMMODITY_COUNT,
		[COMMODITY_TOBACCO] = 1.0 / (double)COMMODITY_COUNT,
		[COMMODITY_WINE]    = 1.0 / (double)COMMODITY_COUNT,
	},

	.budget_proportional_allocation = {
		[COMMODITY_FRUIT]   = 1.0 / (double)COMMODITY_COUNT,
		[COMMODITY_BREAD]   = 1.0 / (double)COMMODITY_COUNT,
		[COMMODITY_FISH]    = 1.0 / (double)COMMODITY_COUNT,
		[COMMODITY_COFFEE]  = 1.0 / (double)COMMODITY_COUNT,
		[COMMODITY_TEA]     = 1.0 / (double)COMMODITY_COUNT,
		[COMMODITY_TOBACCO] = 1.0 / (double)COMMODITY_COUNT,
		[COMMODITY_WINE]    = 1.0 / (double)COMMODITY_COUNT,
	},

	.labour_real_allocation  = { 0.0 },
	.budget_real_allocation  = { 0.0 },

	.domestic_production  = { 0.0 },
	.domestic_consumption = { 0.0 },
	.domestic_prices      = { 0.0 },

	.marginal_utility           = { 0.0 },
	.marginal_utility_spend     = 0.0,
	.marginal_disutility_labour = 0.0,
	.marginal_disutility_earn   = 0.0,

	.constants = {
		.utility_base = {
			[COMMODITY_FRUIT]   = 2.0,
			[COMMODITY_BREAD]   = 2.5,
			[COMMODITY_FISH]    = 3.0,
			[COMMODITY_COFFEE]  = 3.5,
			[COMMODITY_TEA]     = 4.0,
			[COMMODITY_TOBACCO] = 4.5,
			[COMMODITY_WINE]    = 5.0,
		},
		.utility_dropoff = {
			[COMMODITY_FRUIT]   = 0.20,
			[COMMODITY_BREAD]   = 0.25,
			[COMMODITY_FISH]    = 0.30,
			[COMMODITY_COFFEE]  = 0.35,
			[COMMODITY_TEA]     = 0.40,
			[COMMODITY_TOBACCO] = 0.45,
			[COMMODITY_WINE]    = 0.50,
		},
		.production_scale = {
			[COMMODITY_FRUIT]   = 14.0,
			[COMMODITY_BREAD]   = 12.0,
			[COMMODITY_FISH]    = 10.0,
			[COMMODITY_COFFEE]  = 13.0,
			[COMMODITY_TEA]     = 15.0,
			[COMMODITY_TOBACCO] = 17.0,
			[COMMODITY_WINE]    = 19.0,
		},
		.disutility_scale     = 70.0,
		.disutility_asymptote = 16.0,
	},
};

const char* CommodityName(Commodity commodity)
{
	switch (commodity) {
		case (COMMODITY_FRUIT):   { return "FRUIT";   }
		case (COMMODITY_BREAD):   { return "BREAD";   }
		case (COMMODITY_FISH):    { return "FISH";    }
		case (COMMODITY_COFFEE):  { return "COFFEE";  }
		case (COMMODITY_TEA):     { return "TEA";     }
		case (COMMODITY_TOBACCO): { return "TOBACCO"; }
		case (COMMODITY_WINE):    { return "WINE";    }
		default: { return "UNKNOWN"; }
	}
}

double Utility(Market* market, Commodity commodity, double quantity_consumed)
{
	const double base    = market->constants.utility_base[commodity];
	const double dropoff = market->constants.utility_dropoff[commodity];

	return (base / dropoff) * (1.0 - (exp(-dropoff * quantity_consumed)));
}

double MarginalUtility(Market* market, Commodity commodity, double quantity_consumed)
{
	const double base    = market->constants.utility_base[commodity];
	const double dropoff = market->constants.utility_dropoff[commodity];

	return base * exp(-dropoff * quantity_consumed);
}

double DisUtilityLabour(Market* market, double quantity_labour)
{
	const double scale     = market->constants.disutility_scale;
	const double asymptote = market->constants.disutility_asymptote;

	return scale * ((-quantity_labour / asymptote) - log(asymptote - quantity_labour) + log(asymptote));
}

double MarginalDisUtilityLabour(Market* market, double quantity_labour)
{
	const double scale     = market->constants.disutility_scale;
	const double asymptote = market->constants.disutility_asymptote;

	return scale * ((-1.0 / (quantity_labour - asymptote)) - (1.0 / asymptote));
}

double Production(Market* market, Commodity commodity, double quantity_labour)
{
	const double scale = market->constants.production_scale[commodity];

	return (scale * sqrt(quantity_labour + 1.0)) - scale;
}

double MarginalProduction(Market* market, Commodity commodity, double quantity_labour)
{
	const double scale = market->constants.production_scale[commodity];

	return scale / (2.0 * sqrt(quantity_labour + 1.0));
}

void NormalizeAllocation(double* allocation)
{
	double total = 0.0;

	for (uInt32 commodity = 0; commodity < COMMODITY_COUNT; commodity++) {
		total += allocation[commodity];
	}

	for (uInt32 commodity = 0; commodity < COMMODITY_COUNT; commodity++) {
		allocation[commodity] /= total;
	}
}

void AdvanceSimulation(Market* market)
{
	for (uInt32 commodity = 0; commodity < COMMODITY_COUNT; commodity++) {
		market->labour_real_allocation[commodity] = market->labour_proportional_allocation[commodity] * market->labour_supply;
		market->budget_real_allocation[commodity] = market->budget_proportional_allocation[commodity] * market->labour_income;
	}

	for (uInt32 commodity = 0; commodity < COMMODITY_COUNT; commodity++) {
		double labour = market->labour_real_allocation[commodity];
		market->domestic_production[commodity] = Production(market, commodity, labour);
	}


	for (uInt32 commodity = 0; commodity < COMMODITY_COUNT; commodity++) {
		double supply = market->domestic_production[commodity];
		double demand = market->budget_real_allocation[commodity];
		market->domestic_prices[commodity] = demand / supply;
	}

	for (uInt32 commodity = 0; commodity < COMMODITY_COUNT; commodity++) {
		market->domestic_consumption[commodity] = market->domestic_production[commodity];
	}

	for (uInt32 commodity = 0; commodity < COMMODITY_COUNT; commodity++) {
		double consumed = market->domestic_consumption[commodity];
		market->marginal_utility[commodity] = MarginalUtility(market, commodity, consumed);
	}

	market->marginal_utility_spend = 0.0;
	for (uInt32 commodity = 0; commodity < COMMODITY_COUNT; commodity++) {
		double price = market->domestic_prices[commodity];
		double marginal_utility = market->marginal_utility[commodity];
		double allocation = market->budget_proportional_allocation[commodity];
		market->marginal_utility_spend += (marginal_utility / price) * allocation;
	}

	market->marginal_disutility_labour = MarginalDisUtilityLabour(market, market->labour_supply);
	market->labour_wage_rate = market->labour_income / market->labour_supply;
	market->marginal_disutility_earn = market->marginal_disutility_labour / market->labour_wage_rate;

	{ // Adjust Labour Supply
		double pain = market->marginal_disutility_earn;
		double gain = market->marginal_utility_spend;

		market->labour_supply +=
			(gain > pain) ?
			(LABOUR_SUPPLY_INCREMENT) :
			(-LABOUR_SUPPLY_INCREMENT);

		market->labour_supply = ROSE_Clamp(
			market->labour_supply,
			LABOUR_SUPPLY_INCREMENT,
			market->constants.disutility_asymptote - LABOUR_SUPPLY_INCREMENT
		);
	}

	{ // Adjust Budget Allocation
		uInt32 best_commodity = 0;
		double best_marginal_utility_per_price = 0.0;

		for (uInt32 commodity = 0; commodity < COMMODITY_COUNT; commodity++) {
			double marginal_utility_per_price =
				market->marginal_utility[commodity] /
				market->domestic_prices[commodity];

			if (marginal_utility_per_price > best_marginal_utility_per_price) {
				best_commodity = commodity;
				best_marginal_utility_per_price = marginal_utility_per_price;
			}
		}

		market->budget_proportional_allocation[best_commodity] += PROPORTIONAL_INCREMENT;
		NormalizeAllocation(market->budget_proportional_allocation);
	}

	{ // Adjust Labour Allocation
		uInt32 most_profitable_commodity = 0;
		double most_profit = 0.0;

		for (uInt32 commodity = 0; commodity < COMMODITY_COUNT; commodity++) {
			double quantity_labour = market->labour_real_allocation[commodity];
			double profitability = MarginalProduction(market, commodity, quantity_labour) * market->domestic_prices[commodity];

			if (profitability > most_profit) {
				most_profit = profitability;
				most_profitable_commodity = commodity;
			}
		}

		market->labour_proportional_allocation[most_profitable_commodity] += PROPORTIONAL_INCREMENT;
		NormalizeAllocation(market->labour_proportional_allocation);
	}
}

int main(void)
{
	ROSE_Init(L"My Window", 1200, 700);
	ROSE_WindowVSync(false);

	float value = 0;
	ROSE_Slider* slider = ROSE_SliderCreate((ROSE_Vec2){ 500.0f, 20.0f }, 0.0f, 1.0f, &value);

	ROSE_DrawInfo info = ROSE_DRAW_DEFAULT;
	info.position.x = 50.0f;
	info.position.y = 50.0f;

	double time = 0.0;

	while (true) {
		if (ROSE_WindowClosed())
			break;

		if (ROSE_WindowMinimized())
			continue;

		time = ROSE_Time();
		value = (sin(time) + 1.0) / 2.0;

		ROSE_FrameClear(CLEAR_COLOR);
		ROSE_SliderDraw(slider, info);
		ROSE_FrameRender();
	}

	ROSE_Quit();
}

