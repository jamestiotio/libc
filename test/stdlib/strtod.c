/*
 * Copyright © 2017 Embedded Artistry LLC.
 * License: MIT. See LICENSE file for details.
 */

#include "stdlib_tests.h"
#include "test/ulpsDistance.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

// Cmocka needs these
// clang-format off
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
// clang-format on

// TODO: "strtod INFINITY tests are not enabled"

#pragma mark - Defintions -

#define length(x) (sizeof(x) / sizeof *(x))

static struct
{
	const char* s;
	double f;
} t[] = {
	{"0", 0.0},
	{"00.00", 0.0},
	{"-.00000", -0.0},
	/*TODO: Enable {"1e+1000000", INFINITY},*/
	{"1e-1000000", 0},
	// 2^-1074 * 0.5 - eps
	{".2470328229206232720882843964341106861825299013071623822127928412503377536351043e-323", 0},
	// 2^-1074 * 0.5 + eps
	{".2470328229206232720882843964341106861825299013071623822127928412503377536351044e-323",
	 0x1p-1074},
	// 2^-1074 * 1.5 - eps
	{".7410984687618698162648531893023320585475897039214871466383785237510132609053131e-323",
	 0x1p-1074},
	// 2^-1074 * 1.5 + eps
	{".7410984687618698162648531893023320585475897039214871466383785237510132609053132e-323",
	 0x1p-1073},
	// 2^-1022 + 2^-1075 - eps
	{".2225073858507201630123055637955676152503612414573018013083228724049586647606759e-307",
	 0x1p-1022},
	// 2^-1022 + 2^-1075 + eps
	{".2225073858507201630123055637955676152503612414573018013083228724049586647606760e-307",
	 0x1.0000000000001p-1022},
	// 2^1024 - 2^970 - eps
	{"17976931348623158079372897140530341507993413271003782693617377898044"
	 "49682927647509466490179775872070963302864166928879109465555478519404"
	 "02630657488671505820681908902000708383676273854845817711531764475730"
	 "27006985557136695962284291481986083493647529271907416844436551070434"
	 "2711559699508093042880177904174497791.999999999999999999999999999999",
	 0x1.fffffffffffffp1023},
	// 2^1024 - 2^970
	/*TODO: enable {"17976931348623158079372897140530341507993413271003782693617377898044"
	"49682927647509466490179775872070963302864166928879109465555478519404"
	"02630657488671505820681908902000708383676273854845817711531764475730"
	"27006985557136695962284291481986083493647529271907416844436551070434"
	 "2711559699508093042880177904174497792", INFINITY},*/
	// some random numbers
	{".5961860348131807091861002266453941950428e00", 0.59618603481318067}, // 0x1.313f4bc3b584cp-1
	{"1.815013169218038729887460898733526957442e-1", 0.18150131692180388}, // 0x1.73b6f662e1712p-3
	{"42.07082357534453600681618685682257590772e-2", 0.42070823575344535}, // 0x1.aece23c6e028dp-2
	{"665.4686306516261456328973225579833470816e-3", 0.66546863065162609}, // 0x1.54b84dea53453p-1
	{"6101.852922970868621786690495485449831753e-4", 0.61018529229708685}, // 0x1.386a34e5d516bp-1
	{"76966.95208236968077849464348875471158549e-5", 0.76966952082369677}, // 0x1.8a121f9954dfap-1
	{"250506.5322228682496132604807222923702304e-6", 0.25050653222286823}, // 0x1.0084c8cd538c2p-2
	{"2740037.230228005325852424697698331177377e-7", 0.27400372302280052}, // 0x1.18946e9575ef4p-2
	{"20723093.50049742645941529268715428324490e-8", 0.20723093500497428}, // 0x1.a868b14486e4dp-3
	{"0.7900280238081604956226011047460238748912e1", 7.9002802380816046}, // 0x1.f99e3100f2eaep+2
	{"0.9822860653737296848190558448760465863597e2", 98.228606537372968}, // 0x1.88ea17d506accp+6
	{"0.7468949723190370809405570560160405324869e3", 746.89497231903704}, // 0x1.75728e73f48b7p+9
	{"0.1630268320282728475980459844271031751665e4", 1630.2683202827284}, // 0x1.97912c28d5cbp+10
	{"0.4637168629719170695109918769645492022088e5", 46371.686297191707}, // 0x1.6a475f6258737p+15
	{"0.6537805944497711554209461686415872067523e6", 653780.59444977110}, // 0x1.3f3a9305bb86cp+19
	{"0.2346324356502437045212230713960457676531e6", 234632.43565024371}, // 0x1.ca4437c3631eap+17
	{"0.9709481716420048341897258980454298205278e8", 97094817.164200485}, // 0x1.7263284a8242cp+26
	{"0.4996908522051874110779982354932499499602e9", 499690852.20518744}, // 0x1.dc8ad6434872ap+28
};

#pragma mark - Declarations -

static const double epsilon = (double)0.000000000000001;

#pragma mark - Private Functions -

static void strtod_test(void** state)
{
	char* p;

	for(size_t i = 0; i < length(t); i++)
	{
		double x = strtod(t[i].s, &p);
		int64_t distance = ulpsDistanceDouble(x, t[i].f);
		// We check both ULP and an absolute epsilon because ULP is
		// failing when close to zero, so we fall back to an absolute
		// epsilon comparison in that case.
		printf("Distance: %ld, fabs: %ld\n", distance, fabs(x - t[i].f));
		assert_true((distance < 3) || (fabs(x - t[i].f) < epsilon));
	}
}

#pragma mark - Public Functions -

int strtod_tests(void)
{
	const struct CMUnitTest strtod_tests[] = {cmocka_unit_test(strtod_test)};

	return cmocka_run_group_tests(strtod_tests, NULL, NULL);
}
