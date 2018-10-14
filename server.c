    #include "lib/server.h"

void MyClientConnected(int id, clientInfo startInfo); // função que é chamada quando um cliente é connectado
void MyClientMoved(int id, mov_msg mov);// função que é chamada quando cliente manda mensagem de movimento
void startGame(void);// função que é chamada quando cliente 0 confirma o inicio do jogo
void MyBroadcast(void);// um exemplo de como mandar uma mensagem para todos os usuários
void readMap(void);
void readMonster(void);
void printMap(void);
FILE* fileOpen(char filedir[], char mode[]);
void stats_calc(int* ATT_BASE, int ATT);
void limpaClientPos(int x, int y);
int checkBlock(int x, int y);
void comeArvore(int i);
void movePlayer(int i, int x, int y);
void randPlacer(int x, int y);
void moveMonster(void);
void initFighters(void);
void sendSetFight(int id, int boolean);
void setFight(int id, int x, int y, char pos);
void runaway(int id);
void killPlayer(void);
void killMonster(void);
void checkPlayersAlive(void);
void fight(int id, char cmd);
void sendStatusToClient(int id);
void sendStatusToAll(void);
void sendEnemyStatusToClient(int id, int who_id);

int map_choosed;
char** mapa;
int Row;
int Column;
int enemies_total = 0;
int grand_enemies_total;
#define MONSTRO_MAX 100
player enemies[MONSTRO_MAX];
int clients_playing = 0;

int main (void) {
    srand(time(NULL)); // seed pra usar o rand durante o jogo
    clientMoved = MyClientMoved;
    clientConnected = MyClientConnected;
    clientConfirmed = startGame;

    system("clear");

    init();

    while (1) {
        // espera alguma mensagem
        sleepServer();

        // verifando se foi uma conexão..
        checkConn();

        // se foi alguém se comunicando...
        wasClient();

        //broadcasting das modificações do mapa
        if(game_status == 2) {
            usleep(200);
            broadcast();
        }
	}
}

void MyClientConnected(int id, clientInfo startInfo) {
    int HP, ATK, DEF;
    int i, x, y;
    printf("Client connected, id = %d, map = %d\n", id, startInfo.mapa);
    printf("Client choosed %d class, CON = %d, DEX = %d, STR = %d.\n", startInfo.classe, startInfo.CON, startInfo.DEX, startInfo.STR);
    printf("Client name = %s.\n",  startInfo.nome);
    int boolean = 1;

    //Thief
    if(startInfo.classe == 0) {
        HP = 10;
        ATK = 20;
        DEF = 15;
    }
    //Paladin
    else if(startInfo.classe == 1) {
        HP = 15; 
        ATK = 10;
        DEF = 20;
    }
    //Barbarian
    else if(startInfo.classe == 2) {
        HP = 20;
        ATK = 15;
        DEF = 10;
    }

    system("clear");
    if(id == 0) {
        map_choosed = startInfo.mapa;
        readMap();
        readMonster();
        grand_enemies_total = enemies_total;
        for(i=0;i<enemies_total;i++){
        	do{ 
                enemies[i].fight = 0;
                enemies[i].turn = 0;
        		enemies[i].x = (rand() % (Row-2) ) + 1;
            	enemies[i].y = (rand() % (Column-2) ) + 1;
        	}while(mapa[enemies[i].x][enemies[i].y] != ' ');

        	mapa[enemies[i].x][enemies[i].y] = 'm';

        	printf("Monster ID: [%d]\n",i);
            printf("Starting Position: (%d,%d)\n",enemies[i].x,enemies[i].y);
            printf("<HP>: %d\n",enemies[i].vida);
            printf("<ATK>: %d\n",enemies[i].ataque);
            printf("<DEF>: %d\n",enemies[i].defesa);
        }

        sendTxtToClient(clients[id].sockid, "Voce e o jogador 0. Digite qualquer tecla para iniciar o jogo.\n");
    }
    else{
        sendTxtToClient(clients[id].sockid, "Aguarde o criador do jogo iniciar a partida!\n");
    }

    stats_calc(&HP, startInfo.CON);
    stats_calc(&DEF, startInfo.DEX);
    stats_calc(&ATK, startInfo.STR);
    
    do{
    	clients[id].x = (rand() % (Row-2) ) + 2; //posicao inicial do jogador em x
    	clients[id].y = (rand() % (Column-2) ) + 2; //posicao inicial do jogador em y
    }while(mapa[clients[id].x][clients[id].y] != ' ');

    printf("Client %d coord = (%d; %d).\n", id, clients[id].x, clients[id].y);
    mapa[ clients[id].x ][ clients[id].y ] = '0' + id;

    clients[id].vida = HP;
    clients[id].ataque = ATK;
    clients[id].defesa = DEF;
    clients[id].turn = 0; //tá certo?
    clients[id].fight = 0;
    clients[id].whofight = -1; //tá certo?
    strcpy(clients[id].nome, startInfo.nome);

    printMap();
    game_status = 1;

    clients_playing++;

    return;
}

