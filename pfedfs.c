/**********************************************************************/
/*           Parallel-Fault Event-Driven Fault Simulator              */
/*                                                                    */
/*           Author: Tony Hi Keung Ma                                 */
/*           last update : 4/11/1986                                  */
/**********************************************************************/

#include <stdio.h>
#include "global.h"
#include "miscell.h"


/*
 * routine to do the fault simulation 
 */

#define num_of_pattern 16

unsigned int Mask[16] = {0x00000003, 0x0000000c, 0x00000030, 0x000000c0,
                         0x00000300, 0x00000c00, 0x00003000, 0x0000c000,
                         0x00030000, 0x000c0000, 0x00300000, 0x00c00000,
                         0x03000000, 0x0c000000, 0x30000000, 0xc0000000,};

unsigned int Unknown[16] = {0x00000001, 0x00000004, 0x00000010, 0x00000040,
                            0x00000100, 0x00000400, 0x00001000, 0x00004000,
                            0x00010000, 0x00040000, 0x00100000, 0x00400000,
                            0x01000000, 0x04000000, 0x10000000, 0x40000000,};

wptr first_faulty_wire;
extern int debug;

fptr
fault_simulate_vectors(vectors,num_vectors,flist,total_detect_num)
char *vectors[];
int num_vectors;
fptr flist;
int *total_detect_num;
{
    int i,j,nv,current_detect_num;
    fptr fault_sim_a_vector();
 
/* for every vector */
    /*for(i=0; i<num_vectors; i++)*/
    for(i=num_vectors-1; i>=0; i--) {

/* for every input, set its value to the current vector value */
        for(j=0; j<ncktin; j++) {
            nv=ctoi(vectors[i][j]);
            sort_wlist[j]->value = nv;
        }

        flist=fault_sim_a_vector(flist,&current_detect_num);
        *total_detect_num += current_detect_num;
        fprintf(stderr,"vector[%d] detects %d faults (%d)\n",i,
                current_detect_num,*total_detect_num);
    }
    return(flist);
}

