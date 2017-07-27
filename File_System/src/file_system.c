
#include "file_system.h"


int main() {

	pthread_mutex_init(&solicitud_mutex,NULL);


	crearArchivoLog();
	config = cargarConfiguracion();
	iniciarMetadata();
	crearServidor();
	atenderPedidos();

	free(config->PUERTO_KERNEL);
	free(config->PUNTO_MONTAJE);
	free(config);


}

void crearArchivoLog(){
		logger = iniciarLog(ARCHIVOLOG,"File_System");
		log_info(logger, "Iniciando File_System. \n");
	}

t_config_FS* cargarConfiguracion() {
	log_info(logger,"Cargando archivo de configuracion 'file_system.config'...\n");


	t_config_FS * conf = malloc(sizeof(t_config_FS));
	t_config* config = config_create(getenv("archivo_configuracion_fs"));

	conf->PUERTO_KERNEL = malloc(strlen(config_get_string_value(config, "PUERTO"))+1);
	strcpy(conf->PUERTO_KERNEL, config_get_string_value(config, "PUERTO"));

	conf->PUNTO_MONTAJE = malloc(strlen(config_get_string_value(config,"PUNTO_MONTAJE"))+1);
	strcpy(conf->PUNTO_MONTAJE, config_get_string_value(config,"PUNTO_MONTAJE"));
	if(!string_ends_with(conf->PUNTO_MONTAJE, "/")) string_append(&conf->PUNTO_MONTAJE, "/");

	conf->TAMANIO_BLOQUES = config_get_int_value(config,"TAMANIO_BLOQUES");
	conf->CANTIDAD_BLOQUES = config_get_int_value(config,"CANTIDAD_BLOQUES");

	config_destroy(config);

	return conf;

}

void iniciarMetadata(){
	pathMetadata = string_new();
	string_append(&pathMetadata, config->PUNTO_MONTAJE);
	log_info(logger, "Path Metadata %s", pathMetadata);
	string_append(&pathMetadata, "Metadata");
	log_info(logger, "Path Metadata %s", pathMetadata);

	pathBloques=string_new();
	string_append(&pathBloques, config->PUNTO_MONTAJE);
	string_append(&pathBloques, "Bloques");
	log_info(logger, "Path Bloques %s", pathBloques);

	pathArchivos = string_new();
	string_append(&pathArchivos, config->PUNTO_MONTAJE);
	string_append(&pathArchivos, "Archivos");

	mkdirRecursivo(config->PUNTO_MONTAJE);
	mkdir(pathMetadata, 0777);
	mkdir(pathBloques, 0777);
	mkdir(pathArchivos, 0777);

	char *p = string_new();
	string_append(&p, pathMetadata);
	string_append(&p, "/Metadata.bin");

	if(existeArchivo(p)){
		t_config* configAux = config_create(p);
		int bloques = config_get_int_value(configAux, "CANTIDAD_BLOQUES");
		int size = config_get_int_value(configAux, "TAMANIO_BLOQUES");
		if(bloques != config->CANTIDAD_BLOQUES || size != config->TAMANIO_BLOQUES){
			log_error(logger, "Ya Existe un FS en ese punto de montaje con valores distintos");
			exit(1);
		}
		config_destroy(configAux);
	}
	free(p);

	pathMetadataArchivo = string_new();
	string_append(&pathMetadataArchivo, pathMetadata);
	string_append(&pathMetadataArchivo, "/Metadata.bin");

	FILE * metadata = fopen(pathMetadataArchivo, "w");
	fprintf(metadata, "TAMANIO_BLOQUES=%d\n", config->TAMANIO_BLOQUES);
	fprintf(metadata, "CANTIDAD_BLOQUES=%d\n", config->CANTIDAD_BLOQUES);
	fprintf(metadata, "MAGIC_NUMBER=SADICA\n");
	fclose(metadata);

	int sizeBitArray = config->CANTIDAD_BLOQUES / 8;
	if((sizeBitArray %8) !=0)
		sizeBitArray++;

	pathMetadataBitarray = string_new();
	string_append(&pathMetadataBitarray, pathMetadata);
	string_append(&pathMetadataBitarray, "/Bitmap.bin");

	if(existeArchivo(pathMetadataBitarray)){
		FILE * bitmap = fopen(pathMetadataBitarray, "rb");

		struct stat stats;
		fstat(fileno(bitmap), &stats);

		char* data = malloc(stats.st_size);
		fread(data, stats.st_size, 1, bitmap);

		fclose(bitmap);

		bitArray = bitarray_create_with_mode(data, stats.st_size, LSB_FIRST);

	}
	else{
		bitArray = bitarray_create_with_mode(string_repeat('\0', sizeBitArray), sizeBitArray, LSB_FIRST);

		FILE * bitmap = fopen(pathMetadataBitarray, "w");
		fwrite(bitArray->bitarray, sizeBitArray, 1, bitmap);
		fclose(bitmap);
	}

	int j;
	FILE* bloque;

	for(j=0; j<config->CANTIDAD_BLOQUES; j++){
		char* pathBloque = string_new();
		string_append(&pathBloque, pathBloques);
		string_append(&pathBloque, "/");
		string_append(&pathBloque, string_itoa(j));
		string_append(&pathBloque, ".bin");

		if(!existeArchivo(pathBloque)){
			bloque = fopen(pathBloque, "w");
			fwrite(string_repeat('\0', config->TAMANIO_BLOQUES), config->TAMANIO_BLOQUES, 1, bloque);
			fclose(bloque);
		}
		free(pathBloque);
	}

	log_debug(logger, "Se finalizo la creacion de Metadata");





}