void MyClientMoved(int id, mov_msg mov){
    usleep(100); // verificado experimentalmente que melhora a dinâmica do jogo
    printf("Client %d moved: %c\n", id, mov.msg);
    int x, y;

    x = clients[id].x;
    y = clients[id].y;

    if(clients[id].fight == 0) {
        if(mov.msg == 'd') {
            y++;

            if( y < (Column - 2) && (mapa[x][y] == ' ' || mapa[x][y] == '*') ) {
                limpaClientPos(clients[id].x, clients[id].y);

                if(checkBlock(x, (y + 1)))
                    comeArvore(id);
            
                movePlayer(id, 0, 1);
                moveMonster();
            }
            else if( y < (Column - 1) ) {
                if(mapa[x][y] == 'm') {
                   printf("Player vs Monster d\n"); 
                   setFight(id, x, y, mapa[x][y]);
                }
                else if(mapa[x][y] >= '0' && mapa[x][y] <= '9') {
                   setFight(id, x, y, mapa[x][y]);
                }
            }
        }
        else if(mov.msg == 'a') {
            y--;

            if( y > 0 && (mapa[x][y] == ' ' || mapa[x][y] == '*') ) {
                limpaClientPos(clients[id].x, clients[id].y);

                if(checkBlock(x, y - 1))
                    comeArvore(id);

                movePlayer(id, 0, -1);
                moveMonster();        
            }
            else if( y > 2 ) {
                if(mapa[x][y] == 'm') {
                   setFight(id, x, y, mapa[x][y]);
                   printf("Player vs Monster a\n");
                }
                else if(mapa[x][y] >= '0' && mapa[x][y] <= '9') {
                   setFight(id, x, y, mapa[x][y]);
                }
            }
        }
        else if(mov.msg == 'w') {
            x--;

            if( x > 0 && (mapa[x][y] == ' ' || mapa[x][y] == '*') ) {
                limpaClientPos(clients[id].x, clients[id].y);

                if(checkBlock((x - 1), y))
                    comeArvore(id);
            
                movePlayer(id, -1, 0);
            	moveMonster();
            }
            else if( x > 1 ) {
                if(mapa[x][y] == 'm') {
                   setFight(id, x, y, mapa[x][y]);
                   printf("Player vs Monster w\n");
                }
                else if(mapa[x][y] >= '0' && mapa[x][y] <= '9') {
                   setFight(id, x, y, mapa[x][y]);
                }
            }
        }
        else if(mov.msg == 's') {
            x++;

            if( x < (Row - 1) && (mapa[x][y] == ' ' || mapa[x][y] == '*') ) {
                limpaClientPos(clients[id].x, clients[id].y);

                if(checkBlock((x - 1), y))
                    comeArvore(id);

                movePlayer(id, 1, 0);
                moveMonster();
            }
            else if( x < (Row - 1) ) {
                if(mapa[x][y] == 'm') {
                   setFight(id, x, y, mapa[x][y]);
                   printf("Player vs Monster s\n");
                }
                else if(mapa[x][y] >= '0' && mapa[x][y] <= '9') {
                   setFight(id, x, y, mapa[x][y]);
                }
            }
        }
    }
    else if(mov.msg == 'r') {
       runaway(id);
    }
    else if(clients[id].fight == 1 || clients[id].fight == 2) {
       fight(id, mov.msg);
    }

    printMap();
    checkPlayersAlive();

    return;
}

