#include <stdio.h>

void cuboPorReferencia( int *Num);

int main(){
    int numero = 5;
    printf("El valor original del numero es %d", numero);
    cuboPorReferencia(&numero);
    printf ("\nEl nuevo valor de número es %d\n", numero);
    return 0;
}
void cuboPorReferencia(int *Num){
    *Num = (*Num) * (*Num) * (*Num);
}