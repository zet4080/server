#include <platform.h>

#include "xerewards.h"
#include "study.h"

#include <kernel/unit.h>
#include <kernel/item.h>
#include <kernel/pool.h>
#include <kernel/region.h>

#include <tests.h>
#include <CuTest.h>

static void test_manacrystal(CuTest *tc) {
    test_setup();
    test_teardown();
}

static void test_skillpotion(CuTest *tc) {
    unit *u;
    const struct item_type *itype;
    skill* pSkill = NULL;
    int initialWeeks_Entertainment = 0;
    int initialWeeks_Stamina = 0;
    int initialWeeks_Magic = 0;

    test_setup();
    test_create_world();
    u = test_create_unit(test_create_faction(NULL), findregion(0, 0));
    itype = test_create_itemtype("skillpotion");
    change_resource(u, itype->rtype, 2);

    learn_skill(u, SK_ENTERTAINMENT, STUDYDAYS * u->number);
    pSkill = unit_skill(u, SK_ENTERTAINMENT);
    sk_set(pSkill, 5);
    initialWeeks_Entertainment = pSkill->weeks = 4;

    learn_skill(u, SK_STAMINA, STUDYDAYS * u->number);
    pSkill = unit_skill(u, SK_STAMINA);
    sk_set(pSkill, 5);
    initialWeeks_Stamina = pSkill->weeks = 4;

    learn_skill(u, SK_MAGIC, STUDYDAYS * u->number);
    pSkill = unit_skill(u, SK_MAGIC);
    sk_set(pSkill, 5);
    initialWeeks_Magic = pSkill->weeks = 4;

    CuAssertIntEquals(tc, 1, use_skillpotion(u, itype, 1, NULL));

    pSkill = unit_skill(u, SK_ENTERTAINMENT);
    CuAssertIntEquals(tc, initialWeeks_Entertainment - 3, pSkill->weeks);

    pSkill = unit_skill(u, SK_STAMINA);
    CuAssertIntEquals(tc, initialWeeks_Stamina - 3, pSkill->weeks);

    pSkill = unit_skill(u, SK_MAGIC);
    CuAssertIntEquals(tc, initialWeeks_Magic - 3, pSkill->weeks);

    test_teardown();
}

CuSuite *get_xerewards_suite(void)
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_manacrystal);
    SUITE_ADD_TEST(suite, test_skillpotion);
    return suite;
}

