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

int free_index = 0;

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
    frame=getfreeframe(pag_del_proceso);

    if(frame==-1)
    {
        return(-1); // Regresar indicando error de memoria insuficiente
    }


    (ptbr+pag_del_proceso)->presente=1;
    (ptbr+pag_del_proceso)->modificado=0;
    (ptbr+pag_del_proceso)->framenumber=frame;


    return(1); // Regresar todo bien
}


int getfreeframe(long pag_del_proceso)
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
        i = swapmemory(pag_del_proceso);
    }

    return i;
}

int swapmemory(long pag_del_proceso)
{

    int i;
    free_index++ % 12;
    FILE *swap_file;
    fopen("swap", "r+b");

    // escribir en 'swap' contenido de systemframetable[free_index] > swap[free_index *4k]
    fseek(swap_file, free_index<<12, SEEK_SET);
    fwrite(systemframetable[free_index].paddress, 4096, 1, swap_file);

    // SWAP
    fseek(swap_file, free_index<<12, SEEK_SET);
    char *swpfreeindx = (char *) malloc(4096);
    fread(swpfreeindx, 4096, 1, swap_file);
    
    // leer swappagproc
    fseek(swap_file, pag_del_proceso<<12, SEEK_SET);
    char *swppagproc = (char *) malloc(4096);
    fread(swppagproc, 4096, 1, swap_file);
    
    //Escribir swpfreeindx
    fseek(swap_file, pag_del_proceso<<12,SEEK_SET);
    fwrite(swpfreeindx, 4096,1,swap_file);

    //Escribir swappagproc
    fseek(swap_file, free_index<<12,SEEK_SET);
    fwrite(swppagproc, 4096,1, swap_file);



    // escribir de swap[free_index *4k] a systemframetable[free_index]
    fseek(swap_file, free_index<<12, SEEK_SET);
    fwrite(systemframetable[free_index].paddress, 4096,1 , swap_file);

    fclose(swap_file);

    for(i=0; i < ptlr; i++) {
        if(ptbr[i].framenumber == free_index) {
            ptbr[i].presente = 0;
            ptbr[i].modificado = 0;
            break;
        }
    }


    
    return free_index;
}