void startGame() {
    char string[] = "++Preparem suas armas, o jogo vai começar.\0";
    string[1] = '0' + map_choosed;
    int i;

    game_status = 2;

    printf("Client 0 confirmed, the game will start now...\n");
    broadcastTxt(string, -1);
    // avisar para os clientes que o jogo vai começar
    // enviar algumas informações, como mapa, status inicial do cliente, etc..

    for(i = 0; i < clients_connected; i++) {
            printf("Player ID: [%d]\n", i);
            printf("Starting Position: (%d,%d)\n", clients[i].x, clients[i].y);
            printf("<HP>: %d\n", clients[i].vida);
            printf("<ATK>: %d\n", clients[i].ataque);
            printf("<DEF>: %d\n", clients[i].defesa);   
    }

    for(i = 0; i < clients_connected; i++) {
        map_changes[pos_broad].tipo = 0;
        map_changes[pos_broad].x = clients[i].x;
        map_changes[pos_broad].y = clients[i].y;
        map_changes[pos_broad].new = '0' + i;
        pos_broad++;
    }
    for(i = 0; i < enemies_total; i++) {
        map_changes[pos_broad].tipo = 0;
        map_changes[pos_broad].x = enemies[i].x;
        map_changes[pos_broad].y = enemies[i].y;
        map_changes[pos_broad].new = 'm';
        pos_broad++;
    }

    sendStatusToAll();

    return;
}

void MyBroadcast(){
    int id, sd;
    for(id = 0; id < MAX_CLIENTS; ++id){
        sd = clients[id].sockid;
        if(sd > 0){
            sendTxtToClient(sd, "teste");
            //enviar mensagem, ex: sendUpdToClient(sd, updt);
        }
    }
}

void readMap(void) {
    FILE* file_map;
    int i;
    int j;
    int cond;

    switch(map_choosed) {
        case 1:
            file_map = fileOpen("./data/mapa1.txt", "rt");
            break;
        case 2:
            file_map = fileOpen("./data/mapa2.txt", "rt");
            break;
        case 3:
            file_map = fileOpen("./data/mapa3.txt", "rt");
            break;
        case 4:
            file_map = fileOpen("./data/mapa4.txt", "rt");
            break;
        case 5:
            file_map = fileOpen("./data/mapa5.txt", "rt");
            break;
        case 6:
            file_map = fileOpen("./data/mapa6.txt", "rt");
            break;
        case 7:
            file_map = fileOpen("./data/mapa7.txt", "rt");
            break;
        case 8:
            file_map = fileOpen("./data/mapa8.txt", "rt");
            break;
        case 9:
            file_map = fileOpen("./data/mapa9.txt", "rt");
            break;
        default:
            printf("EASTER EGG.\n");
            broadcastTxt("MARVEL > DC", -1);
            exit(1);
            break;
    }

    fscanf(file_map, "%d %d %d\n", &Row, &Column, &enemies_total);
    Column++;

    mapa = (char**) malloc((Row + 5) * sizeof(char*));
    if(mapa == NULL) {
        printf("mapa Lruim\n");
        exit(1);  
    } 
    
    for(i = 0; i < Row; i++) {
        mapa[i] = (char*) malloc((Column + 5) * sizeof(char));
        if(mapa[i] == NULL) {
            printf("mapa Cruim\n");
            exit(1);
        }
    }

    for(i = 0; i < Row; i++)
        for(j = 0; j < Column; j++)
            mapa[i][j] = fgetc(file_map);

    fclose(file_map);

    return;
}

int hpRand(int n, int base) {
    int hp;
    hp = (rand() % n) +  base;
    return hp;
}

void stats_calc(int* ATT_BASE, int ATT) {
	int dice = (rand() % 26);

	*ATT_BASE = (*ATT_BASE + ATT) * 10;
	*ATT_BASE += dice;;

	return;
}

FILE* fileOpen(char filedir[], char mode[]) {
    FILE* pf;
    pf = fopen(filedir, mode);
    if(pf == NULL) {
        printf("Error in opening of %s - %s\n", filedir, mode);
        exit(1);
    }

    return pf;
}

