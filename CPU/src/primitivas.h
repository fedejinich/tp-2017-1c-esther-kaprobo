#ifndef PRIMITIVAS_H_
#define PRIMITIVAS_H_

#include "src/Estructuras.h"

t_puntero definirVariable(t_nombre_variable identificador_variable);
t_puntero obtenerPosicionVariable (t_nombre_variable identificador_variable);
void asignar(t_puntero direccion_variable,t_valor_variable valor);
t_valor_variable dereferenciar(t_puntero direccion_variable);
t_valor_variable obtenerValorCompartida(t_nombre_compartida variable);
t_valor_variable asignarValorCompartida(t_nombre_compartida variable,t_valor_variable valor);
void imprimir(t_valor_variable valor_mostrar);
void finalizar();
void wait_kernel(t_nombre_semaforo identificador_semaforo);
void signal_kernel(t_nombre_semaforo identificador_semaforo);


#endif /* PRIMITIVAS_H_ */
