/*
 * tablaDePaginas.c
 *
 *  Created on: 19/5/2017
 *      Author: utnso
 */


#include "tablaDePaginas.h"

void inicializarTablaDePaginas() {
	log_info(logger,"Inicializando tabla de paginas...");

	tablaDePaginas = &memoria[0];
	ultimasPaginasDePIDs = list_create();

	int nroDeFrameTablaDePaginas;
	for(nroDeFrameTablaDePaginas = 0; nroDeFrameTablaDePaginas <= frames; nroDeFrameTablaDePaginas++) {
		t_entradaTablaDePaginas* entradaTablaDePaginas = malloc(sizeof(t_entradaTablaDePaginas));

		int frame = nroDeFrameTablaDePaginas;
		int pid = -1;
		int pagina = -1;

		entradaTablaDePaginas->frame = frame;
		entradaTablaDePaginas->pid = pid;
		entradaTablaDePaginas->pagina = pagina;

		memcpy(&tablaDePaginas[entradaTablaDePaginas->frame], entradaTablaDePaginas, sizeof(t_entradaTablaDePaginas));

		free(entradaTablaDePaginas);
	}

	log_info(logger, "Reservando espacio para tabla de paginas");
	int sizeTablaDePaginas = ceil((double)(sizeof(t_entradaTablaDePaginas) * frames) / (double)frame_size);
	int i;
	for(i = 0; i <= sizeTablaDePaginas; i++)
		escribirTablaDePaginas(i, -33, -1);

	log_info(logger,"Tabla de paginas inicializada.");
}

int escribirTablaDePaginas(int frame, int pid, int pagina) {
	bloquearTablaDePaginas();

	t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(frame);
	if(entrada == EXIT_FAILURE_CUSTOM) {
		log_error(logger, "No se puede escribir en tabla de paginas. Frame: %i, PID: %i, Pagina: %i", frame, pid, pagina);
		desbloquearTablaDePaginas();

		return EXIT_FAILURE_CUSTOM;
	} else {
		entrada->pid = pid;
		entrada->pagina = pagina;
		log_info(logger, "Se escrbio en tabla de paginas. Frame: %i, PID: %i, Pagina: %i", frame, pid, pagina);
		desbloquearTablaDePaginas();

		return EXIT_SUCCESS_CUSTOM;
	}
}

t_entradaTablaDePaginas* getEntradaTablaDePaginas(int index) {
	if(index > frames || index < 0) {
		log_error(logger,"Error getEntradaTablaDePaginas(%i)", index);
		return EXIT_FAILURE_CUSTOM;
	}

	return &tablaDePaginas[index];
}

t_entradaTablaDePaginas* getEntradaTablaDePaginasHash(int pid, int pagina) {
	int posiblePosicion = calcularPosicion(pid, pagina);
	t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(posiblePosicion);
	if(entrada->pid == pid && entrada->pagina == pagina) {
		log_info(logger, "Se encontro la entrada a la tabla de paginas para el PID: %i , Pagina: %i", pid, pagina);
		return entrada;
	}
	else {
		//log_warning(logger, "Colision en funcion de hash, me voy para arriba");
		int i;
		for(i = posiblePosicion; i <= tablaDePaginasSize(); i++) {
			t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(i);
			if(entrada->pid == pid && entrada->pagina == pagina) {
				log_info(logger, "Se encontro la entrada a la tabla de paginas para el PID: %i , Pagina: %i", pid, pagina);
				return entrada;
			}
		}
	}
	int j;
	for(j = posiblePosicion; j >= 0; j--) {
		//log_warning(logger, "Colision en funcion de hash, me voy para abajo");
		t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(j);
		if(entrada->pid == pid && entrada->pagina == pagina) {
			log_info(logger, "Se encontro la entrada a la tabla de paginas para el PID: %i , Pagina: %i", pid, pagina);
			return entrada;
		}
	}

	log_error(logger, "No se encontro la entrada a la tabla de paginas para el PID: %i, Pagina: %i", pid, pagina);
	return EXIT_FAILURE_CUSTOM;
}

void escribirTablaDePaginasHash(int pid, int pagina) {
	int posiblePosicion = calcularPosicion(pid, pagina);
	t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(posiblePosicion);
	if(entrada->pid != -1) {
		entrada->pid = pid;
		entrada->pagina = pagina;
	}
	else {
		int i;
		for(i = posiblePosicion; i <= tablaDePaginasSize(); i++) {
			t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(i);
			if(entrada->pid != -1) {
				entrada->pid = pid;
				entrada->pagina = pagina;
			}
		}
	}
	int j;
	for(j = posiblePosicion; j >= 0; j--) {
		t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(j);
		if(entrada->pid != -1) {
			entrada->pid = pid;
			entrada->pagina = pagina;
		}
	}

};

