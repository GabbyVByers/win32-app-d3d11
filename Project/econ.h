
/*
 *   Header [econ.h]
 */

#ifndef ECON_HEADER_GUARD
#define ECON_HEADER_GUARD

typedef enum ECON_Commodity {
	COMMODITY_FRUIT,
	COMMODITY_BREAD,
	COMMODITY_FISH,
	COMMODITY_COFFEE,
	COMMODITY_TEA,
	COMMODITY_TOBACCO,
	COMMODITY_WINE,
	COMMODITY_COUNT,
} ECON_Commodity;

typedef struct Constants {
	double utility_base     [COMMODITY_COUNT];
	double utility_dropoff  [COMMODITY_COUNT];
	double production_scale [COMMODITY_COUNT];
	double disutility_scale;
	double disutility_asymptote;
} ECON_Constants;

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

	ECON_Constants constants;
} ECON_Market;

ECON_Market ECON_MarketInit(void);
const char* ECON_CommodityName(ECON_Commodity commodity);

double ECON_Utility(ECON_Market* market, ECON_Commodity commodity, double quantity_consumed);
double ECON_MarginalUtility(ECON_Market* market, ECON_Commodity commodity, double quantity_consumed);
double ECON_DisUtilityLabour(ECON_Market* market, double quantity_labour);
double ECON_MarginalDisUtilityLabour(ECON_Market* market, double quantity_labour);
double ECON_Production(ECON_Market* market, ECON_Commodity commodity, double quantity_labour);
double ECON_MarginalProduction(ECON_Market* market, ECON_Commodity commodity, double quantity_labour);

void ECON_NormalizeAllocation(double* allocation);
void ECON_AdvanceSimulation(ECON_Market* market);

#endif /* ECON_HEADER_GUARD */

