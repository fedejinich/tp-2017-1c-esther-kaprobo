#include "Estructuras.h"

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