void crearServidor(){
	log_info(logger, "Creando el socket Servidor");
	fileSystemServer = socket_escucha(ipFileSystem,config->PUERTO_KERNEL);
	listen(fileSystemServer, 1);
	log_debug(logger,"Socket %d creado y escuchando", socketKernel);
	socketKernel = aceptar_conexion(fileSystemServer);

	bool resultado = esperar_handshake(socketKernel,HandshakeFileSystemKernel);

	if(resultado){
		log_debug(logger,"Conexión aceptada del KERNEL %d!!", socketKernel);

	}
	else
	{
		log_error(logger,"Handshake fallo, se aborta conexion\n");

		exit (EXIT_FAILURE);
	}

}


void atenderPedidos(){
	while(1){
			t_paquete* paquete = malloc(sizeof(t_paquete));

			log_info(logger, "Esperando Pedido de Kernel");

			paquete = recibir(socketKernel);

			log_info(logger, "Se recibio paquete desde Kernel");



			pthread_mutex_lock(&solicitud_mutex);


			switch (paquete->codigo_operacion){

			case -1:
				log_error(logger, "Se desconecto KERNEL");
				close(socketKernel);
				liberar_paquete(paquete);
				exit(1);
				break;

			case VALIDAR_ARCHIVO:
				validarArchivo(paquete);
				pthread_mutex_unlock(&solicitud_mutex);
				break;

			case CREAR_ARCHIVO:
				crearArchivo(paquete);
				pthread_mutex_unlock(&solicitud_mutex);
				break;

			case BORRAR_ARCHIVO:
				borrarArchivo(paquete);
				pthread_mutex_unlock(&solicitud_mutex);
				break;

			case SOLICITUD_OBTENCION_DATOS:
				obtenerDatos(paquete);
				pthread_mutex_unlock(&solicitud_mutex);
				break;

			case GUARDAR_DATOS:
				guardarDatos(paquete);
				pthread_mutex_unlock(&solicitud_mutex);
				break;

			}
	}


}



void validarArchivo(t_paquete* paquete){
	log_info(logger, "Validar Archivo");
	char* pathAbsoluto = generarPathArchivo((char*)paquete->data);

	bool existe = existeArchivo(pathAbsoluto);

	free(pathAbsoluto);
	int mandar = 1;

	if(existe){
		log_debug(logger, "El archivo existe");
		enviar(socketKernel,VALIDAR_ARCHIVO_OK, sizeof(int), &mandar );

	}
	else{
		log_warning(logger, "El archivo no existe");
		enviar(socketKernel, VALIDAR_ARCHIVO_FALLO, sizeof(int), &mandar);
	}

	return;
}

