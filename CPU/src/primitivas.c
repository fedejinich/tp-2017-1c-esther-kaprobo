#include "primitivas.h"

t_puntero definirVariable(t_nombre_variable identificador_variable){
	printf("Definiendo variable: %s", identificador_variable);
}

t_puntero obtenerPosicionVariable (t_nombre_variable identificador_variable){
	printf("Obtener Posicion variable: %s", identificador_variable);
}

t_valor_variable dereferenciar(t_puntero direccion_variable){
	printf("Dereferenciar");
}

void asignar(t_puntero direccion_variable,t_valor_variable valor){
	printf("Asigno el valor: %i a la variable en la posicion: %i", valor, direccion_variable);
}

t_valor_variable obtenerValorCompartida(t_nombre_compartida variable){
	printf("Definiendo variable: %s", variable);
}

t_valor_variable asignarValorCompartida(t_nombre_compartida variable,t_valor_variable valor){
	printf("asignar Valor Compartida\n");
}

void irAlLabel(t_nombre_etiqueta etiqueta){
	printf("irALabel\n");
}

void llamarSinRetorno(t_nombre_etiqueta etiqueta){
	printf("llamarSinRetorno\n");
}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){
	printf("llamarConRetorno\n");
}
void retornar(t_valor_variable retorno){
	printf("retornar\n");
}


void finalizar(){
	printf("Termino el programa");
}

void wait_kernel(t_nombre_semaforo identificador_semaforo){
	log_info(logger,"Tamanio semaforo %d", strlen(identificador_semaforo));
	char* nombre_semaforo = malloc(strlen(identificador_semaforo)+1);
	char* barraCero = "\0";
	memcpy(nombre_semaforo, identificador_semaforo, strlen(identificador_semaforo));
	memcpy(nombre_semaforo+strlen(identificador_semaforo), barraCero, 1);
	log_info(logger,"CPU: Pedir semaforo %s de tamanio %d", strlen(nombre_semaforo+1));
	enviar(kernel,PEDIR_SEMAFORO, strlen(nombre_semaforo)+1, nombre_semaforo);
	t_paquete* paquete_semaforo;
	paquete_semaforo= recibir(kernel);
	memcpy(&programaBloqueado, paquete_semaforo->data, 4); //Me trae un 0 si no bloquea y un 1 si bloquea el proceso
	log_info(logger, "ProgramaBloqueado = %d", programaBloqueado);
	free(nombre_semaforo);
	liberar_paquete(paquete_semaforo);
	log_info(logger, "Saliendo del wait");
	return;
}

