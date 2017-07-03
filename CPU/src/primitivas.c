#include "primitivas.h"

t_puntero definirVariable(t_nombre_variable identificador_variable) {
	/*
	 * DEFINIR VARIABLE
	 *
	 * Reserva en el Contexto de Ejecución Actual el espacio necesario para una variable llamada identificador_variable y la registra tanto en el Stack como en el Diccionario de Variables. Retornando la posición del valor de esta nueva variable del stack
	 * El valor de la variable queda indefinido: no deberá inicializarlo con ningún valor default.
	 * Esta función se invoca una vez por variable, a pesar que este varias veces en una línea.
	 * Ej: Evaluar "variables a, b, c" llamará tres veces a esta función con los parámetros "a", "b" y "c"
	 *
	 * @sintax	TEXT_VARIABLE (variables)
	 * 			-- nota: Al menos un identificador; separado por comas
	 * @param	identificador_variable	Nombre de variable a definir
	 * @return	Puntero a la variable recien asignada
	 */

	//RESERVA EL ESPACIO EN MEMORIA? O EN TEORIA YA HAY ESPACIO RESERVADO EN MEMORIA PARA LA VARIABLE A DEFINIR?
	//QUE ONDA LA PARTE DEL DICCIONARIO?

	programaAbortado = false; //ESTO ESTA HARDCODEADO, ADAPTAR;

	if(!programaAbortado) {
		log_info(logger, "Definiendo variable: %c", identificador_variable);
		t_direccion* direccionVariable;
		t_variable* variable = malloc(sizeof(t_variable));
		t_contexto *contexto = malloc(sizeof(t_contexto));
		//int posicionStack = pcb->sizeContextoActual-1;
		contexto= (t_contexto*)(list_get(pcb->contextoActual, pcb->sizeContextoActual-1));

		if(pcb->sizeContextoActual == 1 &&  contexto->sizeVars == 0 ){
			direccionVariable = armarDireccionPrimeraPagina();
			variable->etiqueta = identificador_variable;
			variable->direccion = direccionVariable;
			list_add(contexto->vars, variable);
			contexto->pos = 0;
			contexto->sizeVars++;
		} else if((identificador_variable >= '0') && (identificador_variable <= '9')){
			log_info(logger, "Creando argumento %c", identificador_variable);
			direccionVariable = armarDireccionDeArgumento();
			list_add(contexto->args, direccionVariable);
			log_info(logger,"Direccion de argumento '%c'. Pagina %i, Offset %i, Size %i",
					identificador_variable,
					direccionVariable->pagina,
					direccionVariable->offset,
					direccionVariable->size);
			contexto->sizeArgs++;
		} else if(contexto->sizeVars == 0 && (pcb->sizeContextoActual) > 1){
			//La posicion va a estar definida cuando se llama a la primitiva funcion
			log_info(logger, "Declarando variable '%c' de funcion", identificador_variable);
			direccionVariable = armarDirecccionDeFuncion();
			variable->etiqueta = identificador_variable;
			variable->direccion = direccionVariable;
			list_add(contexto->vars, variable);
			contexto->sizeVars++;
		} else {
			direccionVariable = armarProximaDireccion(direccionVariable);
			variable->etiqueta = identificador_variable;
			variable->direccion = direccionVariable;
			list_add(contexto->vars, variable);
			contexto->sizeVars++;
		}

		int* valor = 6451;
		log_info(logger,"Basura: %i", valor);
		int direccionRetorno = convertirDireccionAPuntero(direccionVariable);

		if(direccionRetorno + 3 > var_max) {
			log_error(logger, "STACK OVERFLOW");
			log_error(logger,"No hay espacio para definir variable '%c'. Abortando programa", identificador_variable);
			programaAbortado = true;
			return EXIT_FAILURE_CUSTOM;
		} else {
			almacenarBytesEnMemoria(direccionVariable, valor);
			log_info(logger,"Direccion de retorno: %i", direccionRetorno);
			return direccionRetorno;
		}
	}

	log_error(logger, "No se puede definir varialbe por programa abortado");
	return EXIT_FAILURE_CUSTOM;
}

t_puntero obtenerPosicionVariable (t_nombre_variable identificador_variable){
	log_info(logger, "Obteniendo posicion variable: %c", identificador_variable);

	t_list* indiceStack = pcb->contextoActual;
	t_contexto* entradaActualStack = list_get(indiceStack, list_size(indiceStack) - 1);

	if((identificador_variable >= '0') && (identificador_variable <= '9')) {
		t_list* args = entradaActualStack->args;

		log_warning(logger, "Paso a int el identificador_varialbe '%c'",identificador_variable);
		int variableInt = identificador_variable - '0';
		log_warning(logger, "En int = %i", variableInt);

		t_direccion* direccion = (t_direccion*) list_get(args, variableInt);

		return direccion;
	} else {
		t_list* vars = entradaActualStack->vars;
		int i;

		for(i = 0; i <= list_size(vars); i++) {
			t_variable* variable = list_get(vars, i);
			if(variable->etiqueta == identificador_variable) {
				return variable->direccion;
			}
		}
	}

	log_error(logger, "No se puedo obtener posicion de variable '%c'", identificador_variable);

	return EXIT_FAILURE_CUSTOM;
}

t_valor_variable dereferenciar(t_puntero direccion_variable){
	log_info(logger, "Dereferenciando direccion de memoria %i", direccion_variable);

	t_direccion* direccion = convertirPunteroADireccion(direccion_variable);
	solicitarBytesAMemoria(direccion);
	t_paquete* paquete = recibir(memoria);

	if(paquete->codigo_operacion == SOLICITAR_BYTES_FALLO) {
		log_error(logger, "Error al dereferenciar(%i)", direccion_variable);
		return EXIT_FAILURE_CUSTOM;
	}

	int valor = (int) paquete->data;
	log_info(logger,"Valor dereferenciado: %d", valor);

	liberar_paquete(paquete);
	free(direccion);

	return valor;
}

void asignar(t_puntero direccion_variable, t_valor_variable valor){
	printf("Asigno el valor: %i a la variable en la posicion: %i", valor, direccion_variable);

	t_direccion* direccion = convertirPunteroADireccion(direccion_variable);

	log_info("Asignando valor %i en Pagina %i, Offset %i, Tamanio %i", direccion->pagina, direccion->offset, direccion->size);
	almacenarBytesEnMemoria(direccion, valor); //VER SI TENGO QUE VERIFICAR QUE SE HAYA ALMACENADO OK

	free(direccion);
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
	char* variableCompartida = malloc(5+strlen(variable));
	char* barraCero = "\0";
	memcpy(variableCompartida, &valor, 4);
	memcpy(variableCompartida, &variable, strlen(variable));
	memcpy(variableCompartida + strlen(variable) + 4, barraCero, 1);
	log_info(logger, "Variable compartida %s le asignamos valor %d", variableCompartida + 4, (int*) variableCompartida[0]);
	enviar(kernel, ESCRIBIR_VARIABLE, 5 + strlen(variable), variableCompartida);
	free(variableCompartida);
	return valor;
}

void irAlLabel(t_nombre_etiqueta etiqueta){
	log_info(logger, "Busco etiqueta: %s y mide: %i", etiqueta, strlen(etiqueta));
	t_puntero_instruccion instruccion = metadata_buscar_etiqueta(etiqueta, pcb->indiceEtiquetas, pcb->sizeIndiceEtiquetas);
	log_info(logger,"Ir a instruccion %i", instruccion);

	pcb->programCounter = instruccion - 1;

	log_info(logger,"Saliendo de label");
	return;
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

