
#include "file_system.h"

int main() {

	crearArchivoLog();
	cargarConfiguracion();

	fileSystemServer = socket_escucha(ipFileSystem,puerto);
	listen(fileSystemServer, 1);
	log_info(logger,"Socket %d creado y escuchando", socketKernel);
	socketKernel = aceptar_conexion(fileSystemServer);

	bool resultado = esperar_handshake(socketKernel,HandshakeFileSystemKernel);

	if(resultado){
		log_info(logger,"Conexión aceptada del KERNEL %d!!", socketKernel);
		printf("Conexion aceptada del KERNEL %d \n",socketKernel);
	}
	else
	{
		log_info(logger,"Handshake fallo, se aborta conexion\n");
		printf("Conexion abortada\n");
		exit (EXIT_FAILURE);
	}
	while(1){
		log_info(logger, "Esperando Pedido de Kernel");
		//Recibimos pedidos de kernel y se hace switch dependiendo operacion
		paquete = recibir(socketKernel);
		log_info(logger, "Se recibio paquete desde Kernel");
		void* path, *buffer;
		int tmpsize = 0, tmpoffset = 0;
		t_num offset, size;

		//char * codigoDeOperacion = getCodigoDeOperacion(paquete->codigo_operacion);
		pthread_mutex_lock(&solicitud_mutex);
		//log_info(logger, "Codigo de operacion FileSystem-Kernel: %s", codigoDeOperacion);

		switch (paquete->codigo_operacion)
		{
		case VALIDAR_ARCHIVO:
			path = malloc(paquete->tamanio);
			memcpy(path, paquete->data, paquete->tamanio);
			log_info(logger, "Path: %s", path);
			bool existe = false;
			int fd = open(path, O_RDONLY); //Porq no toma O_RDONLY?

			if (fd > 0){
				existe = true;
			log_info(logger, "El archivo existe");
			}
			else{
				log_info(logger, "El archivo no existe");
				}

			enviar(socketKernel, VALIDAR_ARCHIVO, paquete->tamanio, &existe);
			close(fd);
			free(path);
			break;
		case CREAR_ARCHIVO:
			path = malloc(paquete->tamanio);
			memcpy(path, paquete->data, paquete->tamanio);
			log_info(logger, "Path: %s", path);

			crearArchivo(path);
			enviar(socketKernel, CREAR_ARCHIVO, 0, 0);
			free(path);
			break;
		case BORRAR_ARCHIVO:
			path = malloc(paquete->tamanio);
			memcpy(path, paquete->data, paquete->tamanio);
			log_info(logger, "Path: %s", path);

			borrarArchivo(path);
			enviar(socketKernel, BORRAR_ARCHIVO, 0, 0);
			free(path);
			break;
		case OBTENER_DATOS:
			tmpsize = paquete->tamanio - sizeof(t_num)*2;
			path = malloc(tmpsize);
			memcpy(path, paquete->data, tmpsize);
			tmpoffset += tmpsize;
			memcpy(&offset, paquete->data + tmpoffset, sizeof(t_num));
			tmpoffset += sizeof(t_num);
			memcpy(&size, paquete->data + tmpoffset, sizeof(t_num));
			log_info(logger, "Path: %s - offset: %d - size: %d", path, offset, size);
			char* data = leerBloquesArchivo(path, offset, size);
			enviar(socketKernel, OBTENER_DATOS, size, data);
			free(data);
			break;
		case GUARDAR_DATOS:
			memcpy(&tmpsize, paquete->data, sizeof(t_num));
			tmpoffset += sizeof(t_num);
			path = malloc(tmpsize);
			memcpy(path, paquete->data + tmpoffset, tmpsize);
			tmpoffset += tmpsize;
			memcpy(&offset, paquete->data + tmpoffset, sizeof(t_num));
			tmpoffset += sizeof(t_num);
			memcpy(&size, paquete->data + tmpoffset, sizeof(t_num));
			tmpoffset += sizeof(t_num);
			buffer = malloc(size);
			memcpy(buffer, paquete->data + tmpoffset, size);
			log_info(logger, "Path: %s - Offset: %d - Size: %d \n Buffer: %s", path, offset, size, buffer);

			escribirBloquesArchivo(path, offset, size, buffer);
			enviar(socketKernel, GUARDAR_DATOS, 0, 0);
			free(data);
			break;
		default:
				log_error(logger, "Se ha desconectado el Kernel");
				pthread_mutex_unlock(&solicitud_mutex);
		        exit(EXIT_FAILURE);
			break;
		}
		log_info(logger, "Finalizo solicitud de %d", socketKernel);
		pthread_mutex_unlock(&solicitud_mutex);

	}
	//Variante HILOS
	//pthread_create(&servidorConexionesKernel, NULL, hiloServidorKernel, NULL);
	//pthread_join(servidorConexionesKernel, NULL);
	return 0;
}

