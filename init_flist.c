/**********************************************************************/
/*           routines building the dummy input and output gate list;  */
/*           building the fault list;                                 */
/*           calculating the total fault coverage.                    */
/*                                                                    */
/*           Author: Hi-Keung Tony Ma                                 */
/*           last update : 10/29/1987                                 */
/**********************************************************************/

#include <stdio.h>
#include "global.h"
#include "miscell.h"

int num_of_gate_fault;
extern int debug;

generate_fault_list()
{
    int i,j,k;
    wptr w;
    nptr n;
    fptr f;
    int fault_num;

    first_fault = NIL(struct FAULT);
    num_of_gate_fault = 0;

    for (i = ncktwire - 1; i >= 0; i--) {
        w = sort_wlist[i];
        n = w->inode[0];
        if (!(f = podemALLOC(1,struct FAULT))) error("No more room!");
        f->node = n;
        f->io = GO;
        f->fault_type = STUCK0;
        f->to_swlist = i;
        switch (n->type) {
            case   AND:
            case   NOR:
            case   NOT:
            case   BUF:
                f->eqv_fault_num = 1;
                for (j = 0; j < w->inode[0]->nin; j++) {
                    if (w->inode[0]->iwire[j]->nout > 1) f->eqv_fault_num++;
                }
                break;
            case INPUT:
            case    OR:
            case  NAND: 
            case   EQV:
            case   XOR: f->eqv_fault_num = 1; break;
        }
        num_of_gate_fault += f->eqv_fault_num;
        f->pnext = first_fault;
        f->pnext_undetect = first_fault;
        first_fault = f;
        if (!(f = podemALLOC(1,struct FAULT))) error("No more room!");
        f->node = n;
        f->io = GO;
        f->fault_type = STUCK1;
        f->to_swlist = i;
        switch (n->type) {
            case    OR:
            case  NAND:
            case   NOT:
            case   BUF:
                f->eqv_fault_num = 1;
                for (j = 0; j < w->inode[0]->nin; j++) {
                    if (w->inode[0]->iwire[j]->nout > 1) f->eqv_fault_num++;
                }
                break;
            case INPUT:
            case   AND:
            case   NOR: 
            case   EQV:
            case   XOR: f->eqv_fault_num = 1; break;
        }
        num_of_gate_fault += f->eqv_fault_num;
        f->pnext = first_fault;
        f->pnext_undetect = first_fault;
        first_fault = f;
        if (w->nout > 1) {
            for (j = 0 ; j < w->nout; j++) {
                n = w->onode[j];
                switch (n->type) {
                    case OUTPUT:
                    case    OR:
                    case   NOR: 
                    case   EQV:
                    case   XOR:
                        if (!(f = podemALLOC(1,struct FAULT))) error("No more room!");
                        f->node = n;
                        f->io = GI;
                        f->fault_type = STUCK0;
                        f->to_swlist = i;
                        f->eqv_fault_num = 1;
                        for (k = 0; k < n->nin; k++) {
                            if (n->iwire[k] == w) f->index = k;
                        }
                        num_of_gate_fault++;
                        f->pnext = first_fault;
                        f->pnext_undetect = first_fault;
                        first_fault = f;
                        break;
                }
                switch (n->type) {
                    case OUTPUT:
                    case   AND:
                    case  NAND: 
                    case   EQV:
                    case   XOR:
                        if (!(f = podemALLOC(1,struct FAULT))) error("Room more room!");
                        f->node = n;
                        f->io = GI;
                        f->fault_type = STUCK1;
                        f->to_swlist = i;
                        f->eqv_fault_num = 1;
                        for (k = 0; k < n->nin; k++) {
                            if (n->iwire[k] == w) f->index = k;
                        }
                        num_of_gate_fault++;
                        f->pnext = first_fault;
                        f->pnext_undetect = first_fault;
                        first_fault = f;
                        break;
                }
            }
        }
    }
    for (f = first_fault, fault_num = 0; f; f = f->pnext, fault_num++) {
        f->fault_no = fault_num;
    }
    fprintf(stdout,"#number of equivalent faults = %d\n", fault_num);
    return;  
}/* end of generate_fault_list */


