#include "primitivas.h"

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

t_puntero definirVariable(t_nombre_variable identificador_variable) {


	//RESERVA EL ESPACIO EN MEMORIA? O EN TEORIA YA HAY ESPACIO RESERVADO EN MEMORIA PARA LA VARIABLE A DEFINIR?
	//QUE ONDA LA PARTE DEL DICCIONARIO?
	log_warning(logger, "definirVariable");

	programaAbortado = 0; //ESTO ESTA HARDCODEADO, ADAPTAR;

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
			direccionVariable = armarProximaDireccion();
			variable->etiqueta = identificador_variable;
			variable->direccion = direccionVariable;
			list_add(contexto->vars, variable);
			contexto->sizeVars++;
		}

		int* valor = (int*) 6451;
		log_info(logger,"Basura: %i", valor);
		int direccionRetorno = convertirDireccionAPuntero(direccionVariable);

		if(direccionRetorno + 3 > var_max) {
			log_error(logger, "STACK OVERFLOW");
			log_error(logger,"No hay espacio para definir variable '%c'. Abortando programa", identificador_variable);
			programaAbortado = true;
			return EXIT_FAILURE_CUSTOM;
		} else {
			// VER FEDE almacenarBytesEnMemoria(direccionVariable, valor);
			log_info(logger,"Direccion de retorno: %i", direccionRetorno);
			return direccionRetorno;
		}
	}

	log_error(logger, "No se puede definir varialbe por programa abortado");
	return EXIT_FAILURE_CUSTOM;
}

t_puntero obtenerPosicionVariable (t_nombre_variable identificador_variable){
	log_warning(logger, "obtenerPosicionVariable");
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
	log_warning(logger, "dereferenciar");
	log_info(logger, "Dereferenciando direccion de memoria %i", direccion_variable);

	t_direccion* direccion = convertirPunteroADireccion(direccion_variable);
	// VER FEDE solicitarBytesAMemoria(direccion);
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
	log_warning(logger, "asignar");
	log_info(logger, "Asigno el valor: %i a la variable en la posicion: %i", valor, direccion_variable);

	t_direccion* direccion = convertirPunteroADireccion(direccion_variable);

	log_info("Asignando valor %i en Pagina %i, Offset %i, Tamanio %i", direccion->pagina, direccion->offset, direccion->size);
	//VER FEDE almacenarBytesEnMemoria(direccion, valor); //VER SI TENGO QUE VERIFICAR QUE SE HAYA ALMACENADO OK

	free(direccion);
}

t_valor_variable obtenerValorCompartida(t_nombre_compartida variable){
	log_warning(logger, "obtenerValorCompartida");

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
	log_warning(logger, "asignarValorCompartida");

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
	log_warning(logger, "irALabel");
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
	log_warning(logger, "llamarConRetorno");
	t_direccion* direccion_nueva = convertirPunteroADireccion(donde_retornar);

	int posicionStack = pcb->sizeContextoActual;
	log_info(logger, "Tamanio contexto actual %i", pcb->sizeContextoActual);

	t_contexto* contexto_nuevo = malloc(sizeof(t_contexto));
	contexto_nuevo->pos = posicionStack;
	contexto_nuevo->args = list_create();
	contexto_nuevo->vars = list_create();
	contexto_nuevo->sizeArgs = 0;
	contexto_nuevo->sizeVars = 0;
	contexto_nuevo->retPos = pcb->programCounter; //VER QUE ONDA ACA
	contexto_nuevo->retVar = direccion_nueva;

	log_info(logger, "Creo nuevo contexto con posicion: %i que debe volver en la sentencia %i y retorno en la variable de posicion Pagina %i,  Offset %i",
			contexto_nuevo->pos, contexto_nuevo->retPos, contexto_nuevo->retVar->pagina, contexto_nuevo->retVar->offset);
	list_add(pcb->contextoActual, contexto_nuevo);
	pcb->sizeContextoActual++;

	irAlLabel(etiqueta);
}

