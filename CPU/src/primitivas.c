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



	if(!programaAbortado) {
		log_info(logger, "Definiendo variable: %c", identificador_variable);
		t_direccion* direccionVariable = malloc(sizeof(t_direccion));
		t_variable* variable = malloc(sizeof(t_variable));
		t_contexto *contexto = malloc(sizeof(t_contexto));

		contexto= (t_contexto*)(list_get(pcb->contextoActual, pcb->sizeContextoActual-1));

		if(pcb->sizeContextoActual == 1 &&  contexto->sizeVars == 0 ){

			direccionVariable = armarDireccionPrimeraPagina();

			variable->etiqueta = identificador_variable;
			variable->direccion = direccionVariable;
			list_add(contexto->vars, variable);
			contexto->pos = 0;
			contexto->sizeVars++;

		} else if((identificador_variable >= '0') && (identificador_variable <= '9')){
			//ES ARGUMENTO

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


		t_puntero direccionRetorno = convertirDireccionAPuntero(direccionVariable);

		if(direccionRetorno + 3 > var_max) {
			int a;
			log_error(logger, "STACK OVERFLOW");
			log_error(logger,"No hay espacio para definir variable '%c'. Abortando programa", identificador_variable);
			enviar(kernel, ABORTADO_STACKOVERFLOW, sizeof(int), a);

			programaAbortado = true;
			flagStackOverflow = 1;
			//return EXIT_FAILURE_CUSTOM;
		} else {
			int valor;
			log_info("Basura: %d", valor);
			almacenarEnMemoria(memoria, logger, pcb->pid, direccionVariable->pagina,direccionVariable->offset, direccionVariable->size, valor );
			log_info(logger,"Direccion de retorno: %d", direccionRetorno);
			return direccionRetorno;
		}
	}

	log_error(logger, "No se puede definir varialbe por programa abortado");
	return EXIT_FAILURE_CUSTOM;
}

/*
* OBTENER POSICION de una VARIABLE
*
* Devuelve el desplazamiento respecto al inicio del segmento Stacken que se encuentra el valor de la variable identificador_variable del contexto actual.
* En caso de error, retorna -1.
*
* @sintax	TEXT_REFERENCE_OP (&)
* @param	identificador_variable 		Nombre de la variable a buscar (De ser un parametro, se invocara sin el '$')
* @return	Donde se encuentre la variable buscada
*/
t_puntero obtenerPosicionVariable (t_nombre_variable identificador_variable){
	log_debug(logger, "Obtener posicion de Variable %c", identificador_variable);

	int posicionStack = pcb->sizeContextoActual-1;
	t_puntero direccionRetorno;
	if((identificador_variable>= '0')&&(identificador_variable <='9')){
		t_direccion* direccion = (t_direccion*)(list_get(((t_contexto*)(list_get(pcb->contextoActual, posicionStack)))->args, (int)identificador_variable-48));
		direccionRetorno = convertirDireccionAPuntero(direccion);
		log_debug(logger, "Obtener valor de %c: pagina: %d, offset: %d, size:%d",identificador_variable, direccion->pagina, direccion->offset, direccion->size);
		return(direccionRetorno);
	}
	else{
		t_variable* variable_nueva;
		int posMax = (((t_contexto*)(list_get(pcb->contextoActual, posicionStack)))->sizeVars)-1;
		while(posMax>=0){
			variable_nueva=((t_variable*)(list_get(((t_contexto*)(list_get(pcb->contextoActual, posicionStack)))->vars, posMax)));
			log_debug(logger,"Variable %c", variable_nueva->etiqueta);
			if(variable_nueva->etiqueta == identificador_variable){
				direccionRetorno = convertirDireccionAPuntero(((t_variable*)(list_get(((t_contexto*)(list_get(pcb->contextoActual, posicionStack)))->vars, posMax)))->direccion);
				log_debug(logger, "Obtengo valor de %c, pag: %d, offset: %d, size:%d", variable_nueva->etiqueta, variable_nueva->direccion->pagina, variable_nueva->direccion->offset, variable_nueva->direccion->size);
				return(direccionRetorno);
			}
			posMax--;
		}
	}
	log_warning(logger, "Error MEMORIA PID: %d", pcb->pid);
	enviar(kernel,EXCEPCION_MEMORIA, sizeof(int), (void*)pcb->pid );
	return -1;
}

