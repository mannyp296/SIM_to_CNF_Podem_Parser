/**********************************************************************/
/*           This is the podem test pattern generator for atpg        */
/*                                                                    */
/*           Author: Hi-Keung Tony Ma                                 */
/*           last update : 07/11/1988                                 */
/**********************************************************************/

#include <stdio.h>
#include "global.h"
#include "miscell.h"

#define U  2
#define D  3
#define B  4
#define CONFLICT 2

extern int backtrack_limit;
extern int total_attempt_num;
int no_of_backtracks;
int find_test;
int no_test;
int unique_imply;

podem(fault, current_backtracks)
fptr fault;
int *current_backtracks;
{
    register int i,j;
    register nptr n;
    register wptr wpi,decision_tree,wtemp,wfault;
    wptr test_possible();
    wptr fault_evaluate();
    long rand();
    int set_uniquely_implied_value();
    int attempt_num = 0;
    void display_fault();

    /* initialize all circuit wires to unknown */
    for (i = 0; i < ncktwire; i++) {
        sort_wlist[i]->value = U;
    }
    no_of_backtracks = 0;
    find_test = FALSE;
    no_test = FALSE;
    decision_tree = NIL(struct WIRE);
    wfault = NIL(struct WIRE);
    mark_propagate_tree(fault->node);
    switch (set_uniquely_implied_value(fault)) {
        case TRUE:
            sim();
            if (wfault = fault_evaluate(fault)) forward_imply(wfault);
            if (check_test()) find_test = TRUE;
            break;
        case CONFLICT:
            no_test = TRUE;
            break;
        case FALSE: break;
    }

    while ((no_of_backtracks < backtrack_limit) && !no_test &&
        !(find_test && (attempt_num == total_attempt_num))) {
        if (wpi = test_possible(fault)) {
            wpi->flag |= CHANGED;
            wpi->pnext = decision_tree;
            decision_tree = wpi;
        }
        else {
            while (decision_tree && !wpi) {
                if (decision_tree->flag & ALL_ASSIGNED) {
                    decision_tree->flag &= ~ALL_ASSIGNED;
                    decision_tree->value = U;
                    decision_tree->flag |= CHANGED;
                    wtemp = decision_tree;
                    decision_tree = decision_tree->pnext;
                    wtemp->pnext = NIL(struct WIRE);
                }
                else {
                    decision_tree->value = decision_tree->value ^ 1;
                    decision_tree->flag |= CHANGED;
                    decision_tree->flag |= ALL_ASSIGNED;
                    no_of_backtracks++;
                    wpi = decision_tree;
                }
            }
            if (!wpi) no_test = TRUE;
        }
again:  if (wpi) {
            sim();
            if (wfault = fault_evaluate(fault)) forward_imply(wfault);
            if (check_test()) {
                find_test = TRUE;
                if (total_attempt_num > 1) {
                    if (attempt_num == 0) {
                        display_fault(fault);
                    }
                    display_io();
                }
                attempt_num++;
                if (total_attempt_num > attempt_num) {
                    wpi = NIL(struct WIRE);
                    while (decision_tree && !wpi) {
                        if (decision_tree->flag & ALL_ASSIGNED) {
                            decision_tree->flag &= ~ALL_ASSIGNED;
                            decision_tree->value = U;
                            decision_tree->flag |= CHANGED;
                            wtemp = decision_tree;
                            decision_tree = decision_tree->pnext;
                            wtemp->pnext = NIL(struct WIRE);
                        }
                        else {
                            decision_tree->value = decision_tree->value ^ 1;
                            decision_tree->flag |= CHANGED;
                            decision_tree->flag |= ALL_ASSIGNED;
                            no_of_backtracks++;
                            wpi = decision_tree;
                        }
                    }
                    if (!wpi) no_test = TRUE;
                    goto again;
                }
            }
        }
    }
    for (wpi = decision_tree; wpi; wpi = wtemp) {
         wtemp = wpi->pnext;
         wpi->pnext = NIL(struct WIRE);
         wpi->flag &= ~ALL_ASSIGNED;
    }
    *current_backtracks = no_of_backtracks;
    unmark_propagate_tree(fault->node);
    if (find_test) {
        if (total_attempt_num == 1) {
               for (i = 0; i < ncktin; i++) {
                      switch (cktin[i]->value) {
                    case 0:
                    case 1: break;
                    case D: cktin[i]->value = 1; break;
                    case B: cktin[i]->value = 0; break;
                    case U: cktin[i]->value = rand()&01; break;
                      }
               }
               display_io();
        }
        else fprintf(stdout, "\n");
        return(TRUE);
    }
    else if (no_test) {
        /*fprintf(stdout,"redundant fault...\n");*/
        return(FALSE);
    }
    else {
        /*fprintf(stdout,"test aborted due to backtrack limit...\n");*/
        return(MAYBE);
    }
}/* end of podem */