void retornar(t_valor_variable retorno){
	log_warning(logger, "retornar");

	log_info(logger,"Posicion de retorno %d", retorno);
	int posConextoActual = pcb->sizeContextoActual - 1;
	t_contexto* contexto_final = list_get(pcb->contextoActual, posConextoActual);
	int direccion = convertirDireccionAPuntero(contexto_final->retVar);
	asignar(direccion,retorno);
	pcb->programCounter = contexto_final->retPos; //VER QUE ONDA ACA

	//Destruyo Contexto de Funcion

	while(contexto_final->sizeVars!=0){
		free(((t_variable*) list_get(contexto_final->vars, contexto_final->sizeVars-1))->direccion);
		free(list_get(contexto_final->vars, contexto_final->sizeVars-1));
		contexto_final->sizeVars--;
	}
	list_destroy(contexto_final->vars);
	log_info(logger,"Destrui vars de funcion");

	while(contexto_final->sizeArgs!=0){
		free((t_direccion*) list_get(contexto_final->args, contexto_final->sizeArgs-1));
		contexto_final->sizeArgs--;
	}

	list_destroy(contexto_final->args);
	log_info(logger,"Destrui args de funcion");


	free(list_get(pcb->contextoActual, pcb->sizeContextoActual-1));
	log_info(logger,"Contexto Destruido\n");
	pcb->sizeContextoActual--;
}

void finalizar(){
	log_warning(logger, "finalizar");

	t_contexto *contexto_a_finalizar = list_get(pcb->contextoActual, pcb->sizeContextoActual-1);

	while(contexto_a_finalizar->sizeVars != 0){
		t_variable * variable_borrar = (t_variable *)list_get(contexto_a_finalizar->vars, contexto_a_finalizar->sizeVars-1);

		log_info(logger,"Direccion: Pagina %i, Offset %i, Tamanio %i", variable_borrar->direccion->pagina, variable_borrar->direccion->offset, variable_borrar->direccion->size);
		free(variable_borrar->direccion);

		log_info(logger,"Etiqueta: %c", variable_borrar->etiqueta);

		//free(list_get(contexto_a_finalizar->vars, contexto_a_finalizar->sizeVars-1)); //FALTA LA MIERDA DE ESTE FREE :)
		free(variable_borrar); //ANTES ESTABA EL DE ARRIBA PERO ASI TAMBIEN DEBERIA ANDAR


		contexto_a_finalizar->sizeVars--;
	}

	list_destroy(contexto_a_finalizar->vars);
	log_info(logger,"Destrui la lista de vars");

	while(contexto_a_finalizar->sizeArgs != 0){
		free((t_direccion*)list_get(contexto_a_finalizar->args, contexto_a_finalizar->sizeArgs-1));
		contexto_a_finalizar->sizeArgs--;
	}

	list_destroy(contexto_a_finalizar->args);
	log_info(logger,"Destrui la lista de args\n");

	free(contexto_a_finalizar);

	pcb->sizeContextoActual--;
	log_info(logger,"El programa finalizo\n");

	programaFinalizado = true;

	int* ok = (int*) 1;
	enviar(kernel, PROGRAMA_FINALIZADO, sizeof(int), ok);
	//enviar(kernel, PROGRAMA_FINALIZADO, sizeof(int), (int*) programaFinalizado);

	destruirPCB(pcb);
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
	log_info(logger, "Primitiva Signal_Kernel");
	char* semaforo = malloc(strlen(identificador_semaforo)+1);
	char* barra_cero = "\0";
	memcpy(semaforo, identificador_semaforo, strlen(identificador_semaforo));
	memcpy(semaforo + strlen(identificador_semaforo), barra_cero, 1);

	log_debug(logger, "Realizando signal de semaforo %s", semaforo);
	enviar(kernel, LIBERAR_SEMAFORO, strlen(semaforo)+1, semaforo);
	free(semaforo);

	log_debug(logger, "Finalizo Signal_Kernel");
	return;
}

t_puntero reservarEnHeap(t_valor_variable espacio){
	log_info(logger, "Primitiva alocar");
	t_pedidoHeap* pedido = malloc(sizeof(t_pedidoHeap));
	t_paquete* paquete = malloc(sizeof(t_paquete));

	int pid = pcb->pid;

	pedido->pid = pid;
	pedido->tamanio = espacio;

	enviar(kernel, SOLICITAR_HEAP, sizeof(t_pedidoHeap), pedido);
	paquete = recibir(kernel);

	if(paquete->codigo_operacion == SOLICITAR_HEAP_FALLO){
		//VER ABORTAR PROCESO
	}

	t_direccion* dire = malloc(sizeof(t_direccion));
	dire = (t_direccion*)paquete->data;

	t_puntero puntero = dire->pagina * tamanio_pag + dire->offset;
	log_info(logger, "Puntero %d", puntero);
	return puntero;

}