/*
* DEREFERENCIAR
*
* Obtiene el valor resultante de leer a partir de direccion_variable, sin importar cual fuera el contexto actual
*
* @sintax	TEXT_DEREFERENCE_OP (*)
* @param	direccion_variable	Lugar donde buscar
* @return	Valor que se encuentra en esa posicion
*/
t_valor_variable dereferenciar(t_puntero direccion_variable){

	log_info(logger, "Dereferenciando direccion de memoria %i", direccion_variable);


	t_direccion* direccion = convertirPunteroADireccion(direccion_variable);
	int valor;

	memcpy(&valor, (solicitarBytesAMemoria(memoria, logger, pcb->pid, direccion->pagina, direccion->offset, direccion->size)), 4);



	log_info(logger,"Valor dereferenciado: %d", valor);


	free(direccion);

	return valor;
}

/*
* ASIGNAR
*
* Inserta una copia del valor en la variable ubicada en direccion_variable.
*
* @sintax	TEXT_ASSIGNATION (=)
* @param	direccion_variable	lugar donde insertar el valor
* @param	valor	Valor a insertar
* @return	void
*/
void asignar(t_puntero direccion_variable, t_valor_variable valor){
	log_warning(logger, "asignar");
	log_info(logger, "Asigno el valor: %d a la variable en la posicion: %d", valor, direccion_variable);
	t_direccion* direccion = convertirPunteroADireccion(direccion_variable);
	log_info(logger, "Asignando valor %i en Pagina %i, Offset %i, Tamanio %i", valor, direccion->pagina, direccion->offset, direccion->size);
	almacenarEnMemoria(memoria, logger, pcb->pid, direccion->pagina, direccion->offset, direccion->size, &valor);
	free(direccion);
	return;
}

/*
* OBTENER VALOR de una variable COMPARTIDA
*
* Pide al kernel el valor (copia, no puntero) de la variable compartida por parametro.
*
* @sintax	TEXT_VAR_START_GLOBAL (!)
* @param	variable	Nombre de la variable compartida a buscar
* @return	El valor de la variable compartida
*/
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

/*
* ASIGNAR VALOR a variable COMPARTIDA
*
* Pide al kernel asignar el valor a la variable compartida.
* Devuelve el valor asignado.
*
* @sintax	TEXT_VAR_START_GLOBAL (!) IDENTIFICADOR TEXT_ASSIGNATION (=) EXPRESION
* @param	variable	Nombre (sin el '!') de la variable a pedir
* @param	valor	Valor que se le quire asignar
* @return	Valor que se asigno
*/
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

/*
* IR a la ETIQUETA
*
* Cambia la linea de ejecucion a la correspondiente de la etiqueta buscada.
*
* @sintax	TEXT_GOTO (goto)
* @param	t_nombre_etiqueta	Nombre de la etiqueta
* @return	void
*/
void irAlLabel(t_nombre_etiqueta etiqueta){
	log_warning(logger, "irALabel");
	log_info(logger, "Busco etiqueta: %s y mide: %i", etiqueta, strlen(etiqueta));
	t_puntero_instruccion instruccion = metadata_buscar_etiqueta(etiqueta, pcb->indiceEtiquetas, pcb->sizeIndiceEtiquetas);
	log_info(logger,"Ir a instruccion %i", instruccion);

	pcb->programCounter = instruccion - 1;

	log_info(logger,"Saliendo de label");
	return;
}

