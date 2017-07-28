/*
 * Commons_Kaprobo_Consola.c
 *
 *  Created on: 27/7/2017
 *      Author: utnso
 */

#include "Commons_Kaprobo_Consola.h";

//LOG
t_log* iniciarLog(char* nombreDelLog,char* nombreDelProceso) {
	remove(nombreDelLog);
	return log_create(nombreDelLog,nombreDelProceso,true,LOG_LEVEL_TRACE);
}


//SOCKETS


un_socket conectar_a(char *IP, char* Port) {

	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(IP);
	direccionServidor.sin_port = htons(Port);

	int socketCliente = socket(AF_INET, SOCK_STREAM, 0);

	if (connect(socketCliente, (void*) &direccionServidor, sizeof(direccionServidor)) !=0){
		return -1;
	}

	return socketCliente;

}

un_socket socket_escucha(char* IP, char* Port) {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	//direccionServidor.sin_addr.s_addr = inet_addr(IP);
	direccionServidor.sin_addr.s_addr = inet_addr(ip_local());
	direccionServidor.sin_port = htons(Port);

		int socketEscucha = socket(AF_INET, SOCK_STREAM, 0);

		int activado = 1;
		setsockopt(socketEscucha, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

		if (bind(socketEscucha, (void*) &direccionServidor, sizeof(direccionServidor)) != 0) {
			perror("Falló el bind");
			return 1;
		}

	return socketEscucha;
}

char* ip_local(){
	struct sockaddr_in host;
	char nombre[255], *ip;
	gethostname(nombre,255);
	host.sin_addr = *(struct in_addr*) gethostbyname(nombre)->h_addr;
	ip = inet_ntoa(host.sin_addr);
	printf("IP: %s\n\n", ip);

	return ip;

}

void enviar(un_socket socket_para_enviar, int codigo_operacion, int tamanio, void * data) {

	int tamanio_paquete = 2 * sizeof(int) + tamanio;
	void * buffer = malloc(tamanio_paquete);

	memcpy(buffer, &codigo_operacion, sizeof(int));
	memcpy(buffer + sizeof(int), &tamanio, sizeof(int));
	memcpy(buffer + 2 * sizeof(int), data, tamanio);

	send(socket_para_enviar, buffer, tamanio_paquete, MSG_WAITALL);

	free(buffer);

}

t_paquete* recibir(un_socket socket_para_recibir) {

	t_paquete * paquete_recibido = malloc(sizeof(t_paquete));
	int retorno = 0;
	retorno = recv(socket_para_recibir, &paquete_recibido->codigo_operacion, sizeof(int),
	MSG_WAITALL);

	if(retorno==0){
		paquete_recibido->codigo_operacion=-1;
		void * informacion_recibida = malloc(sizeof(int));
		paquete_recibido->data = informacion_recibida;
		return paquete_recibido;

	}
	recv(socket_para_recibir, &paquete_recibido->tamanio, sizeof(int),
	MSG_WAITALL);

	void * informacion_recibida = malloc(paquete_recibido->tamanio);

	recv(socket_para_recibir, informacion_recibida, paquete_recibido->tamanio,
	MSG_WAITALL);

	paquete_recibido->data = informacion_recibida;

	return paquete_recibido;
}

un_socket aceptar_conexion(un_socket socket_escuchador) {

	struct sockaddr_storage remoteaddr;

	socklen_t addrlen;

	addrlen = sizeof remoteaddr;

	un_socket nuevo_socket = accept(socket_escuchador,
			(struct sockaddr *) &remoteaddr, &addrlen);

	return nuevo_socket;
}

void liberar_paquete(t_paquete * paquete) {
	free(paquete->data);
	free(paquete);
}

bool realizar_handshake(un_socket socket_del_servidor,int codigo){

	char* mensaje = malloc(21);
	mensaje = "Inicio Autenticacion";
	enviar(socket_del_servidor, codigo, 21, mensaje);

	t_paquete * resultado_del_handhsake = recibir(socket_del_servidor);

	bool resultado = string_equals_ignore_case(
			(char *) resultado_del_handhsake->data, "Autenticado");

	liberar_paquete(resultado_del_handhsake);

	return resultado;

}

bool esperar_handshake(un_socket socket_del_cliente, int codigo) {
	t_paquete * paquete= recibir(socket_del_cliente);

	bool resultado = (string_equals_ignore_case(
			(char *) paquete->data, "Inicio autenticacion")&& paquete->codigo_operacion == codigo);

	liberar_paquete(paquete);

	if (resultado) {

		char * respuesta = malloc(12);
		respuesta = "Autenticado";
		enviar(socket_del_cliente, 1, 12, respuesta);

	} else {

		char * respuesta = malloc(6);
		respuesta = "Error";
		enviar(socket_del_cliente, 1, 6, respuesta);

	}

	return resultado;
}

int getParteEntera(double numeroDecimal) {
	return (int) numeroDecimal;
}

double getParteDecimal(double numeroDecimal) {
	return numeroDecimal - getParteEntera(numeroDecimal);
}
/*
int cantidadPaginasCodigo(char* codigo) {
	int cantidad = -1;

	cantidad = ceil((double)strlen(codigo)/(double)tamanioPagina);

	if(cantidad == -1) {
		perror("Error cantidad paginas codigo");
		return EXIT_FAILURE_CUSTOM;
	}

	return cantidad;
}
*/
int almacenarEnMemoria(un_socket socketMemoria, t_log* logger, int pid, int pagina, int offset, int tamanio, void* buffer) {
	void* bufferSerializado = malloc(sizeof(int) * 4 + tamanio);

	memcpy(bufferSerializado, &pid, sizeof(int));
	memcpy(bufferSerializado + sizeof(int), &pagina, sizeof(int));
	memcpy(bufferSerializado + sizeof(int) * 2, &offset, sizeof(int));
	memcpy(bufferSerializado + sizeof(int) * 3, &tamanio, sizeof(int));
	memcpy(bufferSerializado + sizeof(int) * 4, buffer, tamanio);

	enviar(socketMemoria, ALMACENAR_BYTES, sizeof(int) * 4 + tamanio, bufferSerializado);

	t_paquete* paqueteRecibido = recibir(socketMemoria);

	if(paqueteRecibido->codigo_operacion == ALMACENAR_BYTES_FALLO) {
		log_error(logger, "No se pudieron almacenar %i bytes del PID %i en Pagina %i con Offset %i", tamanio, pid, pagina, offset);
		return EXIT_FAILURE_CUSTOM;
	}

	log_debug(logger, "Almacenados %i bytes del PID %i en Pagina %i con Offset %i", tamanio, pid, pagina, offset);
	return EXIT_SUCCESS_CUSTOM;
}

void* solicitarBytesAMemoria(un_socket socketMemoria, t_log* logger, int pid, int pagina, int offset, int tamanio) {
	log_info(logger, "Solicitud de bytes en proceso, PID %i, Pagina %i, Offset %i, Tamanio %i", pid, pagina, offset, tamanio);

	t_solicitudBytes* solicitud = malloc(sizeof(t_solicitudBytes));
	solicitud->pid = pid;
	solicitud->pagina = pagina;
	solicitud->offset = offset;
	solicitud->tamanio = tamanio;

	enviar(socketMemoria, SOLICITAR_BYTES, sizeof(t_solicitudBytes), solicitud);

	free(solicitud);

	t_paquete* paquete = recibir(socketMemoria);

	if(paquete->codigo_operacion == SOLICITAR_BYTES_FALLO) {
		log_error(logger, "No se pudo cumplir la solicitud de bytes");
		log_error(logger, "PID %i, Pagina %i, Offset %i, Tamanio %i", pid, pagina, offset, tamanio);
		return EXIT_FAILURE_CUSTOM;
	}

	log_debug(logger, "Solicitud de bytes completada con exito. PID %i, Pagina %i, Offset %i, Tamanio %i", pid, pagina, offset, tamanio);
	return paquete->data;
}


/////////////////ESTRUCUTURAS

void destruirPCB(t_pcb* pcb){
	t_contexto* contextoParaFinalizar;

	while(pcb->sizeContextoActual != 0){
		contextoParaFinalizar = list_get(pcb->contextoActual, pcb->sizeContextoActual-1);


		while(contextoParaFinalizar->sizeVars != 0){
			t_direccion *temp = (((t_variable*)list_get(contextoParaFinalizar->vars, contextoParaFinalizar->sizeVars -1))->direccion);
			free(temp);
			free(list_get(contextoParaFinalizar->vars, contextoParaFinalizar->sizeVars-1));
			contextoParaFinalizar->sizeVars --;
		}

		while(contextoParaFinalizar->sizeArgs != 0){
			free((t_direccion*)list_get(contextoParaFinalizar->args, contextoParaFinalizar->sizeArgs-1));
			contextoParaFinalizar->sizeArgs --;
		}
		list_destroy(contextoParaFinalizar->vars);

		list_destroy(contextoParaFinalizar->args);

		free(list_get(pcb->contextoActual, pcb->sizeContextoActual-1));
		pcb->sizeContextoActual --;
	}

	list_destroy(pcb->contextoActual);

	free(pcb->indiceDeCodigo);

	free(pcb->indiceEtiquetas);

	free(pcb);
}


t_pcb* desserializarPCB(char* serializado){


	int  i, y;
	t_pcb* pcb;

	pcb= malloc(sizeof(t_pcb));
	memcpy(pcb, serializado, sizeof(t_pcb));
	serializado += sizeof(t_pcb);

	pcb->indiceDeCodigo = malloc(pcb->sizeIndiceDeCodigo * 2 * sizeof(int));
	memcpy(pcb->indiceDeCodigo, serializado, pcb->sizeIndiceDeCodigo * 2 * sizeof(int));
	serializado += pcb->sizeIndiceDeCodigo * 2 *sizeof(int);

	pcb->indiceEtiquetas = malloc(pcb->sizeIndiceEtiquetas * sizeof(char));
	memcpy(pcb->indiceEtiquetas, serializado, pcb->sizeIndiceEtiquetas * sizeof(char));
	serializado+= pcb->sizeIndiceEtiquetas * sizeof(char);

	pcb->contextoActual = list_create();

	for(i=0; i<pcb->sizeContextoActual; i++){

		t_contexto* contexto = malloc(sizeof(t_contexto));

		memcpy(contexto, serializado, sizeof(t_contexto));

		serializado += sizeof(t_contexto);

		contexto->vars = list_create();
		contexto->args = list_create();

		for(y=0;y<contexto->sizeArgs; y++){
			t_direccion* dir =malloc(sizeof(t_direccion));
			memcpy(dir, serializado, sizeof(t_direccion));
			serializado += sizeof(t_direccion);
			list_add(contexto->args, dir);
		}

		for(y=0; y< contexto->sizeVars; y++){
			t_variable* var = malloc(sizeof(t_variable));

			t_direccion* dire =malloc (sizeof(t_direccion));

			memcpy(var, serializado, sizeof(t_variable));
			serializado+= sizeof(t_variable);

			dire->offset = ((int*)(serializado))[0];
			dire->pagina = ((int*)(serializado))[2];
			dire->size = ((int*)(serializado))[1];

			var->direccion = dire;

			serializado += sizeof(t_direccion);

			list_add(contexto->vars, var);
		}
		list_add(pcb->contextoActual, contexto);
	}
	return pcb;
}


char *serializarPCB(t_pcb *pcb){

	int size = 0;

	char* retorno, *retornotemp;

	size+= sizeof(t_pcb);

	size+= pcb->sizeIndiceEtiquetas * sizeof(char);
	size+= pcb->sizeIndiceDeCodigo * 2 * sizeof(int);

	int i;

	for (i=0; i< pcb->sizeContextoActual; i++){
		size += sizeof(t_contexto);
		int y;
		t_contexto * contexto;
		contexto = list_get(pcb->contextoActual, i);

		for(y=0; y< contexto->sizeArgs; y++){
			size += sizeof(t_direccion);
		}
		for(y=0;y<contexto->sizeVars; y++){
			size+= sizeof(t_variable);

			size+= sizeof(t_direccion);
		}
	}

	retorno = malloc(size);
	retornotemp = retorno;
	pcb->sizeTotal = size;

	memcpy(retornotemp, pcb, sizeof(t_pcb));

	retornotemp += sizeof(t_pcb);

	memcpy(retornotemp, pcb->indiceDeCodigo, pcb->sizeIndiceDeCodigo * 2 * sizeof(int));
	retornotemp += pcb->sizeIndiceDeCodigo * 2 * sizeof(int);

	memcpy(retornotemp, pcb->indiceEtiquetas, pcb->sizeIndiceEtiquetas * sizeof(char));
	//retornotempp = retornotemp;

	retornotemp += pcb->sizeIndiceEtiquetas * sizeof(char);

	for(i=0; i< pcb->sizeContextoActual; i++){
		int y;
		t_contexto* contexto;
		contexto = list_get(pcb->contextoActual, i);
		memcpy(retornotemp, contexto, sizeof(t_contexto));

		retornotemp += sizeof(t_contexto);

		for(y=0; y<contexto->sizeArgs; y++){
			t_direccion* direccion;
			direccion = list_get(contexto->args, y);
			memcpy(retornotemp, direccion, sizeof(t_direccion));
			retornotemp+= sizeof(t_direccion);
		}

		for(y= 0; y<contexto->sizeVars; y++){
			t_variable* var;
			t_direccion* dir;

			var = list_get(contexto->vars, y);

			memcpy(retornotemp, var, sizeof(t_variable));

			retornotemp += sizeof(t_variable);
			t_direccion* diretemp = var->direccion;

			memcpy(retornotemp, &diretemp->offset,4);
			memcpy(retornotemp+4, &diretemp->size, 4);
			memcpy(retornotemp+8, &diretemp->pagina, 4);

			retornotemp+=sizeof(t_direccion);

		}

	}

	return retorno;
}


void destruirCONTEXTO(t_pcb * pcb){
 	t_contexto* contextoParaFinalizar;

 	while(pcb->sizeContextoActual !=0){
 		contextoParaFinalizar = list_get(pcb->contextoActual, pcb->sizeContextoActual -1);

 	while(contextoParaFinalizar->sizeVars !=0){
 			t_direccion * temporal = (((t_variable*)list_get(contextoParaFinalizar->vars, contextoParaFinalizar->sizeVars -1))->direccion);
 			free(temporal);
 			free(list_get(contextoParaFinalizar->vars, contextoParaFinalizar->sizeVars -1));
 			contextoParaFinalizar->sizeVars --;
 		}
 		while(contextoParaFinalizar->sizeArgs != 0){
 			free((t_direccion*)list_get(contextoParaFinalizar->args, contextoParaFinalizar->sizeArgs -1));
 			contextoParaFinalizar->sizeArgs --;
 		}
 		list_destroy(contextoParaFinalizar->vars);

 		list_destroy(contextoParaFinalizar->args);

 		free(list_get(pcb->contextoActual, pcb->sizeContextoActual -1));
 		pcb->sizeContextoActual --;
 	}
 	list_destroy(pcb->contextoActual);
 	free(pcb->indiceDeCodigo);
 	free(pcb->indiceEtiquetas);

 }