fptr
fault_sim_a_vector(flist,num_of_current_detect)
fptr flist;
int *num_of_current_detect;
{
    wptr w,faulty_wire,wtemp;
    fptr f,simulated_fault_list[num_of_pattern],ftemp;
    int fault_type;
    register int i,j,k,start_wire_index,num_of_fault;
    int fault_sim_evaluate();
    wptr get_faulty_wire();
    int sim();
    static int test_no = 0;

    test_no++;
    num_of_fault = 0;

/* num_of_current_detect is used to keep track of the number of undetected
 * faults detected by this vector, initialize it to zero */
    *num_of_current_detect = 0;
    start_wire_index = 10000;
    first_faulty_wire = NULL;

/* initialize the circuit - mark all inputs as changed and all other
 * nodes as unknown (2) */
    for (i = 0; i < ncktwire; i++) {
        if (i < ncktin) {
            sort_wlist[i]->flag |= CHANGED;
        }
        else {
            sort_wlist[i]->value = 2;
        }
    }

    sim(); /* do a fault-free simulation */
    if (debug) { display_io(); }

/* expand the 0,1,2 value in value to a 16 wide bit-mask and store
 * it in wire_value1 and wire_value2 */
    for (i = 0; i < ncktwire; i++) {
        switch (sort_wlist[i]->value) {
            case 1: sort_wlist[i]->wire_value1 = ALL_ONE;
                    sort_wlist[i]->wire_value2 = ALL_ONE; break;
            case 2: sort_wlist[i]->wire_value1 = 0x55555555;
                    sort_wlist[i]->wire_value2 = 0x55555555; break;
            case 0: sort_wlist[i]->wire_value1 = ALL_ZERO;
                    sort_wlist[i]->wire_value2 = ALL_ZERO; break;
        }
        sort_wlist[i]->pnext = NULL;
    }

/* for every fault (start f with the fault list, stop when the last 
 * element is null, step through the fault list using pnext_undetect */
    for (f = flist; f; f = f->pnext_undetect) {
        if (f->detect == REDUNDANT) { continue;} /* ignore redundant faults */

/* if the fault is active
 * (sa1 with correct output of 0 or sa0 with correct output of 1) */
        if (f->fault_type != sort_wlist[f->to_swlist]->value) {

/* if this is an output node or is directly connected to an output
 * the fault is detected */
            if ((f->node->type == OUTPUT) ||
                (f->io == GO && sort_wlist[f->to_swlist]->flag & OUTPUT)) {
                    f->detect = TRUE;
            }
            else {

/* if the fault is an output fault */
                if (f->io == GO) {

/* if this wire is not already marked as faulty, mark the wire as faulty
 * and add the wire to the list of faulty wires. */
                    if (!(sort_wlist[f->to_swlist]->flag & FAULTY)) {
                        sort_wlist[f->to_swlist]->pnext = first_faulty_wire;
                        first_faulty_wire = sort_wlist[f->to_swlist];
                        first_faulty_wire->flag |= FAULTY;
                    }

/* add the fault to the simulated list and inject it */
                    simulated_fault_list[num_of_fault] = f;
                    inject_fault_value(sort_wlist[f->to_swlist], num_of_fault,
                                       f->fault_type);

/* mark the wire as having a fault injected and schedule the outputs of this
 * gate */
                    sort_wlist[f->to_swlist]->flag |= FAULT_INJECTED;
                    for (k = 0; k < sort_wlist[f->to_swlist]->nout; k++) {
                        sort_wlist[f->to_swlist]->onode[k]->owire[0]->flag |=
                                                                    SCHEDULED;
                    }

/* increment the number of simulated faults & set the start wire index
 * to the lesser of the current start_wire_index and the current faulty
 * gate the start_wire_index is used to keep track of the first gate
 * that needs to be evaluated */
                    num_of_fault++;
                    start_wire_index = MIN(start_wire_index,f->to_swlist);
                }

/* the fault is an input fault */
                else {

/* if the fault is propagated, set faulty_wire equal to the faulty wire */
                    if (faulty_wire = get_faulty_wire(f,&fault_type)) {

/* if the fault is shown at an output, it is detected */
                        if (faulty_wire->flag & OUTPUT) {
                            f->detect = TRUE;
                        }

                        else {

/* if this wire is not already marked as faulty, mark the wire as faulty
 * and add the wire to the list of faulty wires. */
                            if (!(faulty_wire->flag & FAULTY)) {
                                faulty_wire->pnext = first_faulty_wire;
                                first_faulty_wire = faulty_wire;
                                first_faulty_wire->flag |= FAULTY;
                            }

/* add the fault to the simulated list and inject it */
                            simulated_fault_list[num_of_fault] = f;
                            inject_fault_value(faulty_wire, num_of_fault,
                                               fault_type);

/* mark the wire as having a fault injected and schedule the outputs of this
 * gate */
                            faulty_wire->flag |= FAULT_INJECTED;
                            for (k = 0; k < faulty_wire->nout; k++) {
                                faulty_wire->onode[k]->owire[0]->flag |=
                                                                   SCHEDULED;
                            }

/* increment the number of simulated faults & set the
 * start wire index to the lesser of the current
 * start_wire_index and the current faulty gate
 * the start_wire_index is used to keep track of the
 * first gate that needs to be evaluated */
                            num_of_fault++;
                            start_wire_index = MIN(start_wire_index,
                                                   f->to_swlist);
                        }
                    }
                }
            }
        }

/*
 * fault simulation starts here
 */

/* if there are 16 faults injected or no more undetected faults,
 * do the fault simulation */
        if ((num_of_fault == num_of_pattern) || !(f->pnext_undetect)) {

/* starting with start_wire_index, evaulate all scheduled wires */
            for (i = start_wire_index; i < ncktwire; i++) {
                if (sort_wlist[i]->flag & SCHEDULED) {
                    sort_wlist[i]->flag &= ~SCHEDULED;
                    fault_sim_evaluate(sort_wlist[i]);
                }
            } /* event evaluations end here */

/* check detection and reset wires' faulty values
 * back to fault-free values
 */
            for (w = first_faulty_wire; w; w = wtemp) {
                wtemp = w->pnext;
                w->pnext = NIL(struct WIRE);
                w->flag &= ~FAULTY;
                w->flag &= ~FAULT_INJECTED;
                w->fault_flag &= ALL_ZERO;
                if (w->flag & OUTPUT) {
                    for (i = 0; i < num_of_fault; i++) {
                        if (!(simulated_fault_list[i]->detect)) {
                            if ((w->wire_value2 & Mask[i]) ^
                                (w->wire_value1 & Mask[i])) {
                                if (((w->wire_value2 & Mask[i]) ^ Unknown[i])&&
                                    ((w->wire_value1 & Mask[i]) ^ Unknown[i])){
                                    simulated_fault_list[i]->detect = TRUE;
                                }
                            }
                        }
                    }
                }
                w->wire_value2 = w->wire_value1;
            }
          /*check();
            check2();*/
            num_of_fault = 0;
            start_wire_index = 10000;
            first_faulty_wire = NULL;
        }
    }

/* trim faults from the front of the fault list */
    while(flist) {
        if (flist->detect == TRUE) {
            (*num_of_current_detect) += flist->eqv_fault_num;
            f = flist->pnext_undetect;
            flist->pnext_undetect = NULL;
            flist = f;
        }
        else {break;}
    }

/* remove faults from within the fault list*/
    if (flist) {
        for (f = flist; f->pnext_undetect; f = ftemp) {
            if (f->pnext_undetect->detect == TRUE) { 
                (*num_of_current_detect) += f->pnext_undetect->eqv_fault_num;
                f->pnext_undetect = f->pnext_undetect->pnext_undetect;
                ftemp = f;
            }
            else {
                ftemp = f->pnext_undetect;
            }
        }
    }
    return(flist);
}/* end of fault_sim_a_vector */