void crearArchivo(t_paquete* paquete){


	log_info(logger, "Creando Archivo");
	char* pathAbsoluto = generarPathArchivo((char*)paquete->data);
	bool existe = existeArchivo(pathAbsoluto);

	if(!existe){
		int bloqueLibre = buscarBloqueLibre();

		if(bloqueLibre == -1){
			log_error(logger, "SIN ESPACIO EN FS");
			enviar(socketKernel, SIN_ESPACIO_FS, sizeof(int), &bloqueLibre);
			return;
		}

		escribirValorBitarray(1, bloqueLibre);

		char* subCarpetas = string_substring_until(pathAbsoluto, string_pos_char(pathAbsoluto, '/'));
		log_debug(logger, "Subcarpetas: %s", subCarpetas);
		mkdirRecursivo(subCarpetas);
		free(subCarpetas);

		FILE* archivo = fopen(pathAbsoluto, "a");

		fprintf(archivo, "TAMANIO=0\n");
		fprintf(archivo,"BLOQUES=[%d]\n", bloqueLibre);

		fclose(archivo);

		log_debug(logger, "Se creo el archivo %s,", pathAbsoluto);
	}
	free(pathAbsoluto);
	int a=1;
	enviar(socketKernel, CREAR_ARCHIVO_OK, sizeof(int), &a);

	return;
}

void borrarArchivo(t_paquete* paquete){
	log_info(logger, "Borrando Archivo");

	char* pathAbsoluto = generarPathArchivo((char*)paquete->data);

	if(!existeArchivo(pathAbsoluto)){
		log_error(logger, "NO EXISTE EL ARCHIVO");
		int b=1;
		enviar(socketKernel, BORRAR_ARCHIVO_FALLO, sizeof(int), &b);
	}
	else{
		t_config * data = config_create(pathAbsoluto);
		char** bloques = config_get_array_value(data, "BLOQUES");

		int j = 0;

		while(bloques[j] != NULL){
			log_info(logger, "Bloque a liberar: %d", atoi(bloques[j]));
			escribirValorBitarray(0, atoi(bloques[j]));
			j++;
		}

		unlink(pathAbsoluto);
		log_debug(logger, "ARCHIVO BORRADO CON EXITO");

		enviar(socketKernel, BORRAR_ARCHIVO_OK, sizeof(int), &j);

		config_destroy(data);
		free(pathAbsoluto);

	}
	return;
}

void obtenerDatos(t_paquete* paquete){
	log_info(logger, "OBTENER DATOS");

	t_paquete* paq = malloc(sizeof(t_paquete));
	paq = recibir(socketKernel);

	t_pedidoGuardadoDatos* pedido = malloc(sizeof(t_pedidoGuardadoDatos));

	pedido = (t_pedidoGuardadoDatos*)paq->data;





	char* path = generarPathArchivo((char*)paq->data);

	if(!existeArchivo(path)){
		int a=1;
		log_error(logger, "NO EXISTE EL ARCHIVO");
		enviar(socketKernel, SOLICITUD_OBTENCION_DATOS_FALLO, sizeof(int), &a);
		return;
	}

	char* buffer = malloc(sizeof(pedido->size));

	t_config* c = config_create(path);
	char** bloques = config_get_array_value(c, "BLOQUES");

	int offsetBloque, bytesLeidos = 0, restoBloque;

	offsetBloque = (pedido->offset % config->TAMANIO_BLOQUES);
	int numBloque = (pedido->offset / config->TAMANIO_BLOQUES);
	int j = numBloque;

	int bloque = atoi(bloques[numBloque]);

	while(bytesLeidos != pedido->size){
		restoBloque = pedido->size - bytesLeidos;

		if(restoBloque > config->TAMANIO_BLOQUES)
			restoBloque = config->TAMANIO_BLOQUES;

		log_info(logger, "Accedo al bloque %d", bloque);

		leerArchivo(bloque, buffer + bytesLeidos, restoBloque, offsetBloque);
		bytesLeidos += restoBloque;

		j++;

		if(bloques[j]!= NULL)
			bloque = atoi(bloques[j]);

		offsetBloque = 0;
	}


	enviar(socketKernel, SOLICITUD_OBTENCION_DATOS_OK, strlen(buffer)+1, buffer);

	free(path);
	free(pedido);
	liberar_paquete(paq);
	return;
}