void crearArchivoLog(){
		logger = iniciarLog(ARCHIVOLOG,"File_System");
		log_info(logger, "Iniciando File_System. \n");
	}

void cargarConfiguracion() {

	log_info(logger,"Cargando archivo de configuracion 'file_system.config'...\n");
	t_config* config = config_create(getenv("archivo_configuracion_fs"));
	puerto = config_get_int_value(config, "PUERTO");
	puntoMontaje = config_get_string_value(config, "PUNTO_MONTAJE");

	if(config_has_property(config,"PUERTO") && config_has_property(config,"PUNTO_MONTAJE"))
		log_info(logger,"Archivo de configuracion cargado correctamente\n");
	else {
		log_error(logger,"Error al cargar archivo de configuracion");
		if(!config_has_property(config,"PUERTO"))
			log_error(logger,"No esta setteado el PUERTO\n");
		if(!config_has_property(config,"PUNTO_MONTAJE"))
			log_error(logger,"Punto Montaje: %s \n",puntoMontaje);
	}
}

/*
void* hiloServidorKernel(void *arg) {

	log_info(logger,"Hilo Servidor KERNEL\n");
	int socketCliente;
	int* socketClienteTemp;
	socketKernel = socket_escucha("127.0.0.1", puerto);
	log_info(logger,"Creacion socket servidor KERNEL exitosa\n");
	listen(socketKernel, 1024);
	while(1) {
		socketCliente = aceptar_conexion(socketKernel);
		log_info(logger,"Iniciando Handshake con KERNEL\n");
		bool resultado_hand = esperar_handshake(socketCliente,KernelValidacion);
		if(resultado_hand) {
			log_info(logger,"Conexión aceptada del KERNEL %d!!", socketCliente);
			printf("Conexion aceptada del KERNEL %d, esperando mensajes \n",socketCliente);
		}
		else {
			log_info(logger,"Handshake fallo, se aborta conexion\n");
			exit (EXIT_FAILURE);
		}
		socketClienteTemp = malloc(sizeof(int));
		*socketClienteTemp = socketCliente;
		pthread_t conexionKernel;
		pthread_create(&conexionKernel, NULL, hiloConexionKernel, (void*)socketClienteTemp);
	}

}

void* hiloConexionKernel(void* socket) {

	//File system espera recibir mensajes de kernel

	while(1) {
		char* buffer = malloc(1000);
		int bytesRecibidos = recv(*(int*)socket, buffer, 1000, 0);
		if (bytesRecibidos <= 0) {
			log_error(logger,"El proceso se desconecto\n");
			return 1;
		}
		buffer[bytesRecibidos] = '\0';
		log_info(logger,"Me llegaron %d bytes con %s, del KERNEL %d\n", bytesRecibidos, buffer,*(int*)socket);
		printf("Me llegaron %d bytes con %s, del KERNEL %d\n", bytesRecibidos, buffer,*(int*)socket);
		free(buffer);
	}

	return 0;
}
*/
void leerMetadataArchivo(){

	char* rutaMetadata = string_duplicate(puntoMontaje);
	string_append(&rutaMetadata, "/Metadata/Metadata.bin"); //Como crear el Metadata.bin
	log_info(logger, "rutaMetadata %s", rutaMetadata);

	t_config* metadata = config_create(rutaMetadata);
	if(metadata == NULL){
		log_error(logger, "No se encuentra metadata");
		fprintf(stderr, "No se encuentra metadata\n");
		exit(1);
	}
	tamanioBloques = config_get_int_value(metadata, "TAMANIO_BLOQUES"); //No toma tamanioBloques de file_system.h
													//TAMANIO_BLOQUES va a estar en el Metadata.bin
	cantidadBloques = config_get_int_value(metadata, "CANTIDAD_BLOQUES"); //No toma cantidadBloques de file_system.h
														//CANTIDAD_BLOQUES va a estar en el Metadata.bin
	if(!string_equals_ignore_case(config_get_string_value(metadata, "MAGIC_NUMBER") , "SADICA")){//MAGIC_NUMBER en Metadata.bin
		log_error(logger, "No es un FS SADICA");
		fprintf(stderr, "No es un FS SADICA\n");
		exit(1);
	}
	config_destroy(metadata);
}

