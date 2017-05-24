#ifndef PRIMITIVAS_H_
#define PRIMITIVAS_H_

#include "src/Estructuras.h"
#include <parser/parser.h>

t_puntero definirVariable(t_nombre_variable identificador_variable);
t_puntero obtenerPosicionVariable (t_nombre_variable identificador_variable);
t_valor_variable dereferenciar(t_puntero direccion_variable);
void asignar(t_puntero direccion_variable,t_valor_variable valor);
t_valor_variable obtenerValorCompartida(t_nombre_compartida variable);
t_valor_variable asignarValorCompartida(t_nombre_compartida variable,t_valor_variable valor);
void irAlLabel(t_nombre_etiqueta etiqueta);
void llamarSinRetorno(t_nombre_etiqueta etiqueta);
void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar);
void retornar(t_valor_variable retorno);
void finalizar();

void wait_kernel(t_nombre_semaforo identificador_semaforo);
void signal_kernel(t_nombre_semaforo identificador_semaforo);


#endif /* PRIMITIVAS_H_ */