player criaMonstro(void)
{
    int x;
    int y;
    int i;
    int k;
    
    player new_monster;
    
    new_monster.x = 2;
    new_monster.y = 2;
    new_monster.turn = 0;
    new_monster.fight = 0;
    new_monster.whofight = -1;

    x = rand()%101;
    y = (5*(x + 100))/4;

    new_monster.vida = y;

    x = rand()%101;

    y = (5*(x + 100))/4;

    new_monster.ataque = y;

    x = rand()%101;
    y = (5*(x + 100))/4;

    new_monster.defesa = y;

    return new_monster;
}

void printMap(void) {
    int i, j;
    for(i = 0; i < Row; i++)
        for(j = 0; j < (Column + 1); j++)
            printf("%c", mapa[i][j]);
    printf("\n");
    return;
}

void limpaClientPos(int x, int y) {
    map_changes[pos_broad].tipo = 0;
    map_changes[pos_broad].x = x;
    map_changes[pos_broad].y = y;
    map_changes[pos_broad].new = ' ';
    pos_broad++;

    mapa[x][y] = ' ';
    return;
}

int checkBlock(int x, int y) {
    return (mapa[x][y] == '*') ? 1 : 0;
}

void comeArvore(int i) {
    upd_msg update;

    if( (rand() % 100 + 1) >= 20 && clients[i].vida <= 400) {
        clients[i].vida += (rand() % 5);
    }
    else { //O Cavaleiro se cansou de tanto cortar arvore
        clients[i].vida -= (rand() % 5);
        if(clients[i].vida <= 0) 
            clients[i].vida = 1;
    }

    update.tipo = 3;
    update.vida = clients[i].vida;
    update.x = clients[i].ataque;
    update.y = clients[i].defesa;

    sendUpdToClient(clients[i].sockid, update);

    return;
}

void movePlayer(int i, int x, int y) {
    clients[i].x += x;
    clients[i].y += y;

    map_changes[pos_broad].tipo = 0;
    map_changes[pos_broad].new = '0' + i;
    map_changes[pos_broad].x = clients[i].x;
    map_changes[pos_broad].y = clients[i].y;
    pos_broad++;

    mapa[ clients[i].x ][ clients[i].y ] = '0' + i;

    return;
}

void randPlacer(int x, int y) {
    int boolean = 1;
    printf("Spawning - ");

    while(boolean) {
        x = ( rand() % (Row - 2) ) + 3;
        y = ( rand() % (Column - 2) ) + 3;
        if(mapa[x][y] == ' ') {
            boolean = 0;
        }
    }
    printf("(%d; %d)\n", x, y);

    return;
}

void moveMonster(void) {
    int i;
    int x;
    int y;
    int dice;

    for(i = 0; i < grand_enemies_total; i++) {
        dice = rand() % 4;        

        if(dice == 0 ) { //w
            x = enemies[i].x - 1;
            y = enemies[i].y;
        }
        else if(dice == 1) { //s
            x = enemies[i].x + 1;
            y = enemies[i].y;
        }
        else if(dice == 2) {
            x = enemies[i].x; //a
            y = enemies[i].y - 1;
        }
        else if(dice == 3) {
            x = enemies[i].x; //d
            y = enemies[i].y + 1;   
        }
        else{// parado

        }    

        if(mapa[x][y] == ' ' && enemies[i].fight == 0 && enemies[i].turn != -1) { //verifico se ele ficou parado e se há obstaculos
          
          	limpaClientPos(enemies[i].x , enemies[i].y);
                        
            enemies[i].x = x; //atualização das coordenadas
            enemies[i].y = y; //

            printf("Monster %d moved to (%d; %d)\n", i, x, y);
            //atribuo a nova posição dele
          
            mapa[x][y] = 'm';
            map_changes[pos_broad].tipo = 0;
            map_changes[pos_broad].x = x;
            map_changes[pos_broad].y = y;
            map_changes[pos_broad].new = 'm';
            pos_broad++;
        }

    }

    return;
}