/*
* LLAMAR SIN RETORNO
*
* Preserva el contexto de ejecución actual para poder retornar luego al mismo.
* Modifica las estructuras correspondientes para mostrar un nuevo contexto vacío.
*
* Los parámetros serán definidos luego de esta instrucción de la misma manera que una variable local, con identificadores numéricos empezando por el 0.
*
* @sintax	Sin sintaxis particular, se invoca cuando no coresponde a ninguna de las otras reglas sintacticas
* @param	etiqueta	Nombre de la funcion
* @return	void
*/
void llamarSinRetorno(t_nombre_etiqueta etiqueta){
	log_info(logger, "llamarSinRetorno");

	int posicionStack = pcb->sizeContextoActual;

	log_info(logger, "Tamanio contexto actual %i", pcb->sizeContextoActual);

	t_contexto* contexto_nuevo = malloc(sizeof(t_contexto));
	contexto_nuevo->pos = posicionStack;
	contexto_nuevo->args = list_create();
	contexto_nuevo->vars = list_create();
	contexto_nuevo->sizeArgs = 0;
	contexto_nuevo->sizeVars = 0;
	contexto_nuevo->retPos = pcb->programCounter;
	contexto_nuevo->retVar = NULL;

	list_add(pcb->contextoActual, contexto_nuevo);

	pcb->sizeContextoActual++;

	irAlLabel(etiqueta);

}

/*
* LLAMAR CON RETORNO
*
* Preserva el contexto de ejecución actual para poder retornar luego al mismo, junto con la posicion de la variable entregada por donde_retornar.
* Modifica las estructuras correspondientes para mostrar un nuevo contexto vacío.
*
* Los parámetros serán definidos luego de esta instrucción de la misma manera que una variable local, con identificadores numéricos empezando por el 0.
*
* @sintax	TEXT_CALL (<-)
* @param	etiqueta	Nombre de la funcion
* @param	donde_retornar	Posicion donde insertar el valor de retorno
* @return	void
*/
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
	contexto_nuevo->retPos = pcb->programCounter;
	contexto_nuevo->retVar = direccion_nueva;

	log_info(logger, "Creo nuevo contexto con posicion: %i que debe volver en la sentencia %i y retorno en la variable de posicion Pagina %i,  Offset %i",
			contexto_nuevo->pos, contexto_nuevo->retPos, contexto_nuevo->retVar->pagina, contexto_nuevo->retVar->offset);

	list_add(pcb->contextoActual, contexto_nuevo);
	pcb->sizeContextoActual++;

	irAlLabel(etiqueta);

}

/*
* RETORNAR
*
* Cambia el Contexto de Ejecución Actual para volver al Contexto anterior al que se está ejecutando, recuperando el Cursor de Contexto Actual, el Program Counter y la direccion donde retornar, asignando el valor de retorno en esta, previamente apilados en el Stack.
*
* @sintax	TEXT_RETURN (return)
* @param	retorno	Valor a ingresar en la posicion corespondiente
* @return	void
*/
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

/*
* FINALIZAR
*
* Cambia el Contexto de Ejecución Actual para volver al Contexto anterior al que se está ejecutando, recuperando el Cursor de Contexto Actual y el Program Counter previamente apilados en el Stack.
* En caso de estar finalizando el Contexto principal (el ubicado al inicio del Stack), deberá finalizar la ejecución del programa.
*
* @sintax	TEXT_END (end)
* @param	void
* @return	void
*/
void finalizar(){
	log_debug(logger, "Finalizar");

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

	programaFinalizado = true;

	void* ok = malloc(sizeof(int));
	enviar(kernel, PROGRAMA_FINALIZADO, sizeof(int), ok);
	destruirPCB(pcb);

}


//Primitivas KERNEL

