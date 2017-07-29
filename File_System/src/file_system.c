
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

	return 1;


}

void crearArchivoLog(){
		logger = iniciarLog(ARCHIVOLOG,"File_System");
		log_info(logger, "Iniciando File_System. \n");
	}

t_config_FS* cargarConfiguracion() {
	log_info(logger,"Cargando archivo de configuracion 'file_system.config'...\n");


	t_config_FS * conf = malloc(sizeof(t_config_FS));
	//t_config* config = config_create(getenv("archivo_configuracion_fs"));
	t_config* config = config_create("file_system.config");
	conf->PUERTO_KERNEL = config_get_int_value(config,"PUERTO");

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
	string_append(&pathMetadata, "Metadata");
	log_info(logger, "Path Metadata %s", pathMetadata);

	pathBloques=string_new();
	string_append(&pathBloques, config->PUNTO_MONTAJE);
	string_append(&pathBloques, "Bloques");
	log_info(logger, "Path Bloques %s", pathBloques);

	pathArchivos = string_new();
	string_append(&pathArchivos, config->PUNTO_MONTAJE);
	string_append(&pathArchivos, "Archivos");
	log_info(logger, "Path Archivos %s",pathArchivos);

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

		printf("PUERTO: %d\n", config->PUERTO_KERNEL);


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
				liberar_paquete(paquete);
				pthread_mutex_unlock(&solicitud_mutex);
				break;

			case CREAR_ARCHIVO:
				crearArchivo(paquete);
				liberar_paquete(paquete);
				pthread_mutex_unlock(&solicitud_mutex);
				break;

			case BORRAR_ARCHIVO:
				borrarArchivo(paquete);
				liberar_paquete(paquete);
				pthread_mutex_unlock(&solicitud_mutex);
				break;

			case SOLICITUD_OBTENCION_DATOS:
				obtenerDatos(paquete);
				liberar_paquete(paquete);
				pthread_mutex_unlock(&solicitud_mutex);
				break;

			case GUARDAR_DATOS:
				guardarDatos(paquete);
				liberar_paquete(paquete);
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

	t_pedidoGuardadoDatos* pedido = malloc(sizeof(t_pedidoGuardadoDatos));
	pedido = (t_pedidoGuardadoDatos*)paquete->data;


	t_paquete* paq = malloc(sizeof(t_paquete));
	paq = recibir(socketKernel);






	char* path = generarPathArchivo((char*)paq->data);

	log_info(logger, "voy a leer path:%s, offset:%d, size:%d", path, pedido->offset, pedido->size);

	if(!existeArchivo(path)){
		int a=1;
		log_error(logger, "NO EXISTE EL ARCHIVO");
		enviar(socketKernel, SOLICITUD_OBTENCION_DATOS_FALLO, sizeof(int), &a);
		return;
	}

	printf("1\n");

	char* buffer = malloc(sizeof(pedido->size));

	t_config* c = config_create(path);
	char** bloques = config_get_array_value(c, "BLOQUES");

	printf("2\n");




	int offsetBloque, bytesLeidos = 0, restoBloque;

	offsetBloque = (pedido->offset % config->TAMANIO_BLOQUES);
	int numBloque = (pedido->offset / config->TAMANIO_BLOQUES);
	int j = numBloque;

	printf("\n\n A VER :%s\n\n", bloques[numBloque]);

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
	log_debug(logger, "GUARDAR DATOS");

	t_pedidoGuardadoDatos* pedido = malloc(sizeof(t_pedidoGuardadoDatos));
	pedido = (t_pedidoGuardadoDatos*)paquete->data;

	t_paquete* paq = malloc(sizeof(t_paquete));
	paq = recibir(socketKernel);

	char* path = generarPathArchivo((char*)paq->data);

	t_paquete* paq2 = malloc(sizeof(t_paquete));
	paq2 = recibir(socketKernel);

	char* escritura = (char*)paq2->data;

	if(!existeArchivo(path)){
		int a=1;
		log_error(logger, "ARCHIVO INEXISTENTE");
		enviar(socketKernel, SOLICITUD_GUARDADO_DATOS_FALLO, sizeof(int), &a);
		return;
	}

	t_config* c = config_create(path);

	char** bloques = config_get_array_value(c, "BLOQUES");
	int cantBloques = cantidadBloques(bloques);

	config_destroy(c);

	int offsetBloque;
	int restoBloque, bloque;

	offsetBloque = (pedido->offset % config->TAMANIO_BLOQUES);

	int numBloque = (pedido->offset / config->TAMANIO_BLOQUES);

	int j = numBloque, bytesEscritos = 0;

	bloque = atoi(bloques[numBloque]);

	while(bytesEscritos < pedido->size){

		restoBloque = pedido->size - bytesEscritos;

		if(restoBloque > config->TAMANIO_BLOQUES)
			restoBloque = config->TAMANIO_BLOQUES;

		log_info(logger, "Accedo al bloque %d", bloque);

		escribirEnArchivo(bloque, escritura + bytesEscritos, restoBloque, offsetBloque);

		bytesEscritos += restoBloque;

		offsetBloque = 0;

		if(bytesEscritos < pedido->size){
			if(numBloque +1 == cantBloques){
				log_info(logger, "Reservo un nuevo Bloque");
				bloque = reservarNuevoBloque(path);
				if(bloque == -1){

					log_error(logger, "NO HAY MAS ESPACIO");
					enviar(socketKernel, SOLICITUD_GUARDADO_DATOS_FALLO, sizeof(int), &bloque);
					return;
				}
			}
			else{
				j++;
				bloque = atoi(bloques[numBloque + j]);
			}
		}
	}

	aumentarTamanioArchivo(pedido->offset, pedido->size, path);

	//free(pedido);
	free(path);

	enviar(socketKernel, SOLICITUD_GUARDADO_DATOS_OK, sizeof(int), &j);



	return;
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


int cantidadBloques(char** bloques){
	int j = 0;

	while(bloques[j] != NULL)
		j++;

	return j;
}


void aumentarTamanioArchivo(int offset, int size, char* path){

	t_config* c = config_create(path);

	int tamanio = config_get_int_value(c, "TAMANIO");
	char* bloques = config_get_string_value(c, "BLOQUES");

	int bytesEscritos = offset + size - tamanio;

	tamanio += bytesEscritos;

	if(bytesEscritos >0 ){

		config_set_value(c, "TAMANIO", string_itoa(tamanio));
		config_set_value(c, "BLOQUES", bloques);
		config_save(c);
	}

	config_destroy(c);

}

void escribirEnArchivo(int bloque, char* buffer, int size, int offset){
	FILE* archivo = fopen(generarPathBloque(bloque), "r+");

	fseek(archivo, offset, SEEK_SET);

	fwrite(buffer, size, 1, archivo);

	fclose(archivo);

}

int reservarNuevoBloque(char* pathArchivo){
	int bloqueLibre = buscarBloqueLibre();

	if(bloqueLibre == -1)
		return -1;

	escribirValorBitarray(1, bloqueLibre);

	t_config* c = config_create(pathArchivo);
	char* bloques = string_new();

	string_append(&bloques, config_get_string_value(c, "BLOQUES"));

	bloques[strlen(bloques)-1] = '\0';

	string_append(&bloques, ",");
	string_append(&bloques, string_itoa(bloqueLibre));
	string_append(&bloques, "]");
	config_set_value(c,"BLOQUES", bloques);

	config_save(c);
	config_destroy(c);
	free(bloques);

	return bloqueLibre;
}


void config_set_value(t_config* self, char* key, char*value){
	t_dictionary* dictionary = self->properties;
	char* duplicate_value = string_duplicate(value);

	if(dictionary_has_key(dictionary, key)){
		dictionary_remove_and_destroy(dictionary, key, free);
	}

	dictionary_put(self->properties, key, (void*)duplicate_value);
}

int config_save(t_config* self){
	return config_save_in_file(self, self->path);
}


int config_save_in_file(t_config* self, char* path){
	FILE* file = fopen(path, "wb+");

	if(file == NULL){
		return -1;
	}

	char* lines = string_new();
	void add_line(char* key, void* value){
		string_append_with_format(&lines, "%s=%s\n", key, value);
	}

	dictionary_iterator(self->properties, add_line);
	int result = fwrite(lines, strlen(lines),1,file);
	fclose(file);
	free(lines);
	return result;
}
