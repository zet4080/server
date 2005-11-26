/* vi: set ts=2:
 +-------------------+  Christian Schlittchen <corwin@amber.kn-bremen.de>
 |                   |  Enno Rehling <enno@eressea-pbem.de>
 | Eressea PBEM host |  Katja Zedel <katze@felidae.kn-bremen.de>
 | (c) 1998 - 2003   |  Henning Peters <faroul@beyond.kn-bremen.de>
 |                   |  Ingo Wilken <Ingo.Wilken@informatik.uni-oldenburg.de>
 +-------------------+  Stefan Reich <reich@halbling.de>

 This program may not be used, modified or distributed 
 without prior permission by the authors of Eressea.
*/

/*
 * Contains defines for the "alliance" vinyambar game (V3).
 * Include this file from settings.h to make eressea work.
 */
#define ENTERTAINFRACTION 20
#define IMMUN_GEGEN_ANGRIFF 8
#define RESOURCE_CONVERSION 1
#define RACE_ADJUSTMENTS 1
#define TEACHDIFFERENCE 2
#define PEASANT_ADJUSTMENT 1
#define GUARD_DISABLES_RECRUIT 1
#define GUARD_DISABLES_PRODUCTION 1
#define RESOURCE_QUANTITY 0.5
#define RECRUITFRACTION 40		/* 100/RECRUITFRACTION% */
#define NEWATSROI 0
#define COMBAT_TURNS 5
#define PEASANTS_DO_NOT_STARVE 0
#define NEW_MIGRATION 1

/* Vermehrungsrate Bauern in 1/10000.
* Evt. Berechnungsfehler, reale Vermehrungsraten scheinen h�her. */
#define PEASANTGROWTH		10
#define BATTLE_KILLS_PEASANTS 20
#define PEASANTLUCK			10

#if NEWATSROI == 1
#define ATSBONUS 2
#define ROIBONUS 4
#endif
