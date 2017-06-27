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
	char* variable_compartida = malloc(strlen(variable)+1);
	char* barra_cero = "\0";
	t_paquete* paquete;
	int valor;
	memcpy(variable_compartida, variable, strlen(variable));
	memcpy(variable_compartida+strlen(variable), barra_cero,1);
	log_info(logger,"Obteniendo la variable %s", variable_compartida);
	enviar(kernel, SOLICITAR_VARIABLE, strlen(variable)+1, variable_compartida);
	paquete = recibir(kernel);
	memcpy(&valor, paquete->data, 4);
	log_info(logger, "El valor de la variable %s es %d", variable_compartida,valor);
	free(variable_compartida);
	liberar_paquete(paquete);
	return valor;
}

t_valor_variable asignarValorCompartida(t_nombre_compartida variable,t_valor_variable valor){
	char* variable_compartida = malloc(5+strlen(variable));
	char* barra_cero="\0";
	memcpy(variable_compartida, &valor, 4);
	memcpy(variable_compartida, &variable, strlen(variable));
	memcpy(variable_compartida+strlen(variable)+4, barra_cero,1);
	log_info(logger,"Variable compartida %s le asignamos valor %d", variable_compartida+4, (int*)variable_compartida[0]);
	enviar(kernel, ESCRIBIR_VARIABLE, 5+strlen(variable), variable_compartida);
	free(variable_compartida);
	return valor;
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


//Primitivas KERNEL
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

void signal_kernel(t_nombre_semaforo identificador_semaforo){
	printf("signal\n");
}

t_puntero reservarEnHeap(t_valor_variable espacio){
	printf("reservar en Heap\n");
	t_puntero * puntero;
	return puntero;
}

void liberarEnHeap(t_puntero puntero){
	printf("liberar\n");
}

t_descriptor_archivo abrirArchivo(t_direccion_archivo direccion, t_banderas flags){
	t_descriptor_archivo* archi;
	return archi;
}

void borrarArchivo(t_descriptor_archivo descriptor_archivo){

}

void cerrarArchivo(t_descriptor_archivo descriptor_archivo){

}

void moverCursor(t_descriptor_archivo descriptor_archivo, t_valor_variable posicion){

}

void escribirArchivo(t_descriptor_archivo descriptor_archivo, void* informacion, t_valor_variable tamanio){

}

void leerArchivo(t_descriptor_archivo descriptor_archivo, t_puntero informacion, t_valor_variable tamanio){

}

