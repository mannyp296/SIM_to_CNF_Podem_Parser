#include <stdio.h>
#include "global.h"
#include "miscell.h"

extern int total_attempt_num;

test()
{
    register fptr undetect_fault;
    register fptr f,fault_under_test, ftemp;
    fptr fault_sim_a_vector(), fault_simulate_vectors();
    int podem();
    int current_detect_num,total_detect_num, i;
    int total_no_of_backtracks = 0;
    int current_backtracks;
    register int no_of_aborted_faults = 0, no_of_redundant_faults = 0;
    register int no_of_calls = 0;
    int display_undetect();

    in_vector_no = 0;
    total_detect_num = 0;
    undetect_fault = first_fault;
    fault_under_test = first_fault;

    if(fsim_only)
    {
        undetect_fault=fault_simulate_vectors(vectors,sim_vectors,undetect_fault,&total_detect_num);
        fault_under_test = undetect_fault;
        in_vector_no+=sim_vectors;
        display_undetect(undetect_fault);
        fprintf(stdout,"\n");
        return;
    }

    while(fault_under_test) {
        switch(podem(fault_under_test,&current_backtracks)) {
            case TRUE:
                if (total_attempt_num == 1) {
                    undetect_fault = fault_sim_a_vector(undetect_fault,&current_detect_num);
                    total_detect_num += current_detect_num;
                }
                else {
                    fault_under_test->detect = TRUE;
                    for (f = undetect_fault; f; f = f->pnext_undetect) {
                        if (f == fault_under_test) {
                            if (f == undetect_fault)
                                undetect_fault = undetect_fault->pnext_undetect;
                            else {
                                ftemp->pnext_undetect = f->pnext_undetect;
                            }
                            break;
                        }
                        ftemp = f;
                    }
                }
                in_vector_no++;
                break;
            case FALSE:
                fault_under_test->detect = REDUNDANT;
                no_of_redundant_faults++;
                break;
            case MAYBE:
                no_of_aborted_faults++;
                break;
        }
        fault_under_test->test_tried = TRUE;
        fault_under_test = NULL;
        for (f = undetect_fault; f; f = f->pnext_undetect) {
            if (!f->test_tried) {
                fault_under_test = f;
                break;
            }
        }
        total_no_of_backtracks += current_backtracks;
        no_of_calls++;
    }
    display_undetect(undetect_fault);
    fprintf(stdout,"\n");
    fprintf(stdout,"#number of aborted faults = %d\n",no_of_aborted_faults);
    fprintf(stdout,"\n");
    fprintf(stdout,"#number of redundant faults = %d\n",no_of_redundant_faults);
    fprintf(stdout,"\n");
    fprintf(stdout,"#number of calling podem1 = %d\n",no_of_calls);
    fprintf(stdout,"\n");
    fprintf(stdout,"#total number of backtracks = %d\n",total_no_of_backtracks);
    return;
}/* end of test */