int getFrameDisponibleHash(int pid, int pagina) {
	int posibleFrame = calcularPosicion(pid, pagina);
	t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(posibleFrame);
	if(entrada->pid == -1)
		return posibleFrame;
	else {
		log_warning(logger, "Colision en funcion de hash, me voy para arriba");
		int i;
		for(i = posibleFrame + 1; i <= tablaDePaginasSize(); i++) {
			entrada = getEntradaTablaDePaginas(i);
			if(entrada->pid == -1) {
				return entrada->frame;
			}
		}
	}
	int j;
	log_warning(logger, "No hay espacios para arriba, me voy para abajo");
	for(j = posibleFrame - 1; j >= 0; j--) {
		entrada = getEntradaTablaDePaginas(j);
		if(entrada->pid == -1) {
			return entrada->frame;
		}
	}

	log_error(logger, "MEMORIA LLENA");
	log_error(logger, "No hay frames disponibles");
	return EXIT_FAILURE_CUSTOM;
}

int paginasDisponibles(int pid, int paginasRequeridas) {
	int cantidadFramesDisponibles = getCantidadFramesDisponibles();

	if(cantidadFramesDisponibles == EXIT_FAILURE_CUSTOM) {
		log_error(logger, "Error paginasDisponibles(%i, %i)", pid, paginasRequeridas);
		return EXIT_FAILURE_CUSTOM;
	}

	if(cantidadFramesDisponibles >= paginasRequeridas)
		return EXIT_SUCCESS_CUSTOM;

	log_error(logger, "No hay mas espacio disponible en memoria para PID %i, Paginas requeridas %i", pid, paginasRequeridas);

	return EXIT_FAILURE_CUSTOM;
}

int getCantidadFramesDisponibles() {
	int i;
	int framesDisponibles = 0;
	for(i = 0; i < frames; i++) { //VER SI VA UN IGUAL ACA i <= frames
		t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(i);

		if(entrada == EXIT_FAILURE_CUSTOM) {
			log_error(logger, "Error en getCantidadFramesDisponibles()");
			return EXIT_FAILURE_CUSTOM;
		}

		if(entrada->pid == -1)
			framesDisponibles++;
	}

	return framesDisponibles;
}

int getCantidadFramesOcupados() {
	int cantidad = getCantidadFramesDisponibles();

	if(cantidad == EXIT_FAILURE_CUSTOM) {
		log_error(logger, "Error en getCantidadFramesOcupados()");
		return EXIT_FAILURE_CUSTOM;
	}
	return frames - cantidad;
}

int tablaDePaginasSize() {
	return frames;
}

int reservarPaginas(int pid, int paginasAReservar) { // LA PRIMERA PAGINA SIEMPRE ES LA 1
	int i;
	for(i = 1; i <= paginasAReservar; i++) {
		int pagina = i;
		int frameDisponible = getFrameDisponibleHash(pid, pagina);

		if(frameDisponible == EXIT_FAILURE_CUSTOM) {
			log_error(logger, "No se pueden reservar %i paginas para PID %i", paginasAReservar, pid);
			return EXIT_FAILURE_CUSTOM;
		}

		int escribio = escribirTablaDePaginas(frameDisponible, pid, pagina);
		if(escribio == EXIT_FAILURE_CUSTOM) {
			log_error(logger, "No se pudo reservar la pagina %i para el PID %i", pagina, pid);
			return EXIT_FAILURE_CUSTOM;
		}
	}

	return EXIT_SUCCESS_CUSTOM;
}

int asignarMasPaginasAProceso(int pid, int paginasAsignar) {
	int ultimaPagina = getUltimaPagina(pid);

	if(ultimaPagina == EXIT_FAILURE_CUSTOM) {
		log_error(logger, "No se pueden asignar %i paginas al PID %i",paginasAsignar, pid);
		return EXIT_FAILURE_CUSTOM;
	}

	int i;
	for(i = 0; i < paginasAsignar; i++) {
		ultimaPagina++;
		int pagina = ultimaPagina;
		int frameDisponible = getFrameDisponibleHash(pid, pagina);

		if(frameDisponible == EXIT_FAILURE_CUSTOM) {
			log_error(logger, "No hay mas espacio para asignar la pagina %i del PID %i", pid, pagina);
			log_error(logger, "No se puedieron asignar todas las pagnas requeridas");
			return EXIT_FAILURE_CUSTOM;
		}

		int exito = escribirTablaDePaginas(frameDisponible, pid, pagina);

		if(exito == EXIT_FAILURE_CUSTOM) {
			log_error(logger, "No se  pudo asignar una pagina mas para el PID %i. PID: %i, ultima pagina que quizo ser asignada: %i", pid, pid, pagina);
			return EXIT_FAILURE_CUSTOM;
		}

		log_info(logger, "Se asigno una pagina mas para el PID %i. PID: %i, ultima pagina asignada: %i", pid, pid, pagina);
	}

	int exito = remplazarUltimaPagina(pid, ultimaPagina);

	if(exito == EXIT_FAILURE_CUSTOM) {
		log_error(logger, "No se pudo completar el proceso de ASIGNAR MAS PAGINAS A PROCESO");
		return EXIT_FAILURE_CUSTOM;
	}

	log_info(logger, "Ultima pagina asignada de PID %i: %i", pid, ultimaPagina);
	log_debug(logger, "Se asignaron %i paginas mas para el PID %i", paginasAsignar, pid);
	return EXIT_SUCCESS_CUSTOM;
}

