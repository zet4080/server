/* vi: set ts=2:
 *
 *
 * Eressea PB(E)M host Copyright (C) 1998-2000
 *      Christian Schlittchen (corwin@amber.kn-bremen.de)
 *      Katja Zedel (katze@felidae.kn-bremen.de)
 *      Henning Peters (faroul@beyond.kn-bremen.de)
 *      Enno Rehling (enno@eressea-pbem.de)
 *      Ingo Wilken (Ingo.Wilken@informatik.uni-oldenburg.de)
 *
 * This program may not be used, modified or distributed without
 * prior permission by the authors of Eressea.
 */

struct plane;
struct attrib;
struct unit;
struct faction;
struct region;
struct faction_list;

typedef struct alliance {
	struct alliance * next;
	struct faction_list * members;
	unsigned int flags;
	int id;
	char * name;
} alliance;

extern alliance * alliances;
extern alliance * findalliance(int id);
extern alliance * makealliance(int id, const char * name);
extern const char * alliancename(const struct alliance * al);
extern void setalliance(struct faction * f, alliance * al);

extern void alliancejoin(void);
extern void alliancekick(void);
extern void alliancevictory(void);
/* execute commands */