wptr
fault_evaluate(fault)
fptr fault;
{
    register int i;
    register int temp1;
    register wptr w;

    if (fault->io) {
        w = fault->node->owire[0];
        if (w->value == U) return(NULL);
        if (fault->fault_type == 0 && w->value == 1) w->value = D;
        if (fault->fault_type == 1 && w->value == 0) w->value = B;
        return(w);
    }
    else {
        w = fault->node->iwire[fault->index];
        if (w->value == U) return(NULL);
        else {
            temp1 = w->value;
            if (fault->fault_type == 0 && w->value == 1) w->value = D;
            if (fault->fault_type == 1 && w->value == 0) w->value = B;
            if (fault->node->type == OUTPUT) return(NULL);
            evaluate(fault->node);
            w->value = temp1;
            if (fault->node->owire[0]->flag & CHANGED) {
                fault->node->owire[0]->flag &= ~CHANGED;
                return (fault->node->owire[0]);
            }
            else return(NULL);
        }
    }
}/* end of fault_evaluate */


forward_imply(w)
wptr w;
{
    register int i;

    for (i = 0; i < w->nout; i++) {
        if (w->onode[i]->type != OUTPUT) {
            evaluate(w->onode[i]);
            if (w->onode[i]->owire[0]->flag & CHANGED)
                forward_imply(w->onode[i]->owire[0]);
            w->onode[i]->owire[0]->flag &= ~CHANGED;
        }
    }
    return;
}/* end of forward_imply */


wptr
test_possible(fault)
fptr fault;
{
    register nptr n;
    register wptr object_wire;
    register int object_level;
    nptr find_propagate_gate();
    wptr find_pi_assignment();
    int trace_unknown_path();

    if (fault->node->type != OUTPUT) {
        if (fault->node->owire[0]->value ^ U) {
            if (!((fault->node->owire[0]->value == B) ||
                (fault->node->owire[0]->value == D))) return(NULL);
            if (!(n = find_propagate_gate(fault->node->owire[0]->level)))
                return(NULL);
            switch(n->type) {
                case  AND:
                case  NOR: object_level = 1; break;
                case NAND:
                case   OR: object_level = 0; break;
                default:
                  /*---- comment out due to error for C2670.sim ---------
                  fprintf(stderr,
                          "Internal Error(1st bp. in test_possible)!\n");
                  exit(-1);
                  -------------------------------------------------------*/
                  return(NULL);
            }
            object_wire = n->owire[0];
        }
        else {
            if (!(trace_unknown_path(fault->node->owire[0])))
                return(NULL);
            if (fault->io) {
                if (fault->fault_type) object_level = 0;
                else object_level = 1;
                object_wire = fault->node->owire[0];
            }
            else {
                if (fault->node->iwire[fault->index]->value  ^ U) {
                    switch (fault->node->type) {
                        case  AND:
                        case  NOR: object_level = 1; break;
                        case NAND:
                        case   OR: object_level = 0; break;
                        default:
                     /*---- comment out due to error for C2670.sim ---------
                            fprintf(stderr,
                               "Internal Error(2nd bp. in test_possible)!\n");
                            exit(-1);
                     -------------------------------------------------------*/
                            return(NULL);
                    }
                    object_wire = fault->node->owire[0];
                }
                else {
                    if (fault->fault_type) object_level = 0;
                    else object_level = 1;
                    object_wire = fault->node->iwire[fault->index];
                }
            }
        }
    }
    else {
        if (fault->node->iwire[0]->value == U) {
            if (fault->fault_type) object_level = 0;
            else object_level = 1;
            object_wire = fault->node->iwire[0];
        }
        else {
        /*--- comment out due to error for C2670.sim ---*/
/*
            fprintf(stderr,"Internal Error(1st bp. in test_possible)!\n");
            exit(-1);
*/
            return(NULL);
        }
    }
    return(find_pi_assignment(object_wire,object_level));
}/* end of test_possible */