check2()
{
    register int i;
  
    for (i = 0; i < ncktwire; i++) {
        if (sort_wlist[i]->flag & (FAULTY | FAULT_INJECTED | SCHEDULED) ||
            (sort_wlist[i]->fault_flag)){
            fprintf(stdout,"something is fishy(check2)...\n");
        }
    }
    return;
}/* end of check2 */


check()
{
    register int i;

    for (i = 0; i < ncktwire; i++) {
        if (sort_wlist[i]->wire_value1 != sort_wlist[i]->wire_value2) {
            fprintf(stdout,"something is fishy...\n");
        }
    }
    return;
}/* end of check */


fault_sim_evaluate(w)
wptr w;
{
    unsigned int new_value;
    register nptr n;
    register int i;
    int combine();
    unsigned int PINV(),PEXOR(),PEQUIV();

    /*if (w->flag & INPUT) return;*/
    n = w->inode[0];
    switch(n->type) {
        case AND:
        case BUF:
        case NAND:
            new_value = ALL_ONE;
            for (i = 0; i < n->nin; i++) {
                new_value &= n->iwire[i]->wire_value2;
            }
            if (n->type == NAND) {
                new_value = PINV(new_value);
            }
            break;
        case OR:
        case NOR:
            new_value = ALL_ZERO;
            for (i = 0; i < n->nin; i++) {
                new_value |= n->iwire[i]->wire_value2;
            }
            if (n->type == NOR) {
                new_value = PINV(new_value);
            }
            break;
        case NOT:
            new_value = PINV(n->iwire[0]->wire_value2);
            break;
        case XOR:
            new_value = PEXOR(n->iwire[0]->wire_value2,
                              n->iwire[1]->wire_value2);
            break;
        case EQV:
            new_value = PEQUIV(n->iwire[0]->wire_value2,
                               n->iwire[1]->wire_value2);
            break;
    }

/* if the newly calculated value is any different than the current value,
 * save it */
    if (w->wire_value1 != new_value) {

/* if this wire is faulty, make sure the fault remains injected */
        if (w->flag & FAULT_INJECTED) {
            combine(w,&new_value);
        }
        w->wire_value2 = new_value;
        if (!(w->flag & FAULTY)) {
            w->flag |= FAULTY;
            w->pnext = first_faulty_wire;
            first_faulty_wire = w;
        }
        for (i = 0; i < w->nout; i++) {
            if (w->onode[i]->type != OUTPUT) {
                w->onode[i]->owire[0]->flag |= SCHEDULED;
            }
        }
    }
    return;
}/* end of fault_sim_evaluate */


