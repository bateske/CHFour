#pragma GCC optimize("Os", "no-ipa-sra")
#include "Taunt.h"

namespace taunt {

// A kind's lines, '|' between them (at most eight of a kind).
struct Pool { const char *lines; uint8_t face; };

static const Pool POOL[KINDS] = {
    /* START */       {"WELCOME TO\nMY TABLE!|READY TO\nLOSE?|BACK FOR\nMORE?|I NEVER\nLOSE.\nMOSTLY.|"
                       "DON'T CRY\nWHEN I WIN.|PULL UP A\nCHAIR, KID.|LET'S MAKE\nTHIS QUICK.", TALK},
    /* WIN_NOW */     {"WATCH\nTHIS...|SAY\nGOODBYE!|AND THAT\nMAKES FOUR.|DRUM ROLL,\nPLEASE!|TOO EASY!|"
                       "FOUR! COUNT\n'EM. FOUR!|LIGHTS OUT!", SMILE},
    /* GIFT */        {"FOR ME? YOU\nSHOULDN'T\nHAVE!|OH, THANK\nYOU KINDLY!|A PRESENT!\nHOW SWEET.|"
                       "YOU'RE TOO\nKIND!", SMILE},
    /* UNBLOCKED */   {"YOU FORGOT\nTO BLOCK!|DIDN'T SEE\nTHAT, DID\nYOU?|YOO-HOO!\nOVER HERE!|"
                       "HELLO? ROW\nOF THREE\nHERE!", SMILE},
    /* FORCED */      {"IT'S OVER IN\n# MOVES.\nI COUNTED.|# MORE MOVES\nAND YOU'RE\nTOAST!|"
                       "I SEE THE\nFUTURE. IT'S\nALL MINE.|TICK TOCK,\nPAL.|I'VE DONE\nTHE MATH.\nYOU LOSE.", SMILE},
    /* MUST_BLOCK */  {"NICE TRY,\nPAL.|NOT ON MY\nWATCH!|I SAW THAT\nCOMING.|BLOCKED!\nNEXT?|DENIED!|"
                       "OH NO YOU\nDON'T!|SNEAKY.\nBUT NO.", RAISED},
    /* MISSED */      {"HA! YOU\nMISSED IT!|YOU HAD ME!\n...AND NOW\nYOU DON'T.|PHEW! I\nMEAN... HA!|"
                       "PSST. YOU\nHAD A WIN.\nOOPS!", SMILE},
    /* BLOCKED */     {"HEY! THAT\nWAS MINE!|RATS!|OH, COME\nON!|WHO TAUGHT\nYOU THAT?|SPOILSPORT!|"
                       "I HAD PLANS\nFOR THAT!|HMPH!", ANGRY},
    /* DOUBLE */      {"TWO WAYS?\nNO FAIR!|UH-OH.|WAIT, WAIT,\nWAIT...|THAT'S NOT\nALLOWED!\n...IS IT?|MOMMY!", SURPRISED},
    /* LOSING */      {"I MEANT TO\nDO THAT.|IS IT HOT\nIN HERE?|THIS TABLE\nIS WOBBLY!|BEGINNER'S\nLUCK!|"
                       "I'M JUST\nWARMING UP!|THE SUN WAS\nIN MY EYES.", ANGRY},
    /* TRAP */        {"TWO WAYS TO\nWIN. PICK\nONE!|CHECK AND\nMATE! WAIT,\nWRONG GAME.|GOTCHA!|"
                       "LEFT OR\nRIGHT? HEH\nHEH HEH.", SMILE},
    /* THREAT */      {"BETTER\nBLOCK THAT!|SEE IT?\nI DO.|YOUR MOVE,\nSMARTY.|THREE IN A\nROW. HINT,\nHINT.", RAISED},
    /* OPEN_CENTRE */ {"THE MIDDLE?\nHOW\nORIGINAL.|A CLASSIC.\nWON'T HELP.", RAISED},
    /* OPEN_EDGE */   {"THE EDGE?\nBOLD. WRONG,\nBUT BOLD.|OH, THIS\nWILL BE\nQUICK.", SMILE},
    /* AHEAD */       {"I LIKE MY\nCHANCES.|MWA-HA-HA!|KEEP GOING.\nIT'S CUTE.|LA LA LA...|IS THIS\nYOUR FIRST\nGAME?", SMILE},
    /* BEHIND */      {"HMM.|DON'T GET\nCOCKY, KID.|LUCKY\nGUESS.|A FLUKE!|I'M LETTING\nYOU WIN.", RAISED},
    /* BANTER */      {"CARRY THE\nONE...|EENY,\nMEENY...|I PRACTICE\nIN THE\nMIRROR.|"
                       "NICE DISC.\nSHAME ABOUT\nTHE MOVE.|MY DISCS\nARE\nSHINIER.|YOUR MOVE,\nCHAMP.", TALK},
    /* IDLE */        {"ANY DAY\nNOW...|I'M GETTING\nOLDER HERE.|ZZZ...", TALK},
    /* DRAWISH */     {"A TIE? I\nDON'T DO\nTIES.", RAISED},
    /* HE_LOST */     {"BEST OF\nTHREE?|I LET YOU\nWIN!|REMATCH!\nRIGHT NOW!|I WASN'T\nREADY!|IMPOSSIBLE!|"
                       "YOU CHEATED\n...DIDN'T\nYOU?", SURPRISED},
    /* HE_WON */      {"I AM THE\nGREATEST!|BETTER LUCK\nNEXT TIME,\nKID!|TOO GOOD!\nTOO GOOD!|"
                       "WHO'S THE\nBOSS? ME!|UNDEFEATED!\n...TODAY.|BOW TO THE\nMASTER!", SMILE},
    /* DRAWN */       {"LET'S CALL\nIT MY WIN.|A TIE?\nBORING!", RAISED},
    /* P2_START */    {"PLAY NICE,\nYOU TWO.|MAY THE\nBEST ONE\nWIN. ME.", TALK},
    /* P2_MISSED */   {"OOH! MISSED\nA WIN\nTHERE.|SO CLOSE!\nSO BLIND!", SMILE},
    /* P2_DOUBLE */   {"TWO WAYS!\nI LIKE YOUR\nSTYLE.|OOH, SNEAKY!", SURPRISED},
    /* P2_BLOCK */    {"DENIED!|GOOD EYE.|NOT TODAY!", RAISED},
    /* P2_THREAT */   {"CAREFUL\nNOW...|ONE MORE\nAND IT'S\nOVER!|I SEE\nTHREE...", RAISED},
    /* P2_BANTER */   {"I'D HAVE\nWON BY NOW.|POPCORN,\nANYONE?|AMATEURS.\nADORABLE.", TALK},
    /* P2_WON */      {"NOT BAD,\nKID.|I TAUGHT\nTHEM THAT.|I COULD\nBEAT YOU\nBOTH.", SMILE},
};

static uint8_t used[KINDS];

void reset() {
    for (auto &u : used) u = 0;
}

static uint8_t lines(const char *s) {
    uint8_t n = 1;
    for (; *s; s++) n = (uint8_t)(n + (*s == '|'));
    return n;
}

static void copy(const char *s, uint8_t i, char *out, uint8_t number) {
    while (i) i = (uint8_t)(i - (*s++ == '|'));
    uint8_t k = 0;
    for (; *s && *s != '|' && k < LINE_MAX - 1; s++) out[k++] = *s == '#' ? (char)('0' + number % 10) : *s;
    out[k] = 0;
}

uint8_t pick(uint8_t kind, c4::Rng &rng, char *out, uint8_t number) {
    const Pool &p = POOL[kind];
    uint8_t n = lines(p.lines), all = (uint8_t)((1u << n) - 1);
    if ((used[kind] & all) == all) used[kind] = 0;
    // The k-th line not yet used.
    uint8_t left = 0;
    for (uint8_t i = 0; i < n; i++) left = (uint8_t)(left + !(used[kind] & (1 << i)));
    uint8_t k = rng.below(left), i = 0;
    for (;; i++) {
        if (used[kind] & (1 << i)) continue;
        if (!k--) break;
    }
    used[kind] |= (uint8_t)(1 << i);
    copy(p.lines, i, out, number);
    return p.face;
}

bool line(uint8_t kind, uint8_t i, char *out) {
    if (kind >= KINDS || i >= lines(POOL[kind].lines)) return false;
    copy(POOL[kind].lines, i, out, 3);
    return true;
}

}  // namespace taunt