void liberarEnHeap(t_puntero puntero) {

	t_liberarHeap* libera = malloc(sizeof(t_liberarHeap));
	t_paquete* paquete;
	int pagina = puntero / tamanio_pag;
	int offset = puntero - (pagina*tamanio_pag);


	void * resul = solicitarBytesAMemoria(memoria, logger, pcb->pid, pagina, offset, sizeof(t_heapMetadata));

	char* instruccion = *(char*)resul;

	int punteroHeap = atoi(instruccion);
	int paginaHeap = punteroHeap/tamanio_pag;
	int offsetHeap = punteroHeap - (paginaHeap * tamanio_pag);

	libera->pid = pcb->pid;
	libera->nroPagina = paginaHeap;
	libera->offset = offsetHeap;

	enviar(kernel, LIBERAR_HEAP, sizeof(t_liberarHeap), libera);

	paquete = recibir(kernel);
	if(paquete->codigo_operacion == LIBERAR_HEAP_OK){
		log_debug(logger, "Bloque Heap apuntando a %d liberado correctamente", punteroHeap);
	}
	else{
		log_warning(logger, "No se ha podido liberar el bloque Heap con puntero %d", punteroHeap);
	}
}

//Primitivas FS

t_descriptor_archivo abrirArchivo(t_direccion_archivo direccion, t_banderas flags){
	//Defino variables locales
	t_descriptor_archivo decriptorArchivo;
	t_envioDeDatosKernelFSAbrir* paquete;
	char* permisos = string_new();
	int pid;
	int resultado;
	//Obtengo el pid
	pid = pcb->pid;
	//Genero la cadena de permisos
	if(flags.creacion){
		string_append(&permisos,'c');
	}
	if(flags.escritura){
		string_append(&permisos,'r');
	}
	if(flags.lectura){
		string_append(&permisos,'w');
	}
	//Cargo los datos en el paquete
	paquete->pid = pid;
	paquete->path = direccion;
	paquete->permisos = permisos;
	//Envio los datos a kernel con el codigo
	log_info(logger, "Enviando datos al Kernel para abrir archivo con el PID %d.",pid);
	enviar(kernel,ABRIR_ARCHIVO,sizeof(t_envioDeDatosKernelFSAbrir),paquete);
	//recibo el resultado
	resultado = recibir(kernel);
	//Analizo el resultado
	if(resultado != ARCHIVO_ABIERTO){
		//Termina el proceso porque no se pudo abrir arhcivo
		log_info(logger, "Hubo un problema intentando abrir archivo con el PID %d.",pid);
		return 0;
	}
	//Deberia recibir de kernel el arch.
	t_descriptor_archivo* archi;
	log_info(logger, "Archivo abierto con el PID %d.",pid);
	return archi;
}

void borrarArchivo(t_descriptor_archivo descriptor_archivo){

}

void cerrarArchivo(t_descriptor_archivo descriptor_archivo){
	//Defino variables locales
	t_envioDeDatosKernelFSLecturaYEscritura* paquete;
	int pid;
	int resultado;
	//Obtengo el pid
	pid = pcb->pid;
	//Cargo los datos en el paquete
	paquete->pid = pid;
	paquete->fd = descriptor_archivo;
	//Envio los datos a kernel con el codigo
	log_info(logger, "Enviando datos al Kernel para cerrar archivo con el PID %d.",pid);
	enviar(kernel,CERRAR_ARCHIVO,sizeof(t_envioDeDatosKernelFSLecturaYEscritura),paquete);
	//Corto aca o espero el resultado?
}

void moverCursor(t_descriptor_archivo descriptor_archivo, t_valor_variable posicion){

}

void escribirArchivo(t_descriptor_archivo descriptor_archivo, void* informacion, t_valor_variable tamanio){

}

void leerArchivo(t_descriptor_archivo descriptor_archivo, t_puntero informacion, t_valor_variable tamanio){
	//Defino variables locales
	t_envioDeDatosKernelFSLecturaYEscritura* paquete;
	int pid;
	int resultado;
	//Obtengo el pid
	pid = pcb->pid;
	//Cargo los datos en el paquete
	paquete->pid = pid;
	paquete->fd = descriptor_archivo;
	paquete->tamanio = tamanio;
	paquete->offset = informacion;
	//Envio los datos a kernel con el codigo
	log_info(logger, "Enviando datos al Kernel para leer datos de archivo con el PID %d.",pid);
	enviar(kernel,OBTENER_DATOS,sizeof(t_envioDeDatosKernelFSLecturaYEscritura),paquete);
	//Recibo resultado de la operacion de lectura.
	resultado = recibir(kernel);
	//Ver que resultado espera desde kernel.
	if(resultado != 1){
		//Termina el proceso porque no se pudo leer el arhcivo.
		log_info(logger, "Hubo un problema intentando leer archivo con el PID %d.",pid);
	}
	//Recibo lo leido.
	char* datosLeidos = recibir(kernel);
	log_info(logger, "La informacion leida es: %s.",datosLeidos);
}