wptr get_faulty_wire(f,fault_type)
fptr f;
int *fault_type;
{
    register int i,is_faulty;

    is_faulty = TRUE;
    switch(f->node->type) {
        case NOT:
        case BUF:
            fprintf(stdout,"something is fishy(get_faulty_net)...\n");
            break;
        case AND:
            for (i = 0; i < f->node->nin; i++) {
                if (f->node->iwire[i] != sort_wlist[f->to_swlist]) {
                    if (f->node->iwire[i]->value != 1) {
                        is_faulty = FALSE;
                    }
                }
            }
            *fault_type = STUCK1;
            break;
        case NAND:
            for (i = 0; i < f->node->nin; i++) {
                if (f->node->iwire[i] != sort_wlist[f->to_swlist]) {
                    if (f->node->iwire[i]->value != 1) {
                        is_faulty = FALSE;
                    }
                }
            }
            *fault_type = STUCK0;
            break;
        case OR:
            for (i = 0; i < f->node->nin; i++) {
                if (f->node->iwire[i] != sort_wlist[f->to_swlist]) {
                    if (f->node->iwire[i]->value != 0) {
                        is_faulty = FALSE;
                    }
                }
            }
            *fault_type = STUCK0;
            break;
        case  NOR:
            for (i = 0; i < f->node->nin; i++) {
                if (f->node->iwire[i] != sort_wlist[f->to_swlist]) {
                    if (f->node->iwire[i]->value != 0) {
                        is_faulty = FALSE;
                    }
                }
            }
            *fault_type = STUCK1;
            break;
        case XOR:
            for (i = 0; i < f->node->nin; i++) {
                if (f->node->iwire[i] != sort_wlist[f->to_swlist]) {
                    if (f->node->iwire[i]->value == 0) {
                        *fault_type = f->fault_type;
                    }
                    else {
                        *fault_type = f->fault_type ^ 1;
                    }
                }
            }
            break;
        case EQV:
            for (i = 0; i < f->node->nin; i++) {
                if (f->node->iwire[i] != sort_wlist[f->to_swlist]) {
                    if (f->node->iwire[i]->value == 0) {
                        *fault_type = f->fault_type ^ 1;
                    }
                    else {
                        *fault_type = f->fault_type;
                    }
                }
            }
            break;
    }
    if (is_faulty) {
        return(f->node->owire[0]);
    }
    return(NULL);
}/* end of get_faulty_wire */


inject_fault_value(faulty_wire,bit_position,fault)
wptr faulty_wire;
int bit_position,fault;
{
    if (fault) faulty_wire->wire_value2 |= Mask[bit_position];
    else faulty_wire->wire_value2 &= ~Mask[bit_position];
    faulty_wire->fault_flag |= Mask[bit_position];
    return;
}/* end of inject_fault_value */


combine(w,new_value)
wptr w;
unsigned int *new_value;
{
    register int i;
  
    for (i = 0; i < num_of_pattern; i++) {
        if (w->fault_flag & Mask[i]) {
            *new_value &= ~Mask[i];
            *new_value |= (w->wire_value2 & Mask[i]);
        }
    }
    return;
} /* end of combine */


unsigned int
PINV(value)
unsigned int value;
{
    return((((value & 0x55555555) << 1) ^ 0xaaaaaaaa) | 
           (((value & 0xaaaaaaaa) >> 1) ^ 0x55555555));
}/* end of PINV */


unsigned int
PEXOR(value1,value2)
unsigned int value1,value2;
{
    unsigned int PINV();
    return((value1 & PINV(value2)) | (PINV(value1) & value2));
}/* end of PEXOR */


unsigned int
PEQUIV(value1,value2)
unsigned int value1,value2;
{
    unsigned int PINV();
    return((value1 | PINV(value2)) & (PINV(value1) | value2));
}/* end of PEQUIV */
