
#include "file_system.h"
int32_t PUERTO_KERNEL;
char* PUERTO_MONTAJE;
char* MAGIC_NUMBER;
int32_t TAMANIO_BLOQUES;
int32_t CANTIDAD_BLOQUES;

int main() {


	crearArchivoLog();
	cargarConfiguracion();
	crearBitArray();
	crearServidor();
	atenderPedidos();
}

void crearArchivoLog(){
		logger = iniciarLog(ARCHIVOLOG,"File_System");
		log_info(logger, "Iniciando File_System. \n");
	}

void cargarConfiguracion() {

	log_info(logger,"Cargando archivo de configuracion 'file_system.config'...\n");
	t_config* config = config_create(getenv("archivo_configuracion_fs"));
	t_config* config2 = config_create(getenv("metadata"));

	PUERTO_KERNEL = config_get_int_value(config, "PUERTO");
	PUERTO_MONTAJE = config_get_string_value(config, "PUNTO_MONTAJE");

	MAGIC_NUMBER = config_get_string_value(config2, "MAGIC_NUMBER");
	CANTIDAD_BLOQUES = config_get_int_value(config2, "CANTIDAD_BLOQUES");
	TAMANIO_BLOQUES = config_get_int_value(config2, "TAMANIO_BLOQUES");

	printf("CANTIDAD:%d\n", CANTIDAD_BLOQUES);

	if(config_has_property(config,"PUERTO") && config_has_property(config,"PUNTO_MONTAJE") && config_has_property(config2,"MAGIC_NUMBER") && config_has_property(config2,"CANTIDAD_BLOQUES") && config_has_property(config2,"TAMANIO_BLOQUES"))
		log_info(logger,"Archivo de configuracion cargado correctamente\n");
	else {
		log_error(logger,"Error al cargar archivo de configuracion");
		if(!config_has_property(config,"PUERTO"))
			log_error(logger,"No esta setteado el PUERTO\n");
		if(!config_has_property(config,"PUNTO_MONTAJE"))
			log_error(logger,"No esta setteado el Puerto Montaje: %s \n",PUERTO_MONTAJE);
		if(!config_has_property(config2, "MAGIC_NUMBER"))
			log_error(logger, "NO ESTA SETTEADO EL MAGIC NUMBER");
		if(!config_has_property(config2, "CANTIDAD_BLOQUES"))
			log_error(logger, "NO ESTA SETTEADO CANTIDAD_BLOQUES");
		if(!config_has_property(config2, "TAMANIO_BLOQUES"))
			log_error(logger, "NO ESTA SETTEADO TAMANIO_BLOQUES");
	}

	iniciarMetadataMap();
}


void iniciarMetadataMap(){
	int size;

	char* nombreArchivo = string_new();
	string_append(&nombreArchivo, PUERTO_MONTAJE);
	string_append(&nombreArchivo, "Metadata/Bitmap.bin");

	int bitmap = open(nombreArchivo, O_RDWR);
	struct stat mystat;

	if(fstat(bitmap, &mystat)<0)
		close(bitmap);

	mmapDeBitmap = mmap(NULL, mystat.st_size, PROT_WRITE | PROT_READ, MAP_SHARED, bitmap, 0);
	close(bitmap);

}

void crearBitArray(){
	printf("Entre\n");

	bitArray = bitarray_create_with_mode(mmapDeBitmap, (TAMANIO_BLOQUES * CANTIDAD_BLOQUES)/(8 * TAMANIO_BLOQUES), MSB_FIRST);

	printf("1\n");

	char* archi = string_new();
	printf("2\n");
	string_append(&archi, PUERTO_MONTAJE);
	printf("3\n");
	string_append(&archi, "Metadata/Bitmap.bin");
	printf("4\n");
	FILE *f;
	printf("5\n");
	f = open(archi, "wr+");
	printf("6\n");
	int32_t i;
	printf("7\n");
	printf("CANTIDAD: %d", CANTIDAD_BLOQUES);
	for(i=0; i < CANTIDAD_BLOQUES; i++){
		fputc(1,f);
	}
	printf("8\n");
	fclose(f);

	log_info(logger, "El tamano del bitarray es : %d ", bitarray_get_max_bit(bitArray));
	return;
}