void guardarDatos(t_paquete* paquete){

}


char* generarPathArchivo(char* path){

	log_info(logger, "Se genererara el Path Absoluto del archivo");
	char* pathAbs = string_new();
	string_append(&pathAbs, config->PUNTO_MONTAJE);
	string_append(&pathAbs, "Archivos");

	if(!string_starts_with(path, "/")) string_append(&pathAbs, "/");

	string_append(&pathAbs, path);


	return pathAbs;


}


void mkdirRecursivo(char* path){

	char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp),"%s",path);
    len = strlen(tmp);
    if(tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for(p = tmp + 1; *p; p++)
        if(*p == '/') {
        	*p = 0;
            mkdir(tmp, 0777);
            *p = '/';
        }
    mkdir(tmp, 0777);
}




bool existeArchivo(char* path){

	FILE * archi = fopen(path, "r");
	if(archi != NULL){
		fclose(archi);
		return true;
	}
	else{
		return false;
	}
}


int buscarBloqueLibre(){
	int bloqueLibre;

	for(bloqueLibre = 0; bitarray_test_bit(bitArray, bloqueLibre)&& bloqueLibre < config->CANTIDAD_BLOQUES; bloqueLibre++){

	}
	if(bloqueLibre >= config->CANTIDAD_BLOQUES)
		return -1;

	return bloqueLibre;

}

void escribirValorBitarray(bool valor, int pos){
	if(valor)
		bitarray_set_bit(bitArray, pos);
	else
		bitarray_clean_bit(bitArray, pos);

	FILE *bitmap = fopen(pathMetadataBitarray, "w");

	fwrite(bitArray->bitarray, bitArray->size, 1, bitmap);
	fclose(bitmap);
	return;
}

int string_pos_char(char* string, char caracter){
	int len = strlen(string), j;

	for(j=0;*(string+len-j) != caracter; j++){

	}
	return len - j ;
}

void leerArchivo(int bloque, char* buffer, int size, int offset){
	FILE* archivo = fopen(generarPathBloque(bloque), "r+");

	fseek(archivo, offset, SEEK_SET);
	fread(buffer, size, 1, archivo);

	fclose(archivo);

}


char* generarPathBloque(int num_bloque){
	char* path_bloque = string_new();
	string_append(&path_bloque, config->PUNTO_MONTAJE);
	string_append(&path_bloque, "Bloques/");
	string_append(&path_bloque, string_itoa(num_bloque));
	string_append(&path_bloque, ".bin");

	return path_bloque;
}










/*





void leerBitMap(){
	int fd;
	char *data;
	struct stat sbuf;
	char* rutaBitMap = string_duplicate(PUNTO_MONTAJE);
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



/*
 *
 *
 *
 *
 */



/*

void crearArchivo(void* path){
	int nroBloque = -1;
	int i;
	char* rutaMetadata = string_new();
	string_append(&rutaMetadata, PUNTO_MONTAJE);
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
	string_append(&rutaMetadata, PUNTO_MONTAJE);
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
	string_append(&rutaMedatada, PUNTO_MONTAJE);
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
		string_append(&pathBloque, PUNTO_MONTAJE);
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
	string_append(&rutaMetadata, PUNTO_MONTAJE);
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
		string_append(&pathBloque, PUNTO_MONTAJE);
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







/*
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

				//crearArchivo(path);
				enviar(socketKernel, CREAR_ARCHIVO, 0, 0);
				free(path);
				break;



			case BORRAR_ARCHIVO:
				path = malloc(paquete->tamanio);
				memcpy(path, paquete->data, paquete->tamanio);
				path[paquete->tamanio] = '\0';
				log_info(logger, "Path: %s", path);

				//borrarArchivo(path);
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
				//char* data = leerBloquesArchivo(path, offset, size);
				//enviar(socketKernel, OBTENER_DATOS, size, data);
				//free(data);
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

				//escribirBloquesArchivo(path, offset, size, buffer);
				enviar(socketKernel, GUARDAR_DATOS, 0, 0);
				//free(data);
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
		*/



