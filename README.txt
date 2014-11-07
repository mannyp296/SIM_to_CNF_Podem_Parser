Pidem Tool Projects

By: Manuel Perez

//////////////////////////
Podem tool Changes
//////////////////////////

This changes are in podem/podem/input.c

d_circuit():

A function used to translate sim files into cnf files.

In order to translate all the sim files into cnf form fist the podem
had to be analysed. It had quite a lot of extra functionality, but
it included a function called display_circuit() that was the base of
the parser. Podem created data structures (node and wire structs)
that would hold all the information needed. A list of all the nodes
was stored in a hash table. This was all incorporate in a function
d_circuit in the input.c file that translated the files. The main
file was also change to accommodate the new function.

The first step was assigning each distinct literal read by the
parser a specific number. This was done by iterating through the
truth table, accessing the node’s input and output wire names and
putting them on an array. The index of the each array allocation
was used as a distinct literal. This alone did not identify “*”
as the not of the literals. In order to fix this a loop that would
check for this specific value was used. Additionally, every time
the parser looked into the array to find the value it would
compare then input string with the strings save in the array
character by character in but ignoring the “*”. Because of this
it was possible to evade assigning different numbers to the same
literal with a “*”. 

As the array was created the parser would save the indexes of
that input and output wires of the specific node. Then using a
switch statement that would check the type of gate of the node,
and it would print a different CNF implementation using the
indexes of the input and output wires. For example the CNF of an
“and” gate was (a*+b*+c)(a+c*)(b+c*). The actual number of
clauses and literals varied with the number of inputs of the node.
With this the CNF translation of the files was printed.

bddd_circuit():

This function was also modelled after display_circuit(). It used
the very same data structures to go through the circuit net-list
recursively and create a bddd data structure. This was done by
matching each node's function and input/output wires to their
bddd function equivalent. The Cudd_DumpDot() was then used to create
the bddd maps in the Examples folder.

Note: Cudd-2.4.2 must be downloaded to use Cudd_DumpDot()