int existeArchivo(char* path){

	char* rutaMetadata = string_new();
	string_append(&rutaMetadata, PUERTO_MONTAJE);
	string_append(&rutaMetadata, "/Archivos");
	string_append(&rutaMetadata, path);
	log_info(logger, "rutaMetadata %s", rutaMetadata);

	int fd = open(rutaMetadata, O_RDWR);

	if(fd > 0){
			close(fd);
			return 1;
		}else{
			close(fd);
			return 0;
	}
}
/*
void leerBitMap(){
	int fd;
	char *data;
	struct stat sbuf;
	char* rutaBitMap = string_duplicate(PUERTO_MONTAJE);
	string_append(&rutaBitMap, "/Metadata/Bitmap.bin"); //Como crear el Bitmap.bin
	log_info(logger, "rutaBitMap %s", rutaBitMap);

	fd = open(rutaBitMap, O_RDWR);
	if (fd < 0) {
		perror("error al abrir el archivo bitmap");
		terminarFileSystem();
	}

	if (stat(rutaBitMap, &sbuf) < 0) {
		perror("stat, fijarse si el archivo esta corrupto");
		terminarFileSystem();
	}

	data = mmap((caddr_t)0, sbuf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED) {
		perror("fallo el mmap del bitmap");
		terminarFileSystem();
	}
	bitMap = bitarray_create_with_mode(data, sbuf.st_size, MSB_FIRST);

	close(fd);
}
*/


void crearArchivo(void* path){
	int nroBloque = -1;
	int i;
	char* rutaMetadata = string_new();
	string_append(&rutaMetadata, PUERTO_MONTAJE);
	string_append(&rutaMetadata, "/Archivos");
	string_append(&rutaMetadata, path);
	log_info(logger, "rutaMetadata %s", rutaMetadata);

	for (i = 0; i < bitArray->size && nroBloque == -1; i++){
		if(!bitarray_test_bit(bitArray, i)){
			bitarray_set_bit(bitArray, i);
			nroBloque = i;
		}
	}

	if(nroBloque == -1){
		log_info(logger, "No hay bloques libres");
	}

	char* data = string_from_format("TAMANIO=1 BLOQUES=[%d]", nroBloque);

	system(string_from_format("touch %s", rutaMetadata));

	int fd = open(rutaMetadata, O_RDWR);
	if(fd < 0){
		log_info(logger, "No se creo el archivo");
	}
	write(fd, data, string_length(data));
	close(fd);
	free(rutaMetadata);
}

void borrarArchivo(void* path){
	int i;
	char* rutaMetadata = string_new();
	string_append(&rutaMetadata, PUERTO_MONTAJE);
	string_append(&rutaMetadata, "/Archivos");
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

	for(i = 0; i*TAMANIO_BLOQUES < tamanio; i++){
		int nroBloque =  atoi(bloques[i]);
		bitarray_clean_bit(bitArray, nroBloque);
	}

	config_destroy(metadata);
	system(string_from_format("rm -f %s", rutaMetadata));
	free(rutaMetadata);
	free(bloques);
}