int remplazarUltimaPagina(int pid, int ultimaPagina) {
	int i;
	for(i = 0; i < list_size(ultimasPaginasDePIDs); i++) {
		t_ultimaPaginaPID* entrada = list_get(ultimasPaginasDePIDs, i);
		if(entrada->pid == pid) {
			log_info(logger, "Remplazando en lista de ultimas paginas. Antes %i, ahora %i", entrada->ultimaPagina, ultimaPagina);
			entrada->ultimaPagina = ultimaPagina;
			return EXIT_SUCCESS_CUSTOM;
		}
	}

	return EXIT_FAILURE_CUSTOM;
}

int getUltimaPagina(int pid) {
	int ultimaPagina = EXIT_FAILURE_CUSTOM;

	int i;
	for(i = 0; i < list_size(ultimasPaginasDePIDs); i++) {
		log_warning(logger, " %i < %i ", i, list_size(ultimasPaginasDePIDs));

		t_ultimaPaginaPID* entrada = list_get(ultimasPaginasDePIDs, i);
		if(pid == entrada->pid) {
			return entrada->ultimaPagina;
		}
	}

	return ultimaPagina;
}

t_list* getEntradasDePID(int pid) {
	int i;
	t_list* lista = list_create();
	for(i = 0; i < frames; i++) {
		t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(i);
		if(entrada->pid == pid)
			list_add(lista, entrada);
	}

	if(lista->elements_count <= 0) {
		log_error(logger, "No se encontraron entradas del PID %i", pid);
		return EXIT_FAILURE_CUSTOM;
	}


	return lista;
}

int esPaginaLiberable(int pid, int pagina) {
	//ver si por algun motivo no se podria liberar la pagina

	return EXIT_SUCCESS_CUSTOM;
}

int liberarPagina(int pid, int pagina) {
	int frame = getFrameByPIDPagina(pid, pagina);

	if(frame == EXIT_FAILURE_CUSTOM) {
		log_error(logger, "No se puede liberar pagina nro %i del PID %i", pagina, pid);
		return EXIT_FAILURE_CUSTOM;
	}

	escribirTablaDePaginas(frame, -1, 0);

	log_debug(logger, "Se libero la pagina nro %i del PID %i", pagina, pid);
	return EXIT_SUCCESS_CUSTOM;
}

bool existePIDPagina(int pid, int pagina) {
	int i;
	for(i = 0; i <= tablaDePaginasSize(); i++) { //VER SI ES <= O < Y SI INICIALIZA EN 0
		t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(i);
		if(entrada->pid == pid && entrada->pagina == pagina)
			return true;
	}

	log_warning(logger, "No existe PID %i Pagina %i en tabla de paginas", pid, pagina);
	return false;
}

int getFrameByPIDPagina(int pid, int pagina) {
	int i;
	for(i = 0; i <= tablaDePaginasSize(); i++) { //VER SI ES <= O < Y SI INICIALIZA EN 0
		t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(i);
		if(entrada->pid == pid && entrada->pagina == pagina)
			return entrada->frame;
	}

	log_error(logger, "No existe PID %i Pagina %i en tabla de paginas", pid, pagina);
	return EXIT_FAILURE_CUSTOM;
}

int getFramePrimeraPagina(int pid) {
	int primeraPagina = -1;
	int i;
	bool encontre = false;
	t_list* entradasDePID = getEntradasDePID(pid);
	for(i = 0; i < list_size(entradasDePID) && !encontre; i++) {
		t_entradaTablaDePaginas* entrada = list_get(entradasDePID,i);
		if(entrada->pagina == 0) {
			encontre = true;
			primeraPagina = entrada->frame;
		}
	}

	if(encontre == false && primeraPagina < 0) {
		log_error(logger, "Error en getPrimeraPagina(%i)", pid);
		return EXIT_FAILURE_CUSTOM;
	}

	return primeraPagina;
}

void* getPaginaByPID(int pid, int pagina) {
	void* buffer = malloc(frame_size);

	t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginasHash(pid, pagina);
	int frame = entrada->frame;

	leerFrame(frame, 0, frame_size, buffer);

	return buffer;
}

void bloquearTablaDePaginas() {
	pthread_mutex_lock(&tablaDePaginasMutex);
	log_info(logger, "Bloqueo tabla de paginas");
}

void desbloquearTablaDePaginas() {
	pthread_mutex_unlock(&tablaDePaginasMutex);
	log_info(logger, "Desbloqueo tabla de paginas");
}