void leerBitMap(){		// con q numero arranco bloques 0 o 1?	considero 1
	int fd;
	char *data;
	struct stat sbuf;
	char* rutaBitMap = string_duplicate(puntoMontaje);
	string_append(&rutaBitMap, "/Metadata/Bitmap.bin"); //Como crear el Bitmap.bin
	log_info(logger, "rutaBitMap %s", rutaBitMap);

	fd = open(rutaBitMap, O_RDWR); //Porq no reconoce O_RDWR?
	if (fd < 0) {
		perror("error al abrir el archivo bitmap");
		exit(1);
	}

	if (stat(rutaBitMap, &sbuf) < 0) {
		perror("stat, fijarse si el archivo esta corrupto");
		exit(1);
	}

	data = mmap((caddr_t)0, sbuf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED) {
		perror("fallo el mmap del bitmap");
		exit(1);
	}
	bitMap = bitarray_create_with_mode(data, sbuf.st_size, LSB_FIRST);	//No reconoce bitMap de file_system.h

	close(fd);
}

void crearArchivo(void* path){
	int nroBloque;
	char* rutaMetadata = string_new();
	string_append(&rutaMetadata, puntoMontaje);
	string_append(&rutaMetadata, path);
	log_info(logger, "rutaMetadata %s", rutaMetadata);

	for(nroBloque = 0; nroBloque < bitMap->size; nroBloque++){
		if(!bitarray_test_bit(bitMap, nroBloque)){
			bitarray_set_bit(bitMap, nroBloque);
			break;
		}
	}

	char* data = string_from_format("TAMANIO=1 BLOQUES=[%d]", nroBloque + 1);

	system(string_from_format("touch %s", rutaMetadata));

	int fd = open(rutaMetadata, O_RDWR);
	write(fd, data, string_length(data));
	close(fd);

	free(rutaMetadata);
}

void borrarArchivo(void* path){
	int i;
	char* rutaMetadata = string_new();
	string_append(&rutaMetadata, puntoMontaje);
	string_append(&rutaMetadata, path);
	log_info(logger, "rutaMetadata %s", rutaMetadata);

	t_config* metadata = config_create(rutaMetadata);
	if(metadata == NULL){
		log_error(logger, "No se encuentra metadata");
		fprintf(stderr, "No se encuentra archivo %s\n", rutaMetadata);
		free(rutaMetadata);
		free(metadata);
		return;
	}
	int tamanio = config_get_int_value(metadata, "TAMANIO");
	char* bloques = config_get_array_value(metadata, "BLOQUES");

	for(i = 0; i*tamanioBloques < tamanio; i++){
		int nroBloque =  atoi(bloques[i / tamanioBloques]);
		bitarray_clean_bit(bitMap, nroBloque - 1);
	}

	config_destroy(metadata);
	system(string_from_format("rm -f %s", rutaMetadata));
	free(rutaMetadata);
	free(bloques);
}

