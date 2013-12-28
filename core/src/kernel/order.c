/* vi: set ts=2:
 +-------------------+  
 |                   |  Christian Schlittchen <corwin@amber.kn-bremen.de>
 | Eressea PBEM host |  Enno Rehling <enno@eressea.de>
 | (c) 1998 - 2004   |  Katja Zedel <katze@felidae.kn-bremen.de>
 |                   |
 +-------------------+  

 This program may not be used, modified or distributed
 without prior permission by the authors of Eressea.
*/

#include <platform.h>
#include <kernel/config.h>
#include "order.h"

#include "skill.h"

#include <util/base36.h>
#include <util/bsdstring.h>
#include <util/language.h>
#include <util/log.h>
#include <util/parser.h>

/* libc includes */
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

# define ORD_KEYWORD(ord) (ord)->data->_keyword
# define ORD_LOCALE(ord) locale_array[(ord)->data->_lindex]->lang
# define ORD_STRING(ord) (ord)->data->_str

typedef struct locale_data {
  struct order_data *short_orders[MAXKEYWORDS];
  struct order_data *study_orders[MAXSKILLS];
  const struct locale *lang;
} locale_data;

static struct locale_data *locale_array[16];
static int nlocales = 0;

typedef struct order_data {
  char *_str;
  int _refcount:20;
  int _lindex:4;
  keyword_t _keyword;
} order_data;

static void release_data(order_data * data)
{
  if (data) {
    if (--data->_refcount == 0) {
      if (data->_str)
        free(data->_str);
      free(data);
    }
  }
}

void replace_order(order ** dlist, order * orig, const order * src)
{
  while (*dlist != NULL) {
    order *dst = *dlist;
    if (dst->data == orig->data) {
      order *cpy = copy_order(src);
      *dlist = cpy;
      cpy->next = dst->next;
      dst->next = 0;
      free_order(dst);
    }
    dlist = &(*dlist)->next;
  }
}

keyword_t get_keyword(const order * ord)
{
  if (ord == NULL) {
    return NOKEYWORD;
  }
  return ORD_KEYWORD(ord);
}

/** returns a plain-text representation of the order.
 * This is the inverse function to the parse_order command. Note that
 * keywords are expanded to their full length.
 */
static char *get_command(const order * ord, char *sbuffer, size_t size)
{
  char *bufp = sbuffer;
  const char *text = ORD_STRING(ord);
  keyword_t kwd = ORD_KEYWORD(ord);
  int bytes;

  if (ord->_persistent) {
    if (size > 0) {
      *bufp++ = '@';
      --size;
    } else {
      WARN_STATIC_BUFFER();
    }
  }
  if (kwd != NOKEYWORD) {
    const struct locale *lang = ORD_LOCALE(ord);
    if (size > 0) {
      if (text)
        --size;
      bytes = (int)strlcpy(bufp, (const char *)LOC(lang, keywords[kwd]), size);
      if (wrptr(&bufp, &size, bytes) != 0)
        WARN_STATIC_BUFFER();
      if (text)
        *bufp++ = ' ';
    } else {
      WARN_STATIC_BUFFER();
    }
  }
  if (text) {
    bytes = (int)strlcpy(bufp, (const char *)text, size);
    if (wrptr(&bufp, &size, bytes) != 0) {
      WARN_STATIC_BUFFER();
      if (bufp - sbuffer >= 6) {
        bufp -= 6;
        while (bufp > sbuffer && (*bufp & 0x80) != 0) {
          ++size;
          --bufp;
        }
        memcpy(bufp, "[...]", 6);   /* TODO: make sure this only happens in eval_command */
        bufp += 6;
      }
    }
  }
  if (size > 0)
    *bufp = 0;
  return sbuffer;
}

char *getcommand(const order * ord)
{
  char sbuffer[DISPLAYSIZE * 2];
  return strdup(get_command(ord, sbuffer, sizeof(sbuffer)));
}

void free_order(order * ord)
{
  if (ord != NULL) {
    assert(ord->next == 0);

    release_data(ord->data);
    free(ord);
  }
}

order *copy_order(const order * src)
{
  if (src != NULL) {
    order *ord = (order *) malloc(sizeof(order));
    ord->next = NULL;
    ord->_persistent = src->_persistent;
    ord->data = src->data;
    ++ord->data->_refcount;
    return ord;
  }
  return NULL;
}