/*
* WAIT
*
* Informa al kernel que ejecute la función wait para el semáforo con el nombre identificador_semaforo.
* El kernel deberá decidir si bloquearlo o no.
*
* @sintax	TEXT_WAIT (wait)
* @param	identificador_semaforo	Semaforo a aplicar WAIT
* @return	void
*/
void wait_kernel(t_nombre_semaforo identificador_semaforo){
	int size = strlen(identificador_semaforo);
	log_info(logger,"Tamanio semaforo %d", size);
	size++;
	char* nombre_semaforo = malloc(strlen(identificador_semaforo)+1);

	nombre_semaforo = identificador_semaforo;
	//memcpy(nombre_semaforo, identificador_semaforo, strlen(identificador_semaforo));


	nombre_semaforo[strlen(identificador_semaforo)] = '\0';




	log_info(logger,"CPU: Pedir semaforo %s de tamanio %d", identificador_semaforo, size);

	enviar(kernel,PEDIR_SEMAFORO, size, identificador_semaforo);
	t_paquete* paquete_semaforo;
	paquete_semaforo= recibir(kernel);
	memcpy(&programaBloqueado, paquete_semaforo->data, 4); //Me trae un 0 si no bloquea y un 1 si bloquea el proceso

	log_info(logger, "ProgramaBloqueado = %d", programaBloqueado);
	//free(nombre_semaforo);// VER ME TIRA ERROR

	liberar_paquete(paquete_semaforo);
	log_info(logger, "Saliendo del wait");
	return;
}

/*
* SIGNAL
*
* Informa al kernel que ejecute la función signal para el semáforo con el nombre identificador_semaforo.
* El kernel deberá decidir si desbloquear otros procesos o no.
*
* @sintax	TEXT_SIGNAL (signal)
* @param	identificador_semaforo	Semaforo a aplicar SIGNAL
* @return	void
*/
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

/*
* RESERVAR MEMORIA
*
* Informa al kernel que reserve en el Heap una cantidad de memoria
* acorde al espacio recibido como parametro.
*
* @sintax	TEXT_MALLOC (alocar)
* @param	valor_variable Cantidad de espacio
* @return	puntero a donde esta reservada la memoria
*/
t_puntero reservarEnHeap(t_valor_variable espacio){
	log_info(logger, "Primitiva alocar");
	t_pedidoHeap* pedido = malloc(sizeof(t_pedidoHeap));
	t_paquete* paquete = malloc(sizeof(t_paquete));

	int pid = pcb->pid;

	pedido->pid = pid;
	pedido->tamanio = espacio;

	enviar(kernel, SOLICITAR_HEAP, sizeof(t_pedidoHeap), pedido);
	paquete = recibir(kernel);

	t_direccion* dire = paquete->data;


	if(paquete->codigo_operacion == SOLICITAR_HEAP_FALLO){
		//VER ABORTAR PROCESO
	}

	t_puntero puntero = convertirDireccionAPuntero(paquete->data);
/*
	t_direccion* dire = malloc(sizeof(t_direccion));
	dire = (t_direccion*)paquete->data;

	t_puntero puntero = dire->pagina * tamanio_pag + dire->offset;
	*/
	log_info(logger, "Puntero %d", puntero);
	return puntero;

}

/*
* LIBERAR MEMORIA
*
* Informa al kernel que libera la memoria previamente reservada con RESERVAR.
* Solo se podra liberar memoria previamente asignada con RESERVAR.
*
* @sintax	TEXT_FREE (liberar)
* @param	puntero Inicio de espacio de memoria a liberar (previamente retornado por RESERVAR)
* @return	void
*/
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

/*
* ABRIR ARCHIVO
*
* Informa al Kernel que el proceso requiere que se abra un archivo.
*
* @syntax 	TEXT_OPEN_FILE (abrir)
* @param	direccion		Ruta al archivo a abrir
* @param	banderas		String que contiene los permisos con los que se abre el archivo
* @return	El valor del descriptor de archivo abierto por el sistema
*/
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


/*
* BORRAR ARCHIVO
*
* Informa al Kernel que el proceso requiere que se borre un archivo.
*
* @syntax 	TEXT_DELETE_FILE (borrar)
* @param	direccion		Ruta al archivo a abrir
* @return	void
*/
void borrarArchivo(t_descriptor_archivo descriptor_archivo){

	enviar(kernel, BORRAR_ARCHIVO, sizeof(t_descriptor_archivo), &descriptor_archivo);

	t_paquete* paquete = recibir(kernel);

	if(paquete->codigo_operacion== BORRAR_ARCHIVO_OK){
		log_debug(logger, "El PID %d ha borrado el archivo con FD %d", pcb->pid, &descriptor_archivo);
	}
	else{
		log_error(logger, "ERROR AL BORRAR ARCHIVO");
		//VER ABORTAR
	}

}