char* leerBloquesArchivo(void* path, int offset, int size){
	char* data = malloc(size), *tmpdata, *pathBloque;
	int i, tmpoffset = 0;
	char* rutaMedatada = string_new();
	string_append(&rutaMedatada, puntoMontaje);
	string_append(&rutaMedatada, path);

	log_info(logger, "rutaMetadata %s", rutaMedatada);

	t_config* metadata = config_create(rutaMedatada);
	if(metadata == NULL){
		log_error(logger, "No se encuentra metadata");
		fprintf(stderr, "No se encuentra archivo %s\n", rutaMedatada);
		free(rutaMedatada);
		free(metadata);
		return NULL;
	}
	int tamanio = config_get_int_value(metadata, "TAMANIO");
	char** bloques = config_get_array_value(metadata, "BLOQUES");

	for(i = offset / tamanioBloques; i < tamanio; i += tamanioBloques, tmpoffset += tamanioBloques){
		pathBloque = string_new();
		string_append(&pathBloque, puntoMontaje);
		string_append_with_format(&pathBloque, "/Bloques/%s.bin", bloques[i / tamanioBloques]);
		tmpdata = leerArchivo(pathBloque);

		if(size - tmpoffset > tamanioBloques){ //falta
			memcpy(data + tmpoffset, tmpdata, tamanioBloques);
		}else{
			memcpy(data + tmpoffset, tmpdata, size - tmpoffset);
			break;
		}

	}

	free(rutaMedatada);
	free(pathBloque);
	free(bloques);
	return data;
}

char* leerArchivo(void* path){
	int fd;
	char *data;
	struct stat sbuf;

	fd = open(path, O_RDWR);
	if (fd == -1){
		perror("error al abrir el archivo");
		return NULL;
	}
	if(stat(path, &sbuf) == -1){
		perror("stat, chequear si el archivo esta corrupto");
		return NULL;
	}
	data = mmap((caddr_t)0, sbuf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if(data == MAP_FAILED){
		perror("Fallo el mmap");
		return NULL;
	}
	close(fd);
	return data;
}

void escribirBloquesArchivo(void* path, int offset, int size, char* buffer){
	char *tmpdata, *pathBloque;
	int i, tmpoffset = 0;
	char* rutaMetadata = string_new();
	string_append(&rutaMetadata, puntoMontaje);
	string_append(&rutaMetadata, path);
	log_info(logger, "rutaMetadata %s", rutaMetadata);

	t_config* metadata = config_create(rutaMetadata);
	if(metadata == NULL){
		log_error(logger, "No se encuentra metadata");
		fprintf(stderr, "No se encuentra archivo %s\n", rutaMetadata);
		free(rutaMetadata);
		free(metadata);
		return;
	}
	int tamanio = config_get_int_value(metadata, "TAMANIO");
	char** bloques = config_get_array_value(metadata, "BLOQUES");

	for(i = offset / tamanioBloques; i < tamanio; i += tamanioBloques, tmpoffset += tamanioBloques){
		pathBloque = string_new();
		string_append(&pathBloque, puntoMontaje);
		string_append_with_format(&pathBloque, "/Bloques/%s.bin", bloques[i / tamanioBloques]);
		tmpdata = leerArchivo(pathBloque);

		if(size - tmpoffset > tamanioBloques){//falta
			memcpy(tmpdata, buffer + tmpoffset, tamanioBloques);
		}else{
			memcpy(tmpdata, buffer + tmpoffset, size-tmpoffset);
		}
	}
	free(rutaMetadata);
	free(pathBloque);
	free(bloques);
}