wptr
find_pi_assignment(object_wire,object_level)
wptr object_wire;
int object_level;
{
    register wptr new_object_wire;
    register int new_object_level;
    wptr find_hardest_control(),find_easiest_control();

    if (object_wire->flag & INPUT) {
    object_wire->value = object_level;
    return(object_wire);
    }
    else {
        switch(object_wire->inode[0]->type) {
        case   OR:
        case NAND:
        if (object_level) new_object_wire = find_easiest_control(object_wire->inode[0]);
        else new_object_wire = find_hardest_control(object_wire->inode[0]);
                break;
        case  NOR:
        case  AND:
        if (object_level) new_object_wire = find_hardest_control(object_wire->inode[0]);
        else new_object_wire = find_easiest_control(object_wire->inode[0]);
                break;
        case  NOT:
        case  BUF:
        new_object_wire = object_wire->inode[0]->iwire[0];
        break;
        }
    switch (object_wire->inode[0]->type) {
        case  BUF:
        case  AND:
        case   OR: new_object_level = object_level; break;
        case  NOT:
        case  NOR:
        case NAND: new_object_level = object_level ^ 1; break;
        }
        if (new_object_wire) return(find_pi_assignment(new_object_wire,new_object_level));
        else return(NULL);
    }
}/* end of find_pi_assignment */


wptr
find_hardest_control(n)
nptr n;
{
    register int i;

    for (i = n->nin - 1; i >= 0; i--) {
        if (n->iwire[i]->value  == U) return(n->iwire[i]);
    }
    return(NULL);
}/* end of find_hardest_control */


wptr
find_easiest_control(n)
nptr n;
{
    register int i;

    for (i = 0; i < n->nin; i++) {
        if (n->iwire[i]->value  == U) return(n->iwire[i]);
    }
    return(NULL);
}/* end of find_easiest_control */


nptr
find_propagate_gate(level)
int level;
{
    register int i,j;
    register wptr w;
    int trace_unknown_path();

    for (i = ncktwire - 1; i >= 0; i--) {
        if (sort_wlist[i]->level == level) return(NULL);
        if ((sort_wlist[i]->value == U) &&
            (sort_wlist[i]->inode[0]->flag & MARKED)) {
            for (j = 0; j < sort_wlist[i]->inode[0]->nin; j++) {
                w = sort_wlist[i]->inode[0]->iwire[j];
                if ((w->value == D) || (w->value == B)) {
                    if (trace_unknown_path(sort_wlist[i]))
                        return(sort_wlist[i]->inode[0]);
                   break;
                }
            }
        }
    }
}/* end of find_propagate_gate */


trace_unknown_path(w)
wptr w;
{
    register int i;
    register wptr wtemp;

    if (w->flag & OUTPUT) return(TRUE);
    for (i = 0; i < w->nout; i++) {
        wtemp = w->onode[i]->owire[0];
        if (wtemp->value == U) {
            if(trace_unknown_path(wtemp)) return(TRUE);
        }
    }
    return(FALSE);
}/* end of trace_unknown_path */


check_test()
{
    register int i,is_test;

    is_test = FALSE;
    for (i = 0; i < ncktout; i++) {
        if ((cktout[i]->value == D) || (cktout[i]->value == B)) {
            is_test = TRUE;
        }
        if (is_test == TRUE) break;
    }
    return(is_test);
}/* end of check_test */


mark_propagate_tree(n)
nptr n;
{
    register int i,j;

    if (n->flag & MARKED) return;
    n->flag |= MARKED;
    for (i = 0; i < n->nout; i++) {
        for (j = 0; j < n->owire[i]->nout; j++) {
            mark_propagate_tree(n->owire[i]->onode[j]);
        }
    }
    return;
}/* end of mark_propagate_tree */


unmark_propagate_tree(n)
nptr n;
{
    register int i,j;

    if (n->flag & MARKED) {
        n->flag &= ~MARKED;
        for (i = 0; i < n->nout; i++) {
            for (j = 0; j < n->owire[i]->nout; j++) {
                unmark_propagate_tree(n->owire[i]->onode[j]);
            }
        }
    }
    return;
}/* end of unmark_propagate_tree */


