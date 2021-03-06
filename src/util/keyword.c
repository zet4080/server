#include <platform.h>
#include <kernel/config.h>
#include "keyword.h"

#include <util/language.h>
#include <util/umlaut.h>
#include <util/log.h>
#include <util/strings.h>

#include <critbit.h>

#include <assert.h>
#include <string.h>

const char * keyword(keyword_t kwd)
{
    static char result[32]; /* FIXME: static return value */
    if (kwd==NOKEYWORD || keywords[kwd] == NULL) {
        return NULL;
    }
    if (!result[0]) {
        strcpy(result, "keyword::");
    }
    str_strlcpy(result + 9, keywords[kwd], sizeof(result) - 9);
    return result;
}

static const char * keyword_key(int kwd) {
    assert(kwd < MAXKEYWORDS && kwd >= 0);
    return keyword((keyword_t)kwd);
}

void init_keyword(const struct locale *lang, keyword_t kwd, const char *str) {
    void **tokens = get_translations(lang, UT_KEYWORDS);
    struct critbit_tree **cb = (critbit_tree **)tokens;
    add_translation(cb, str, (int)kwd);
}

void init_keywords(const struct locale *lang) {
    init_translations(lang, UT_KEYWORDS, keyword_key, MAXKEYWORDS);
}

keyword_t findkeyword(const char *s) {
    int i;
    for (i = 0; i != MAXKEYWORDS; ++i) {
        if (keywords[i] && (strcmp(s, keywords[i]) == 0)) {
            return (keyword_t)i;
        }
    }
    return NOKEYWORD;
}

keyword_t get_keyword(const char *s, const struct locale *lang) {
    keyword_t result = NOKEYWORD;

    assert(lang);
    assert(s);
    while (*s == '@') ++s;

    if (*s) {
        char buffer[64];
        char *str = transliterate(buffer, sizeof(buffer) - sizeof(int), s);

        if (str && str[0] && str[1]) {
            int i;
            void *match;
            void **tokens = get_translations(lang, UT_KEYWORDS);
            critbit_tree *cb = (critbit_tree *)*tokens;
            if (cb && cb_find_prefix(cb, str, strlen(str), &match, 1, 0)) {
                cb_get_kv(match, &i, sizeof(int));
                result = (keyword_t)i;
                return keyword_disabled(result) ? NOKEYWORD : result;
            }
        }
    }
    return NOKEYWORD;
}

static bool disabled_kwd[MAXKEYWORDS];

void enable_keyword(keyword_t kwd, bool enabled) {
    assert(kwd < MAXKEYWORDS);
    disabled_kwd[kwd] = !enabled;
}

bool keyword_disabled(keyword_t kwd) {
    assert(kwd < MAXKEYWORDS);
    return disabled_kwd[kwd];
}

const char *keyword_name(keyword_t kwd, const struct locale *lang)
{
    return LOC(lang, mkname("keyword", keywords[kwd]));
}

const char *keywords[MAXKEYWORDS] = {
    "//",
    "banner",
    "work",
    "attack",
    "steal",
    NULL,
    "name",
    "use",
    "describe",
    "enter",
    "guard",
    "message",
    "end",
    "ride",
    "number",
    "follow",
    "research",
    "give",
    "help",
    "combat",
    "ready",
    "buy",
    "contact",
    "teach",
    "study",
    "make",
    "maketemp",
    "move",
    "password",
    "recruit",
    "reserve",
    "route",
    "sabotage",
    "option",
    "spy",
    "quit",
    "hide",
    "carry",
    "tax",
    "entertain",
    "sell",
    "leave",
    "forget",
    "cast",
    "show",
    "destroy",
    "plant",
    "grow",
    "default",
    "origin",
    "email",
    "piracy",
    "group",
    "sort",
    "prefix",
    "alliance",
    "claim",
    "promote",
    "pay",
    "loot",
    "autostudy",
    "locale",
};