void set_order(struct order **destp, struct order *src)
{
  if (*destp == src)
    return;
  free_order(*destp);
  *destp = src;
}

void free_orders(order ** olist)
{
  while (*olist) {
    order *ord = *olist;
    *olist = ord->next;
    ord->next = NULL;
    free_order(ord);
  }
}

static order_data *create_data(keyword_t kwd, const char *sptr, int lindex)
{
  const char *s = sptr;
  order_data *data;
  const struct locale *lang = locale_array[lindex]->lang;

  if (kwd != NOKEYWORD)
    s = (*sptr) ? sptr : NULL;

  /* learning, only one order_data per skill required */
  if (kwd == K_STUDY) {
    skill_t sk = findskill(parse_token(&sptr), lang);
    switch (sk) {
    case NOSKILL:              /* fehler */
      break;
    case SK_MAGIC:             /* kann parameter haben */
      if (*sptr != 0)
        break;
    default:                   /* nur skill als Parameter, keine extras */
      data = locale_array[lindex]->study_orders[sk];
      if (data == NULL) {
        const char *skname = skillname(sk, lang);
        data = (order_data *) malloc(sizeof(order_data));
        locale_array[lindex]->study_orders[sk] = data;
        data->_keyword = kwd;
        data->_lindex = lindex;
        if (strchr(skname, ' ') != NULL) {
          size_t len = strlen(skname);
          data->_str = malloc(len + 3);
          data->_str[0] = '\"';
          memcpy(data->_str + 1, skname, len);
          data->_str[len + 1] = '\"';
          data->_str[len + 2] = '\0';
        } else {
          data->_str = strdup(skname);
        }
        data->_refcount = 1;
      }
      ++data->_refcount;
      return data;
    }
  }

  /* orders with no parameter, only one order_data per order required */
  else if (kwd != NOKEYWORD && *sptr == 0) {
    data = locale_array[lindex]->short_orders[kwd];
    if (data == NULL) {
      data = (order_data *) malloc(sizeof(order_data));
      locale_array[lindex]->short_orders[kwd] = data;
      data->_keyword = kwd;
      data->_lindex = lindex;
      data->_str = NULL;
      data->_refcount = 1;
    }
    ++data->_refcount;
    return data;
  }
  data = (order_data *) malloc(sizeof(order_data));
  data->_keyword = kwd;
  data->_lindex = lindex;
  data->_str = s ? strdup(s) : NULL;
  data->_refcount = 1;
  return data;
}

static order *create_order_i(keyword_t kwd, const char *sptr, int persistent,
  const struct locale *lang)
{
  order *ord = NULL;
  int lindex;

  /* if this is just nonsense, then we skip it. */
  if (lomem) {
    switch (kwd) {
    case K_KOMMENTAR:
    case NOKEYWORD:
      return NULL;
    default:
      break;
    }
  }

  for (lindex = 0; lindex != nlocales; ++lindex) {
    if (locale_array[lindex]->lang == lang)
      break;
  }
  if (lindex == nlocales) {
    locale_array[nlocales] = (locale_data *) calloc(1, sizeof(locale_data));
    locale_array[nlocales]->lang = lang;
    ++nlocales;
  }

  ord = (order *) malloc(sizeof(order));
  ord->_persistent = persistent;
  ord->next = NULL;

  ord->data = create_data(kwd, sptr, lindex);

  return ord;
}

