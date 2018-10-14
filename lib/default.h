#ifndef DEFAULT_H
#define DEFAULT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <termios.h>
#include <time.h>
#include <assert.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 512
#define PORT 9002
#define NAME_SIZE 40

#define Media_comp(new_monster, media) (media > 500) ? ((new_monster).simbolo = 'M') : ((new_monster).simbolo = 'm')

struct mov_msg{
	char msg;
};

struct upd_msg{//Mensagem de atualização do jogo
	int tipo, vida;
    int x, y; 
    int a, b; 
	char new;
    char old;
};

/*
Tipo 0:
    Apresenta modificações no mapa.
    x, y coordenadas novas
    a, b coordenadas antigas
    char new, icone novo
    char old, icone antigo
Tipo 1:
    Apresenta modificações do stats do inimigo em pvp
    x ataque
    y defesa
    vida hp
Tipo 2:
    Apresenta modificações do stats do inimigo em pve
Tipo 3:
    Apresenta modificações do stats do usuario.
    x ataque
    y defesa
    vida hp
    a score
Tipo 4:
    Saiu da batalha Movimentacao e mapa volta ao normal
Tipo 5:
    Perdeu o jogo
Tipo 6:
    Vai lutar contra o boss
Tipo 7:
    Venceu
*/

struct clientInfo{
    int mapa;
    char nome[NAME_SIZE];
    int CON, STR, DEX;
    int classe;
};

struct monsters{
	int x, y;
	// int informações do game, hp, ataque, defesa, pontos...;
    	int vida, ataque, defesa, turn;
    	char simbolo;
	int fight, whofight;
};

typedef struct monsters monsters;
typedef struct clientInfo clientInfo;
typedef struct mov_msg mov_msg;
typedef struct upd_msg upd_msg;
#endif