void readMonster(void){
	 FILE* file_monster;
    int i;

    switch(map_choosed) {
        case 1:
            file_monster = fileOpen("./data/monstros1.bin", "rb");
            break;
        case 2:
            file_monster = fileOpen("./data/monstros2.bin", "rb");
            break;
        case 3:
            file_monster = fileOpen("./data/monstros3.bin", "rb");
            break;
        case 4:
            file_monster = fileOpen("./data/monstros4.bin", "rb");
            break;
        case 5:
            file_monster = fileOpen("./data/monstros5.bin", "rb");
            break;
        case 6:
            file_monster = fileOpen("./data/monstros6.bin", "rb");
            break;
        case 7:
            file_monster = fileOpen("./data/monstros7.bin", "rb");
            break;
        case 8:
            file_monster = fileOpen("./data/monstros8.bin", "rb");
            break;
        case 9:
            file_monster = fileOpen("./data/monstros9.bin", "rb");
            break;
        default:
            broadcastTxt("MARVEL > DC", -1);
            printf("EASTER EGG\n");
            exit(1);
    }

    fread(enemies, sizeof(player), enemies_total, file_monster);
    fclose(file_monster);
    printf("%d monstros lidos com sucesso.\n", enemies_total);


    return;
}

void setFight(int id, int x, int y, char pos) { //Tem um bug de reconhecimento de who_id aqui.
    int i;
    int who_id;

    printf("Player %d in (%d; %d) - %d\n", id, clients[id].x, clients[id].y, mapa[clients[id].x][clients[id].y]);
    printf("Enemy in (%d; %d) - %c\n", x, y, pos);

    if(pos >= '0' && pos <= '9') { //busco se o player esta lutando contra player
        who_id = (int) pos - '0'; //e o id do player a ser atacado

        if(clients[who_id].fight == 0) {
            clients[id].fight = 1; //set o fight tipo 1 (pvp)
            printf("Player %d engaged a combat with %d player.\n", id, who_id);
        }
    
    }
    else if(pos == 'm') { //busco se o player esta lutando contra monstro

        for(i = 0; i < grand_enemies_total; i++)
            if(enemies[i].x == x && enemies[i].y == y) //busco qual dos monstros estão à lutar
                who_id = i; //através das coordenadas compartilhadas
        
        if(enemies[who_id].fight == 0  && enemies[who_id].turn != -1) {
            clients[id].fight = 2; //seto o fight tipo 2 (pve) comentario
            printf("Player %d engaged a combat with %d monster.\n", id, who_id);
        }
    }

    if(clients[id].fight == 1) {
        printf("PVP: ");

        clients[id].whofight = who_id; //atribuo a quem o cliente esta lutando
        clients[id].turn = 0; //seu turno é zero
        
        clients[who_id].fight = 1; //digo que a "vitima" esta sendo atacada e tipo 1 (pvp)
        clients[who_id].whofight = id; //e que ele esta sento atacado pelo client[  id  ]
        clients[who_id].turn = 1; //seu turno é 1

        sendSetFight(id, 1); //passo como 1 para haver envio ao player adversario
    }
    else if(clients[id].fight == 2) {
        printf("PVE: ");

        clients[id].whofight = who_id;
        clients[id].turn = 0;

        enemies[who_id].fight = 2; //aqui, o tipo é dois por ser pve
        enemies[who_id].whofight = id;
        enemies[who_id].turn = 0;
   
        sendSetFight(id, 0); //passo como 0 para não haver envio ao enemies
    }

    printf("id = %d VS who_id = %d\n", id, who_id);

    return;
}

void sendSetFight(int id, int boolean) {
    upd_msg update;
    int who_id = clients[id].whofight;

    update.tipo = clients[id].fight;
    
    if(update.tipo == 1) {
        update.x = clients[who_id].ataque;
        update.y = clients[who_id].defesa;
        update.vida = clients[who_id].vida;
    }
    else if(update.tipo == 2) {
        update.x = enemies[who_id].ataque;
        update.y = enemies[who_id].defesa;
        update.vida = enemies[who_id].vida;

        boolean = 0; //Se o tipo de combate for pve, não existe player adversario para receber updates
    }


    sendUpdToClient(clients[id].sockid, update); //envio atualizações ao cliente
    
    update.tipo = 3;
    update.x = clients[id].ataque;
    update.y = clients[id].defesa;
    update.vida = clients[id].vida;

    sendUpdToClient(clients[id].sockid, update); //envio atualizações ao cliente

    printf("Update send to %d.\n", id);

    if(boolean == 1) { //se for pvp ele entra
        sendSetFight(who_id, 0); //como passo 0 como boolean, garanto que a recursão terá fim
        printf("Update send to %d.\n", who_id);
    }

    return;
}

