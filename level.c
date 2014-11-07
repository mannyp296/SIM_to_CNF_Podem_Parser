/**********************************************************************/
/*           This is the levelling routine for atpg                   */
/*                                                                    */
/*           Author: Tony Hi-Keung Ma                                 */
/*           last update : 1/27/1987                                  */
/**********************************************************************/


#include <stdio.h>
#include "global.h"
#include "miscell.h"


void
level_circuit()
{
    register wptr w,wfirst,wtemp;
    register nptr nfirst,nlast,ncurrent,nprelast;
    register int wire_index,i,j,k;
    register int schedule,level = 0;

    nfirst = NIL(struct NODE);
    nlast = NIL(struct NODE);
    wfirst = NIL(struct WIRE);
    sort_wlist = podemALLOC(ncktwire,wptr);

    /* build up the initial node event list */
    for (wire_index = 0; wire_index < ncktin; wire_index++) {
        sort_wlist[wire_index] = cktin[wire_index];
        cktin[wire_index]->wlist_index = wire_index;
        cktin[wire_index]->flag |= MARKED;
        cktin[wire_index]->level = level;
        for (i = 0; i < cktin[wire_index]->nout; i++) {
            if (!(cktin[wire_index]->onode[i]->pnext) &&
                cktin[wire_index]->onode[i] != nlast) {
                cktin[wire_index]->onode[i]->pnext = nfirst;
                nfirst = cktin[wire_index]->onode[i];
            }
            if (!nlast) nlast = nfirst;
        }
    }
    nprelast = nlast;

    while(nfirst) {
        ncurrent = nfirst;
        nfirst = nfirst->pnext;
        ncurrent->pnext = NIL(struct NODE);
        schedule = TRUE;
        for (i = 0; i < ncurrent->nin; i++) {
            if (!(ncurrent->iwire[i]->flag & MARKED)) {
                schedule = FALSE;
            }
        }
        if (schedule) {
            for (j = 0; j < ncurrent->nout; j++) {
                sort_wlist[wire_index] = ncurrent->owire[j];
                ncurrent->owire[j]->wlist_index = wire_index;
                wire_index++;
                ncurrent->owire[j]->pnext = wfirst;
                wfirst = ncurrent->owire[j];
                for (k = 0; k < ncurrent->owire[j]->nout; k++) {
                    if (!(ncurrent->owire[j]->onode[k]->pnext) &&
                        ncurrent->owire[j]->onode[k] != nlast) {
                        nlast->pnext = ncurrent->owire[j]->onode[k];
                        nlast = ncurrent->owire[j]->onode[k];
                        if (!nfirst) nfirst = nlast;
                    }
                }
            }
        }
        else {
            if (!(ncurrent->pnext) && ncurrent != nlast) {
                nlast->pnext = ncurrent;
                nlast = ncurrent;
            }
            if (!nfirst) nfirst = nlast;
        }
        if (ncurrent == nprelast) {
            level++;
            for (w = wfirst; w; w = wtemp) {
                wtemp = w->pnext;
                w->pnext = NIL(struct WIRE);
                w->flag |= MARKED;
                w->level = level;
            }
            wfirst = NIL(struct WIRE);
            nprelast = nlast;
        }
    }
    /* unmark each wire */
    for (i = 0; i < ncktwire; i++) {
        sort_wlist[i]->flag &= ~MARKED;
    }
}/* end of level */


void
rearrange_gate_inputs()
{
    register int i,j,k;
    register wptr wtemp;
    register nptr n;

    for (i = ncktin; i < ncktwire; i++) {
        if (n = sort_wlist[i]->inode[0]) {
            for (j = 0; j < n->nin; j++) {
                for (k = j + 1; k < n->nin; k++) {
                    if (n->iwire[j]->level > n->iwire[k]->level) {
                        wtemp = n->iwire[j];
                        n->iwire[j] = n->iwire[k];
                        n->iwire[k] = wtemp;
                    }
                }
            }
        }
    }
}/* end of rearrange_gate_inputs */
