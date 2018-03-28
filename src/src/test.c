#include <stdio.h>

#include "usedBDD.h"

// #include "kernel.h"





int main() {
    BDD v1, v3;
    BDD seta, setb;
    static int v[2] = {1,3};
    
    bdd_init(100,100);
    bdd_setvarnum(5);

    v1 = bdd_ithvar(1);
    v3 = bdd_ithvar(3);

       // One way 
    seta = bdd_addref( bdd_apply(v1,v3,bddop_and) );
    bdd_printtable(seta);

       // Another way 
    setb = bdd_addref( bdd_makeset(v,2) );
    bdd_printtable(setb);
}