char* leerBloquesArchivo(void* path, int offset, int size){
	char* data = malloc(size+1), *tmpdata, *pathBloque;
	int i, tmpoffset = 0;
	char* rutaMedatada = string_new();
	string_append(&rutaMedatada, PUERTO_MONTAJE);
	string_append(&rutaMedatada, "/Archivos");
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

	for(i = offset / TAMANIO_BLOQUES; i * TAMANIO_BLOQUES < tamanio; i ++, tmpoffset += TAMANIO_BLOQUES){
		pathBloque = string_new();
		string_append(&pathBloque, PUERTO_MONTAJE);
		string_append_with_format(&pathBloque, "/Bloques/%s.bin", bloques[i]);
		tmpdata = leerArchivo(pathBloque);

		if(size - tmpoffset > TAMANIO_BLOQUES){ //falta
			memcpy(data + tmpoffset, tmpdata, TAMANIO_BLOQUES);
		}else{
			memcpy(data + tmpoffset, tmpdata, size - tmpoffset);
			break;
		}

	}
	data[size] = '\0';
	log_info(logger, "Contenido: %s", data);

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
	if (fd < 0){
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
	string_append(&rutaMetadata, PUERTO_MONTAJE);
	string_append(&rutaMetadata, "/Archivos");
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

	for(i = offset / TAMANIO_BLOQUES; i * TAMANIO_BLOQUES < tamanio; i ++, tmpoffset += TAMANIO_BLOQUES){
		pathBloque = string_new();
		string_append(&pathBloque, PUERTO_MONTAJE);
		string_append_with_format(&pathBloque, "/Bloques/%s.bin", bloques[i]);
		tmpdata = leerArchivo(pathBloque);

		if(size - tmpoffset > TAMANIO_BLOQUES){//falta
			memcpy(tmpdata, buffer + tmpoffset, TAMANIO_BLOQUES);
		}else{
			memcpy(tmpdata, buffer + tmpoffset, size-tmpoffset);
		}
	}
	free(rutaMetadata);
	free(pathBloque);
	free(bloques);
}

void terminarFileSystem(){
	log_trace(logger, "Termino FileSystem");

	if(bitArray != NULL){
		bitarray_destroy(bitArray);
	}
	exit(1);
}


void crearServidor(){
	fileSystemServer = socket_escucha(ipFileSystem,PUERTO_KERNEL);
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

}


void atenderPedidos(){
	while(1){
			log_info(logger, "Esperando Pedido de Kernel");
			//Recibimos pedidos de kernel y se hace switch dependiendo operacion
			paquete = recibir(socketKernel);
			log_info(logger, "Se recibio paquete desde Kernel");
			void* buffer;
			char* path;
			int tmpsize = 0, tmpoffset = 0;
			t_num offset, size;
			t_num sizePath = 0;

			//char * codigoDeOperacion = getCodigoDeOperacion(paquete->codigo_operacion);
			pthread_mutex_lock(&solicitud_mutex);
			//log_info(logger, "Codigo de operacion FileSystem-Kernel: %s", codigoDeOperacion);

			switch (paquete->codigo_operacion)
			{
			case VALIDAR_ARCHIVO:
				path = malloc(paquete->tamanio);
				memcpy(path, paquete->data, paquete->tamanio);
				path[paquete->tamanio] = '\0';
				log_info(logger, "Path: %s", path);
				bool existe = false;
				int fd = open(path, O_RDONLY);

				if(existeArchivo(path)){
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
				path[paquete->tamanio] = '\0';
				log_info(logger, "Path: %s", path);

				crearArchivo(path);
				enviar(socketKernel, CREAR_ARCHIVO, 0, 0);
				free(path);
				break;
			case BORRAR_ARCHIVO:
				path = malloc(paquete->tamanio);
				memcpy(path, paquete->data, paquete->tamanio);
				path[paquete->tamanio] = '\0';
				log_info(logger, "Path: %s", path);

				borrarArchivo(path);
				enviar(socketKernel, BORRAR_ARCHIVO, 0, 0);
				free(path);
				break;
			case OBTENER_DATOS:
				memcpy(&sizePath, paquete->data + tmpoffset, tmpsize = sizeof(t_num));
				tmpoffset += tmpsize;
				path = malloc(sizePath + 1);
				memcpy(path, paquete->data + tmpoffset, tmpsize = sizePath);
				path[sizePath] = '\0';
				tmpoffset += tmpsize;
				memcpy(&offset, paquete->data + tmpoffset, tmpsize = sizeof(t_valor_variable));
				tmpoffset += tmpsize;
				memcpy(&size, paquete->data + tmpoffset, tmpsize = sizeof(t_valor_variable));
				tmpoffset += tmpsize;
				log_info(logger, "Path: %s - offset: %d - size: %d", path, offset, size);
				char* data = leerBloquesArchivo(path, offset, size);
				enviar(socketKernel, OBTENER_DATOS, size, data);
				free(data);
				break;
			case GUARDAR_DATOS:
				tmpoffset = 0;
				memcpy(&sizePath, paquete->data + tmpoffset, tmpsize = sizeof(t_num));
				tmpoffset += tmpsize;
				path = malloc(sizePath + 1);
				memcpy(path, paquete->data + tmpoffset, tmpsize = sizePath);
				path[sizePath] = '\0';
				tmpoffset += tmpsize;
				memcpy(&offset, paquete->data + tmpoffset, tmpsize = sizeof(t_valor_variable));
				tmpoffset += tmpsize;
				memcpy(&size, paquete->data + tmpoffset, tmpsize = sizeof(t_valor_variable));
				tmpoffset += tmpsize;
				buffer = malloc(size);
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
		return ;

}