/*
* CERRAR ARCHIVO
*
* Informa al Kernel que el proceso requiere que se cierre un archivo.
*
* @syntax 	TEXT_CLOSE_FILE (cerrar)
* @param	descriptor_archivo		Descriptor de archivo del archivo abierto
* @return	void
*/
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

/*
* MOVER CURSOR DE ARCHIVO
*
* Informa al Kernel que el proceso requiere que se mueva el cursor a la posicion indicada.
*
* @syntax 	TEXT_SEEK_FILE (buscar)
* @param	descriptor_archivo		Descriptor de archivo del archivo abierto
* @param	posicion			Posicion a donde mover el cursor
* @return	void
*/
void moverCursor(t_descriptor_archivo descriptor_archivo, t_valor_variable posicion){

	t_moverCursor* mover = malloc(sizeof(t_moverCursor));

	mover->fd = descriptor_archivo;
	mover->pid=pcb->pid;
	mover->posicion = posicion;

	enviar(kernel, MOVER_CURSOR_ARCHIVO, sizeof(t_moverCursor), mover);

	t_paquete* rta = recibir(kernel);

	if(rta->codigo_operacion==MOVER_CURSOR_ARCHIVO_OK){
		log_debug(logger, "El PID %d ha movido el cursor del FD %d en la posicion %d", pcb->pid, descriptor_archivo,posicion);
	}
	else{
		log_error(logger,"Error al mover el cursor del FD %d", descriptor_archivo);
		//VER ABORTAR
	}

}


/*
* ESCRIBIR ARCHIVO
*
* Informa al Kernel que el proceso requiere que se escriba un archivo previamente abierto.
* El mismo escribira "tamanio" de bytes de "informacion" luego del cursor
* No es necesario mover el cursor luego de esta operación
*
* @syntax 	TEXT_WRITE_FILE (escribir)
* @param	descriptor_archivo		Descriptor de archivo del archivo abierto
* @param	informacion			Informacion a ser escrita
* @param	tamanio				Tamanio de la informacion a enviar
* @return	void
*/
void escribirArchivo(t_descriptor_archivo descriptor_archivo, void* informacion, t_valor_variable tamanio){
	log_debug(logger, "Primitiva escribir Archivo");
	log_debug(logger, "FD: %d, info: %s", descriptor_archivo, (char*)informacion);


	t_escribirArchivo* escribir = malloc(sizeof(t_escribirArchivo));

	escribir->pid = pcb->pid;
	escribir->fd = descriptor_archivo;
	escribir->size = tamanio;
	escribir->info = informacion;
	char* texto = malloc(tamanio);
	memcpy(texto, (char*)informacion, tamanio);

	enviar(kernel, ESCRIBIR_ARCHIVO, sizeof(t_escribirArchivo), escribir);
	enviar(kernel, 1, tamanio, texto);
	t_paquete* paquete = recibir(kernel);

	if(paquete->codigo_operacion== ESCRIBIR_ARCHIVO_OK){
		log_debug(logger, "Se escribio correctamente el archivo");
	}
	else{
		log_error(logger, "Hubo un error");
		//ver abortar
	}

}

/*
* LEER ARCHIVO
*
* Informa al Kernel que el proceso requiere que se lea un archivo previamente abierto.
* El mismo leera "tamanio" de bytes luego del cursor.
* No es necesario mover el cursor luego de esta operación
*
* @syntax 	TEXT_READ_FILE (leer)
* @param	descriptor_archivo		Descriptor de archivo del archivo abierto
* @param	informacion			Puntero que indica donde se guarda la informacion leida
* @param	tamanio				Tamanio de la informacion a leer
* @return	void
*/
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