/* computing the actual fault coverage */
compute_fault_coverage()
{
    register double gate_fault_coverage,eqv_gate_fault_coverage;
    register int no_of_detect,eqv_no_of_detect,eqv_num_of_gate_fault;
    register fptr f;

    debug = 0;
    no_of_detect = 0;
    gate_fault_coverage = 0;
    eqv_no_of_detect = 0;
    eqv_gate_fault_coverage = 0;
    eqv_num_of_gate_fault = 0;
    
    for (f = first_fault; f; f = f->pnext) {

        if (debug) {
            if (!f->detect) {
                switch (f->node->type) {
                   case INPUT:
                        fprintf(stdout,"primary input: %s\n",f->node->owire[0]->name);
                      break;
                    case OUTPUT:
                        fprintf(stdout,"primary output: %s\n",f->node->iwire[0]->name);
                      break;
                    default:
                        fprintf(stdout,"gate: %s ;",f->node->name);
                        if (f->io == GI) {
                            fprintf(stdout,"input wire name: %s\n",f->node->iwire[f->index]->name);
                        }
                        else {
                            fprintf(stdout,"output wire name: %s\n",f->node->owire[0]->name);
                        }
                      break;
                }
                fprintf(stdout,"fault_type = ");
                switch (f->fault_type) {
                   case STUCK0:
                      fprintf(stdout,"s-a-0\n"); break;
                   case STUCK1:
                      fprintf(stdout,"s-a-1\n"); break;
                }
                fprintf(stdout,"no of equivalent fault = %d\n",f->eqv_fault_num);
                fprintf(stdout,"detection flag = %d\n",f->detect);
                fprintf(stdout,"\n");
            }
        }

         if (f->detect == TRUE) {
              no_of_detect += f->eqv_fault_num;
            eqv_no_of_detect++;
          }
          eqv_num_of_gate_fault++;
    }
    if (num_of_gate_fault != 0) 
    gate_fault_coverage = (((double) no_of_detect) / num_of_gate_fault) * 100;
    if (eqv_num_of_gate_fault != 0) 
    eqv_gate_fault_coverage = (((double) eqv_no_of_detect) / eqv_num_of_gate_fault) * 100;
    
    /* print out fault coverage results */
    fprintf(stdout,"\n");
    fprintf(stdout,"#FAULT COVERAGE RESULTS :\n");
    fprintf(stdout,"#number of test vectors = %d\n",in_vector_no);
    fprintf(stdout,"#total number of gate faults = %d\n",num_of_gate_fault);
    fprintf(stdout,"#total number of detected faults = %d\n",no_of_detect);
    fprintf(stdout,"#total gate fault coverage = %5.2f%%\n",gate_fault_coverage);
    fprintf(stdout,"#number of equivalent gate faults = %d\n",eqv_num_of_gate_fault);
    fprintf(stdout,"#number of equivalent detected faults = %d\n",eqv_no_of_detect);
    fprintf(stdout,"#equivalent gate fault coverage = %5.2f%%\n",eqv_gate_fault_coverage);
    fprintf(stdout,"\n");
    return;  
}/* end of compute_fault_coverage */


create_dummy_gate()
{
    register int i,j;
    register int num_of_dummy;
    register nptr n,*n2;
    char sgate[25];
    char intstring[25];
    nptr getnode();

    num_of_dummy = 0;

    for (i = 0; i < ncktin; i++) {
          num_of_dummy++;
         /*--- changed by Shi-Yu, April 2, 1997 ---*/
         sprintf(intstring, "%d", num_of_dummy);
/*
         itoa(num_of_dummy,intstring);
*/
         sprintf(sgate,"dummy_gate%s",intstring);
         n = getnode(sgate);
         n->nout = 1;
         n->type = INPUT;
         if (!(n->owire = podemALLOC(1,wptr))) error("No more room!");
         n->owire[0] = cktin[i];
         if (!(cktin[i]->inode = podemALLOC(1,nptr))) error("No more romm!");
         cktin[i]->inode[0] = n;
         cktin[i]->nin = 1;
    }

    for (i = 0; i < ncktout; i++) {
          num_of_dummy++;
         itoa(num_of_dummy,intstring);
         sprintf(sgate,"dummy_gate%s",intstring);
         n = getnode(sgate);
         n->nin = 1;
         n->type = OUTPUT;
         if (!(n->iwire = podemALLOC(1,wptr))) error("No more room!");
         n->iwire[0] = cktout[i];
         cktout[i]->nout++;
         if (!(n2 = podemALLOC(cktout[i]->nout,nptr))) error("No more room!");
         n2[(cktout[i]->nout - 1)] = n;
        for (j = 0; j < (cktout[i]->nout - 1); j++) {
            n2[j] = cktout[i]->onode[j];
        }
         if (cktout[i]->onode) free(cktout[i]->onode);
         cktout[i]->onode = n2;
    }
    return;
}/* end of create_dummy_gate */


/*
*   The itoa function
*/

reverse(s)  /*  reverse string s in place */
char s[];
{
   int c,i,j;

   for(i=0,j = strlen(s) - 1;i<j;i++,j--) {
      c = s[i];
      s[i] = s[j];
      s[j] = c;
   }
}

itoa(n,s)   /*  convert n to characters in s */
int n;
char s[];
{
   int i,sign;

   if ((sign = n) < 0) { /* record sign */
       n = -n;
   }
   i = 0;
   do {   /*  generate digits in reverse order */
        s[i++] = n % 10 + '0' ;  /* get next digit */
   } while ((n/=10) > 0); /* delete it */
   if (sign < 0) {
       s[i++] = '-';
   }
   s[i] = '\0';
   reverse(s);

}  /*   end of itoa  */


