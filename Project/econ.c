
/*
 *   Source [econ.c]
 */

#include "econ.h"
#include "rose.h"

#include <math.h>

#define LABOUR_SUPPLY_INCREMENT 0.1
#define PROPORTIONAL_INCREMENT 0.01

ECON_Market ECON_MarketInit(void)
{
	return (ECON_Market) {

		.labour_supply    = 8.0,
		.labour_income    = 1000.0,
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
}

const char* ECON_CommodityName(ECON_Commodity commodity)
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

double ECON_Utility(ECON_Market* market, ECON_Commodity commodity, double quantity_consumed)
{
	const double base    = market->constants.utility_base[commodity];
	const double dropoff = market->constants.utility_dropoff[commodity];

	return (base / dropoff) * (1.0 - (exp(-dropoff * quantity_consumed)));
}

double ECON_MarginalUtility(ECON_Market* market, ECON_Commodity commodity, double quantity_consumed)
{
	const double base    = market->constants.utility_base[commodity];
	const double dropoff = market->constants.utility_dropoff[commodity];

	return base * exp(-dropoff * quantity_consumed);
}

double ECON_DisUtilityLabour(ECON_Market* market, double quantity_labour)
{
	const double scale     = market->constants.disutility_scale;
	const double asymptote = market->constants.disutility_asymptote;

	return scale * ((-quantity_labour / asymptote) - log(asymptote - quantity_labour) + log(asymptote));
}

double ECON_MarginalDisUtilityLabour(ECON_Market* market, double quantity_labour)
{
	const double scale     = market->constants.disutility_scale;
	const double asymptote = market->constants.disutility_asymptote;

	return scale * ((-1.0 / (quantity_labour - asymptote)) - (1.0 / asymptote));
}

double ECON_Production(ECON_Market* market, ECON_Commodity commodity, double quantity_labour)
{
	const double scale = market->constants.production_scale[commodity];

	return (scale * sqrt(quantity_labour + 1.0)) - scale;
}

double ECON_MarginalProduction(ECON_Market* market, ECON_Commodity commodity, double quantity_labour)
{
	const double scale = market->constants.production_scale[commodity];

	return scale / (2.0 * sqrt(quantity_labour + 1.0));
}

void ECON_NormalizeAllocation(double* allocation)
{
	double total = 0.0;

	for (uInt32 commodity = 0; commodity < COMMODITY_COUNT; commodity++) {
		total += allocation[commodity];
	}

	for (uInt32 commodity = 0; commodity < COMMODITY_COUNT; commodity++) {
		allocation[commodity] /= total;
	}
}

void ECON_AdvanceSimulation(ECON_Market* market)
{
	for (uInt32 commodity = 0; commodity < COMMODITY_COUNT; commodity++) {
		market->labour_real_allocation[commodity] = market->labour_proportional_allocation[commodity] * market->labour_supply;
		market->budget_real_allocation[commodity] = market->budget_proportional_allocation[commodity] * market->labour_income;
	}

	for (uInt32 commodity = 0; commodity < COMMODITY_COUNT; commodity++) {
		double labour = market->labour_real_allocation[commodity];
		market->domestic_production[commodity] = ECON_Production(market, commodity, labour);
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
		market->marginal_utility[commodity] = ECON_MarginalUtility(market, commodity, consumed);
	}

	market->marginal_utility_spend = 0.0;
	for (uInt32 commodity = 0; commodity < COMMODITY_COUNT; commodity++) {
		double price = market->domestic_prices[commodity];
		double marginal_utility = market->marginal_utility[commodity];
		double allocation = market->budget_proportional_allocation[commodity];
		market->marginal_utility_spend += (marginal_utility / price) * allocation;
	}

	market->marginal_disutility_labour = ECON_MarginalDisUtilityLabour(market, market->labour_supply);
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
		ECON_NormalizeAllocation(market->budget_proportional_allocation);
	}

	{ // Adjust Labour Allocation
		uInt32 most_profitable_commodity = 0;
		double most_profit = 0.0;

		for (uInt32 commodity = 0; commodity < COMMODITY_COUNT; commodity++) {
			double quantity_labour = market->labour_real_allocation[commodity];
			double profitability = ECON_MarginalProduction(market, commodity, quantity_labour) * market->domestic_prices[commodity];

			if (profitability > most_profit) {
				most_profit = profitability;
				most_profitable_commodity = commodity;
			}
		}

		market->labour_proportional_allocation[most_profitable_commodity] += PROPORTIONAL_INCREMENT;
		ECON_NormalizeAllocation(market->labour_proportional_allocation);
	}
}