void runaway(int id) {
    int who_id = clients[id].whofight;
    int dice;
    int vs_player = 0;
    upd_msg update;

    printf("Client %d runaway from %d.\n", id, who_id);

    if(clients[id].fight == 1) {
        dice = clients[who_id].ataque / 10;
    }
    else if(clients[id].fight == 2) {
        dice = enemies[who_id].ataque / 10;
    }

    clients[id].vida -= dice;

    
    if(clients[id].fight == 1) {
        clients[who_id].fight = 0;
        clients[who_id].whofight = -1;
        clients[who_id].turn = 0;
        vs_player = 1;

        if(rand() % 2)
            clients[who_id].ataque += dice;
        else
            clients[who_id].defesa += dice;

        update.tipo = 4;
        sendUpdToClient(clients[who_id].sockid, update);

    }
    else if(clients[id].fight == 2) {
        enemies[who_id].fight = 0;
        enemies[who_id].whofight = -1;
        enemies[who_id].turn = 0;
    
    }
    clients[id].whofight = -1;
    clients[id].fight = 0;
    clients[id].turn = 0;
    
    update.tipo = 4;
    sendUpdToClient(clients[id].sockid, update);


    update.tipo = 0;
    update.x = 0; //ao sair da luta informo ao client para imprimir o mapa na tela
    update.y = 0; //como o x e y são zero, o caractere é sempre *
    update.new = '*';
    sendUpdToClient(clients[id].sockid, update);

    if(vs_player) {
        update.tipo = 0;
        update.x = 0; //ao sair da luta informo ao client para imprimir o mapa na tela
        update.y = 0; //como o x e y são zero, o caractere é sempre *
        update.new = '*';
        sendUpdToClient(clients[who_id].sockid, update);
    }

        

    return;
}

void killPlayer(void) {
    upd_msg update;
    int who_id;
    int i;

    for(i = 0; i < clients_connected; i++) {
        if(clients[i].vida <= 0 && clients[i].turn != -1) {
            printf("Player %d died.\n", i);

            who_id = clients[i].whofight;
            clients[i].turn = -1;
            update.tipo = 5;

            limpaClientPos(clients[i].x, clients[i].y);

            sendUpdToClient(clients[i].sockid, update);

            update.tipo = 4;
            clients[who_id].fight = 0;
            clients[who_id].whofight = -1;
            clients[who_id].turn = 0;

            sendUpdToClient(clients[who_id].sockid, update);

            clients_playing--;

            disconnectCleint(i);
            clients_connected--;
        }
    }

    return;
}

void killMonster(void) {
    upd_msg update;
    int who_id;
    int i;

    for(i = 0; i < grand_enemies_total; i++) {
        if(enemies[i].vida <= 0 && enemies[i].turn != -1) {
            printf("Monster %d died.\n", i);

            who_id = enemies[i].whofight;
            enemies[i].turn = -1;
            limpaClientPos(enemies[i].x, enemies[i].y);

            printf("Player %d won a combat with %d monster.\n", who_id, i);

            update.tipo = 4;
            clients[who_id].fight = 0;
            clients[who_id].whofight = -1;
            clients[who_id].turn = 1;

            sendUpdToClient(clients[who_id].sockid, update);

            enemies_total--;
        }
    }

    return;
}

void checkPlayersAlive(void) {
    upd_msg update;
    int i, sd;

    killMonster();
    killPlayer();

    printf("There is %d clients playing.\n", clients_playing);
    printf("There is %d monster alive.\n", enemies_total);

    if(clients_playing == 0) {
        update.tipo = 5;

        for(i = 0; i < MAX_CLIENTS; i++) {
            sd = clients[i].sockid;
            if(sd > 0) sendUpdToClient(sd, update);
        }
        game_status = 0;
    }
    else if(clients_playing == 1 && enemies_total == 0) {
        for(i = 0; i < MAX_CLIENTS; i++) {
            sd = clients[i].sockid;
            if(sd > 0) {
                if(clients[i].vida > 0) {
                    printf("Client %d won.\n", i);
                    update.tipo = 7;
                    sendUpdToClient(sd, update);

                    disconnectCleint(i);
                    clients_connected--;
                    game_status = 0;
                }
            }
        }

    }

    return;
}