order *create_order(keyword_t kwd, const struct locale * lang,
  const char *params, ...)
{
  char zBuffer[DISPLAYSIZE];
  if (params) {
    char *bufp = zBuffer;
    int bytes;
    size_t size = sizeof(zBuffer) - 1;
    va_list marker;

    va_start(marker, params);
    while (*params) {
      if (*params == '%') {
        int i;
        const char *s;
        ++params;
        switch (*params) {
        case 's':
          s = va_arg(marker, const char *);
          bytes = (int)strlcpy(bufp, s, size);
          if (wrptr(&bufp, &size, bytes) != 0)
            WARN_STATIC_BUFFER();
          break;
        case 'd':
          i = va_arg(marker, int);
          bytes = (int)strlcpy(bufp, itoa10(i), size);
          if (wrptr(&bufp, &size, bytes) != 0)
            WARN_STATIC_BUFFER();
          break;
        case 'i':
          i = va_arg(marker, int);
          bytes = (int)strlcpy(bufp, itoa36(i), size);
          if (wrptr(&bufp, &size, bytes) != 0)
            WARN_STATIC_BUFFER();
          break;
        default:
          assert(!"unknown format-character in create_order");
        }
      } else if (size > 0) {
        *bufp++ = *params;
        --size;
      }
      ++params;
    }
    va_end(marker);
    *bufp = 0;
  } else {
    zBuffer[0] = 0;
  }
  return create_order_i(kwd, zBuffer, 0, lang);
}

order *parse_order(const char *s, const struct locale * lang)
{
  while (*s && !isalnum(*(unsigned char *)s) && !ispunct(*(unsigned char *)s))
    ++s;
  if (*s != 0) {
    keyword_t kwd;
    const char *sptr;
    int persistent = 0;

    while (*s == '@') {
      persistent = 1;
      ++s;
    }
    sptr = s;
    kwd = findkeyword(parse_token(&sptr), lang);
    if (kwd != NOKEYWORD) {
      while (isxspace(*(unsigned char *)sptr))
        ++sptr;
      s = sptr;
    }
    return create_order_i(kwd, s, persistent, lang);
  }
  return NULL;
}

/**
 * Returns true if the order qualifies as "repeated". An order is repeated if it will overwrite the
 * old default order. K_BUY is in this category, but not K_MOVE.
 *
 * \param ord An order.
 * \return true if the order is long
 * \sa is_exclusive(), is_repeated(), is_persistent()
 */
bool is_repeated(const order * ord)
{
  keyword_t kwd = ORD_KEYWORD(ord);
  const struct locale *lang = ORD_LOCALE(ord);
  const char * s;
  int result = 0;

  switch (kwd) {
  case K_CAST:
  case K_BUY:
  case K_SELL:
  case K_ROUTE:
  case K_DRIVE:
  case K_WORK:
  case K_BESIEGE:
  case K_ENTERTAIN:
  case K_TAX:
  case K_RESEARCH:
  case K_SPY:
  case K_STEAL:
  case K_SABOTAGE:
  case K_STUDY:
  case K_TEACH:
  case K_BREED:
  case K_PIRACY:
  case K_PLANT:
    result = 1;
    break;

  case K_FOLLOW:
    /* FOLLOW is only a long order if we are following a ship. */
    parser_pushstate();
    init_tokens(ord);
    skip_token();
    s = getstrtoken();
    result = isparam(s, lang, P_SHIP);
    parser_popstate();
    break;

  case K_MAKE:
    /* Falls wir MACHE TEMP haben, ignorieren wir es. Alle anderen
     * Arten von MACHE zaehlen aber als neue defaults und werden
     * behandelt wie die anderen (deswegen kein break nach case
     * K_MAKE) - und in thisorder (der aktuelle 30-Tage Befehl)
     * abgespeichert). */
    parser_pushstate();
    init_tokens(ord);           /* initialize token-parser */
    skip_token();
    s = getstrtoken();
    result = !isparam(s, lang, P_TEMP);
    parser_popstate();
    break;
  default:
    result = 0;
  }
  return result;
}

/**
 * Returns true if the order qualifies as "exclusive". An order is exclusive if it makes all other
 * long orders illegal. K_MOVE is in this category, but not K_BUY.
 *
 * \param ord An order.
 * \return true if the order is long
 * \sa is_exclusive(), is_repeated(), is_persistent()
 */
