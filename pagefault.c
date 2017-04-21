#include <stdio.h>
#include <stdlib.h>
#include "mmu.h"
#include <limits.h>

#define RESIDENTSETSIZE 24

extern char *base;
extern int framesbegin;
extern int idproc;
extern int systemframetablesize;
extern int ptlr;

extern struct SYSTEMFRAMETABLE *systemframetable;
extern struct PROCESSPAGETABLE *ptbr;
extern struct PROCESSPAGETABLE *gprocesspagetable;

int getfreeframe();
int swapmemory();

// Rutina de fallos de página

int pagefault(char *vaddress)
{
    int i;
    int frame;
    long pag_a_expulsar;
    long pag_del_proceso;

    // Calcula la página del proceso
    pag_del_proceso=(long) vaddress>>12;
    // Cuenta los marcos asignados al proceso
    i=countframesassigned();
  
    // Busca un marco libre en el sistema
    frame=getfreeframe();

    if(frame==-1)
    {
        return(-1); // Regresar indicando error de memoria insuficiente
    }


    (ptbr+pag_del_proceso)->presente=1;
    (ptbr+pag_del_proceso)->framenumber=frame;


    return(1); // Regresar todo bien
}


int getfreeframe()
{
    int i;
    // Busca un marco libre en el sistema
    for(i=framesbegin;i<systemframetablesize+framesbegin;i++) {
        if(!systemframetable[i].assigned)
        {
            systemframetable[i].assigned=1;
            break;
        }
    }

    if(i<systemframetablesize+framesbegin) {
        systemframetable[i].assigned=1;
    }

    else {
        i = swapmemory();
    }

    return i;
}

int swapmemory()
{
    // buscar el menos usado
    int i;
    unsigned long last = ULONG_MAX;
    FILE *file_swap;
    int last_index = -1;

    for (i=0; i<RESIDENTSETSIZE/2; i++)
    {
        if(ptbr[i].tlastaccess < last) {
            last_index = i;
            last = ptbr[i].tlastaccess;
        }
    }
    printf(">>MENOS USADO: i:%d val:%d\n", last_index, ptbr[last_index]);
    

    if(ptbr[last_index].modificado == 1) {
        //escribir a memoria
        int FN = ptbr[last_index].framenumber;
        printf(">>Frame fue modificado, escribiendo a memoria.\n");
        
        /*file_swap = fopen("swap", "wb");
        fwrite(&systemframetable[FN],sizeof(systemframetable[FN]),1, file_swap);*/
    }
    
    ptbr[last_index].presente = 0;


    printf(">>Swap completado de frame: %d\n", ptbr[last_index].framenumber);
    return ptbr[last_index].framenumber;
}