void fight(int id, char cmd) {
    int who_id = clients[id].whofight; 
    int dice = clients[id].ataque / 2; //dice é a 1/4 do ataque + rand(ataque)
    dice += rand() % clients[id].ataque; 
    int n = 70;

    printf("Player %d is fighting.\n", id);

    if(cmd == 'a' || cmd == 'A') {
        if(clients[id].fight == 1) { //pvp
             if(dice > clients[who_id].defesa) { //se o dice for maior que a defesa do oponente
                printf("Client %d attacked client %d.\n", id, who_id); 
                clients[who_id].vida = clients[who_id].vida + (clients[who_id].defesa - dice); //Uma parte da defesa aguentou o "tranco"

                clients[id].vida += (rand() % n); //regen
                clients[who_id].vida += (rand() % n); //regen

                if(clients[who_id].vida <= 0) {
                    clients[id].ataque += (rand() % 50); 
                    clients[id].defesa += (rand() % 50);
                }
             }
             else printf("Client %d missed [pvp].\n", id);
        }
        else if(clients[id].fight == 2) { //pve
            if(dice > enemies[who_id].defesa) { 
                printf("Client %d attacked monster %d.\n", id, who_id);
                enemies[who_id].vida = enemies[who_id].vida + (enemies[who_id].defesa - dice);

                clients[id].vida += (rand() % n); //regen
                enemies[who_id].vida += (rand() % n); //regen

                if(enemies[who_id].vida <= 0) {
                    clients[id].ataque += (rand() % 50); 
                    clients[id].defesa += (rand() % 50);
                }
                else {
                    dice = enemies[who_id].ataque / 2;
                    dice += rand() % clients[id].ataque;

                    if(dice > clients[id].defesa) {
                        printf("Monster %d attacked player %d.\n", who_id, id);
                        clients[id].vida = clients[id].vida + (clients[id].defesa - dice);
                    }
                    else printf("Monster %d missed.\n", who_id);
                }
            }

            else printf("Client %d missed [pve].\n", id);
        }
    }
    else if(clients[id].vida <= 300 && (cmd == 'w' || cmd == 'W')) {
        dice = (rand() % 70) + 10;
        if(rand() % 100 <= 15) {
            clients[id].vida += dice;

            dice = enemies[who_id].ataque / 2;
            dice += rand() % clients[id].ataque;     

            if(clients[id].fight == 2) {
                if(dice > clients[id].defesa) {
                    printf("Monster %d attacked player %d.\n", who_id, id);
                    clients[id].vida = clients[id].vida + (clients[id].defesa - dice);
                }
                else printf("Monster %d missed.\n", who_id);
            }
        }
        printf("Player %d gained %d HP.\n", id, dice);
    }



    sendStatusToClient(id);
    sendEnemyStatusToClient(id, who_id);

    return;
}

void sendStatusToClient(int id) {
    int i;
    upd_msg att_update;

    att_update.tipo = 3;
    att_update.vida = clients[id].vida;
    att_update.x = clients[id].ataque;
    att_update.y = clients[id].defesa;

    sendUpdToClient(clients[id].sockid, att_update);

    return;
}

void sendStatusToAll(void) {
    int id;

    for(id = 0; id < clients_connected; id++) {
        sendStatusToClient(id);
    }

    return;
}

void sendEnemyStatusToClient(int id, int who_id) {
    upd_msg att_update;

    if(clients[id].fight == 1) {
        att_update.tipo = 1;
        att_update.x = clients[who_id].ataque;
        att_update.y = clients[who_id].defesa;
        att_update.vida = clients[who_id].vida;        
    }
    else if(clients[id].fight == 2) {
        att_update.tipo = 2;
        att_update.x = enemies[who_id].ataque;
        att_update.y = enemies[who_id].defesa;
        att_update.vida = enemies[who_id].vida;        
    }

    sendUpdToClient(clients[id].sockid, att_update);

    return;
}
