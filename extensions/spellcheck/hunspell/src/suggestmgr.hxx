/******* BEGIN LICENSE BLOCK *******
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 * 
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * 
 * The Initial Developers of the Original Code are Kevin Hendricks (MySpell)
 * and L�szl� N�meth (Hunspell). Portions created by the Initial Developers
 * are Copyright (C) 2002-2005 the Initial Developers. All Rights Reserved.
 * 
 * Contributor(s): Kevin Hendricks (kevin.hendricks@sympatico.ca)
 *                 David Einstein (deinst@world.std.com)
 *                 L�szl� N�meth (nemethl@gyorsposta.hu)
 *                 Davide Prina
 *                 Giuseppe Modugno
 *                 Gianluca Turconi
 *                 Simon Brouwer
 *                 Noll Janos
 *                 Biro Arpad
 *                 Goldman Eleonora
 *                 Sarlos Tamas
 *                 Bencsath Boldizsar
 *                 Halacsy Peter
 *                 Dvornik Laszlo
 *                 Gefferth Andras
 *                 Nagy Viktor
 *                 Varga Daniel
 *                 Chris Halls
 *                 Rene Engelhard
 *                 Bram Moolenaar
 *                 Dafydd Jones
 *                 Harri Pitkanen
 *                 Andras Timar
 *                 Tor Lillqvist
 * 
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 ******* END LICENSE BLOCK *******/

#ifndef _SUGGESTMGR_HXX_
#define _SUGGESTMGR_HXX_

#define MAXSWL 100
#define MAXSWUTF8L (MAXSWL * 4)
#define MAX_ROOTS 100
#define MAX_WORDS 100
#define MAX_GUESS 200
#define MAXNGRAMSUGS 4
#define MAXPHONSUGS 2

// timelimit: max ~1/4 sec (process time on Linux) for a time consuming function
#define TIMELIMIT (CLOCKS_PER_SEC >> 2)
#define MINTIMER 100
#define MAXPLUSTIMER 100

#define NGRAM_LONGER_WORSE  (1 << 0)
#define NGRAM_ANY_MISMATCH  (1 << 1)
#define NGRAM_LOWERING      (1 << 2)

#include "atypes.hxx"
#include "affixmgr.hxx"
#include "hashmgr.hxx"
#include "langnum.hxx"
#include <time.h>

enum { LCS_UP, LCS_LEFT, LCS_UPLEFT };

class SuggestMgr
{
  char *          ckey;
  int             ckeyl;
  w_char *        ckey_utf;

  char *          ctry;
  int             ctryl;
  w_char *        ctry_utf;

  AffixMgr*       pAMgr;
  int             maxSug;
  struct cs_info * csconv;
  int             utf8;
  int             langnum;
  int             nosplitsugs;
  int             maxngramsugs;
  int             complexprefixes;


public:
  SuggestMgr(const char * tryme, int maxn, AffixMgr *aptr);
  ~SuggestMgr();

  int suggest(char*** slst, const char * word, int nsug, int * onlycmpdsug);
  int ngsuggest(char ** wlst, char * word, int ns, HashMgr* pHMgr);
  int suggest_auto(char*** slst, const char * word, int nsug);
  int suggest_stems(char*** slst, const char * word, int nsug);
  int suggest_pos_stems(char*** slst, const char * word, int nsug);

  char * suggest_morph(const char * word);
  char * suggest_morph_for_spelling_error(const char * word);

private:
   int testsug(char** wlst, const char * candidate, int wl, int ns, int cpdsuggest,
     int * timer, clock_t * timelimit);
   int checkword(const char *, int, int, int *, clock_t *);
   int check_forbidden(const char *, int);

   int capchars(char **, const char *, int, int);
   int replchars(char**, const char *, int, int);
   int doubletwochars(char**, const char *, int, int);
   int forgotchar(char **, const char *, int, int);
   int swapchar(char **, const char *, int, int);
   int longswapchar(char **, const char *, int, int);
   int movechar(char **, const char *, int, int);
   int extrachar(char **, const char *, int, int);
   int badcharkey(char **, const char *, int, int);
   int badchar(char **, const char *, int, int);
   int twowords(char **, const char *, int, int);
   int fixstems(char **, const char *, int);

   int capchars_utf(char **, const w_char *, int wl, int, int);
   int doubletwochars_utf(char**, const w_char *, int wl, int, int);
   int forgotchar_utf(char**, const w_char *, int wl, int, int);
   int extrachar_utf(char**, const w_char *, int wl, int, int);
   int badcharkey_utf(char **, const w_char *, int wl, int, int);
   int badchar_utf(char **, const w_char *, int wl, int, int);
   int swapchar_utf(char **, const w_char *, int wl, int, int);
   int longswapchar_utf(char **, const w_char *, int, int, int);
   int movechar_utf(char **, const w_char *, int, int, int);

   int mapchars(char**, const char *, int, int);
   int map_related(const char *, int, char ** wlst, int, int, const mapentry*, int, int *, clock_t *);
   int map_related_utf(w_char *, int, int, int, char ** wlst, int, const mapentry*, int, int *, clock_t *);
   int ngram(int n, char * s1, const char * s2, int opt);
   int mystrlen(const char * word);
   int leftcommonsubstring(char * s1, const char * s2);
   int commoncharacterpositions(char * s1, const char * s2, int * is_swap);
   void bubblesort( char ** rwd, char ** rwd2, int * rsc, int n);
   void lcs(const char * s, const char * s2, int * l1, int * l2, char ** result);
   int lcslen(const char * s, const char* s2);

};

#endif