bool is_exclusive(const order * ord)
{
  keyword_t kwd = ORD_KEYWORD(ord);
  const struct locale *lang = ORD_LOCALE(ord);
  int result = 0;

  switch (kwd) {
  case K_MOVE:
  case K_ROUTE:
  case K_DRIVE:
  case K_WORK:
  case K_BESIEGE:
  case K_ENTERTAIN:
  case K_TAX:
  case K_RESEARCH:
  case K_SPY:
  case K_STEAL:
  case K_SABOTAGE:
  case K_STUDY:
  case K_TEACH:
  case K_BREED:
  case K_PIRACY:
  case K_PLANT:
    result = 1;
    break;

  case K_FOLLOW:
    /* FOLLOW is only a long order if we are following a ship. */
    parser_pushstate();
    init_tokens(ord);
    skip_token();
    result = isparam(getstrtoken(), lang, P_SHIP);
    parser_popstate();
    break;

  case K_MAKE:
    /* Falls wir MACHE TEMP haben, ignorieren wir es. Alle anderen
     * Arten von MACHE zaehlen aber als neue defaults und werden
     * behandelt wie die anderen (deswegen kein break nach case
     * K_MAKE) - und in thisorder (der aktuelle 30-Tage Befehl)
     * abgespeichert). */
    parser_pushstate();
    init_tokens(ord);           /* initialize token-parser */
    skip_token();
    result = !isparam(getstrtoken(), lang, P_TEMP);
    parser_popstate();
    break;
  default:
    result = 0;
  }
  return result;
}

/**
 * Returns true if the order qualifies as "long". An order is long if it excludes most other long
 * orders.
 *
 * \param ord An order.
 * \return true if the order is long
 * \sa is_exclusive(), is_repeated(), is_persistent()
 */
bool is_long(const order * ord)
{
  keyword_t kwd = ORD_KEYWORD(ord);
  const struct locale *lang = ORD_LOCALE(ord);
  bool result = false;

  switch (kwd) {
  case K_CAST:
  case K_BUY:
  case K_SELL:
  case K_MOVE:
  case K_ROUTE:
  case K_DRIVE:
  case K_WORK:
  case K_BESIEGE:
  case K_ENTERTAIN:
  case K_TAX:
  case K_RESEARCH:
  case K_SPY:
  case K_STEAL:
  case K_SABOTAGE:
  case K_STUDY:
  case K_TEACH:
  case K_BREED:
  case K_PIRACY:
  case K_PLANT:
    return true;

  case K_FOLLOW:
    /* FOLLOW is only a long order if we are following a ship. */
    parser_pushstate();
    init_tokens(ord);
    skip_token();
    result = isparam(getstrtoken(), lang, P_SHIP);
    parser_popstate();
    break;

  case K_MAKE:
    /* Falls wir MACHE TEMP haben, ignorieren wir es. Alle anderen
     * Arten von MACHE zaehlen aber als neue defaults und werden
     * behandelt wie die anderen (deswegen kein break nach case
     * K_MAKE) - und in thisorder (der aktuelle 30-Tage Befehl)
     * abgespeichert). */
    parser_pushstate();
    init_tokens(ord);           /* initialize token-parser */
    skip_token();
    result = !isparam(getstrtoken(), lang, P_TEMP);
    parser_popstate();
    break;
  default:
    result = false;
  }
  return result;
}

/**
 * Returns true if the order qualifies as "persistent". An order is persistent if it will be
 * included in the template orders. @-orders, comments and most long orders are in this category,
 * but not K_MOVE.
 *
 * \param ord An order.
 * \return true if the order is persistent
 * \sa is_exclusive(), is_repeated(), is_persistent()
 */
bool is_persistent(const order * ord)
{
  keyword_t kwd = ORD_KEYWORD(ord);
  int persist = ord->_persistent != 0;
  switch (kwd) {
  case K_MOVE:
  case NOKEYWORD:
    /* lang, aber niemals persistent! */
    return false;

  case K_KOMMENTAR:
    return true;

  default:
    return persist || is_repeated(ord);
  }

}

char *write_order(const order * ord, char *buffer, size_t size)
{
  if (ord == 0) {
    buffer[0] = 0;
  } else {
    keyword_t kwd = ORD_KEYWORD(ord);
    if (kwd == NOKEYWORD) {
      const char *text = ORD_STRING(ord);
      strlcpy(buffer, (const char *)text, size);
    } else {
      get_command(ord, buffer, size);
    }
  }
  return buffer;
}

void push_order(order ** ordp, order * ord)
{
  while (*ordp)
    ordp = &(*ordp)->next;
  *ordp = ord;
}

void init_tokens(const struct order *ord)
{
  char *cmd = getcommand(ord);
  init_tokens_str(cmd, cmd);
}