set_uniquely_implied_value(fault)
fptr fault;
{
    register wptr w;
    register int pi_is_reach = FALSE;
    register int i;

    if (fault->io) w = fault->node->owire[0];
    else {
        w = fault->node->iwire[fault->index];
        switch (fault->node->type) {
            case NOT:
            case BUF:
                /*---- comment out due to error for C2670.sim ---------
                fprintf(stderr,
                  "Internal Error(1st bp. in set_uniquely_implied_value)!\n");
                exit(-1);
                -------------------------------------------------------*/
                return(NULL);
            case AND:
            case NAND:
                 for (i = 0; i < fault->node->nin; i++) {
                     if (fault->node->iwire[i] != w) {
                         switch (backward_imply(fault->node->iwire[i],1)) {
                             case TRUE: pi_is_reach = TRUE; break;
                             case CONFLICT: return(CONFLICT); break;
                             case FALSE: break;
                         }
                     }
                 }
                 break;
            case OR:
            case NOR:
                 for (i = 0; i < fault->node->nin; i++) {
                     if (fault->node->iwire[i] != w) {
                         switch (backward_imply(fault->node->iwire[i],0)) {
                             case TRUE: pi_is_reach = TRUE; break;
                             case CONFLICT: return(CONFLICT); break;
                             case FALSE: break;
                         }
                     }
                 }
                 break;
        }
    }
    switch (backward_imply(w,(fault->fault_type ^ 1))) {
        case TRUE: pi_is_reach = TRUE; break;
        case CONFLICT: return(CONFLICT); break;
        case FALSE: break;
    }
    return(pi_is_reach);
}/* end of set_uniquely_implied_value */


backward_imply(current_wire,logic_level)
wptr current_wire;
int logic_level;
{
    register int pi_is_reach = FALSE;
    register int i;

    if (current_wire->flag & INPUT) {
        if (current_wire->value != U &&
            current_wire->value != logic_level) {
            return(CONFLICT);
        }
        current_wire->value = logic_level;
        current_wire->flag |= CHANGED;
        return(TRUE);
    }
    else {
        switch (current_wire->inode[0]->type) {
            case NOT:
                switch (backward_imply(current_wire->inode[0]->iwire[0],
                    (logic_level ^ 1))) {
                    case TRUE: pi_is_reach = TRUE; break;
                    case CONFLICT: return(CONFLICT); break;
                    case FALSE: break;
                }
                break;
            case NAND:
                if (!logic_level) {
                    for (i = 0; i < current_wire->inode[0]->nin; i++) {
                        switch (backward_imply(current_wire->inode[0]->iwire[i],1)) {
                            case TRUE: pi_is_reach = TRUE; break;
                            case CONFLICT: return(CONFLICT); break;
                            case FALSE: break;
                        }
                    }
                }
                break;
            case AND:
                if (logic_level) {
                    for (i = 0; i < current_wire->inode[0]->nin; i++) {
                        switch (backward_imply(current_wire->inode[0]->iwire[i],1)) {
                            case TRUE: pi_is_reach = TRUE; break;
                            case CONFLICT: return(CONFLICT); break;
                            case FALSE: break;
                        }
                    }
                }
                break;
            case OR:
                if (!logic_level) {
                    for (i = 0; i < current_wire->inode[0]->nin; i++) {
                        switch (backward_imply(current_wire->inode[0]->iwire[i],0)) {
                            case TRUE: pi_is_reach = TRUE; break;
                            case CONFLICT: return(CONFLICT); break;
                            case FALSE: break;
                        }
                    }
                }
                break;
            case NOR:
                if (logic_level) {
                    for (i = 0; i < current_wire->inode[0]->nin; i++) {
                        switch (backward_imply(current_wire->inode[0]->iwire[i],0)) {
                            case TRUE: pi_is_reach = TRUE; break;
                            case CONFLICT: return(CONFLICT); break;
                            case FALSE: break;
                        }
                    }
                }
                break;
            case BUF:
                switch (backward_imply(current_wire->inode[0]->iwire[0],logic_level)) {
                    case TRUE: pi_is_reach = TRUE; break;
                    case CONFLICT: return(CONFLICT); break;
                    case FALSE: break;
               }
                break;
        }
        return(pi_is_reach);
    }
}/* end of backward_imply */
