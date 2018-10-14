#include "lib/client.h"

/* Color Definitions */
#define DEFAULT          "0"

#define BLACK            "0;30"
#define RED              "0;31"
#define GREEN            "0;32"
#define YELLOW           "0;33"
#define BLUE             "0;34"
#define PURPLE           "0;35"
#define CYAN             "0;36"
#define WHITE            "0;37"
#define DARK_GRAY        "1;30"
#define LIGHT_RED        "1;31"
#define LIGHT_GREEN      "1;32"
#define LIGHT_YELLOW     "1;33"
#define LIGHT_BLUE       "1;34"
#define LIGHT_PURPLE     "1;35"
#define LIGHT_CYAN       "1;36"
#define LIGHT_WHITE      "1;37"

#define BLACK_BG            "0;30"
#define RED_BG              "0;31"
#define GREEN_BG            "0;32"
#define YELLOW_BG           "0;33"
#define BLUE_BG             "0;34"
#define PURPLE_BG           "0;35"
#define CYAN_BG             "0;36"
#define WHITE_BG            "0;37"
#define DARK_GRAY_BG        "1;30"
#define LIGHT_RED_BG        "1;31"
#define LIGHT_GREEN_BG      "1;32"
#define LIGHT_YELLOW_BG     "1;33"
#define LIGHT_BLUE_BG       "1;34"
#define LIGHT_PURPLE_BG     "1;35"
#define LIGHT_CYAN_BG       "1;36"
#define LIGHT_WHITE_BG      "1;37"

typedef struct {
    int x, y;
    // int informações do game, hp, ataque, defesa, pontos...;
    int vida, ataque, defesa, turn;
    int classe;
    int sockid, fight, whofight;
    char nome[NAME_SIZE];
} player;

enum str_menu {
    START_GAME = 0,
    OPTIONS = 1,
    TUTORIAL = 2,
    QUIT = 3,
};

enum opt_menu {
    CHANGE_NAME = 0,
    CHANGE_MAP = 1,
    CHANGE_IP = 2,
    CREATE_MAP = 3,
    CHANGE_STATUS = 4,
    HOME = 5,
};

void playerChoice(void);
void printMap(char **map);
int menu(int* lobby, int* error, int* starting);
int mainMenu(char cmd, int* sel);
int optionMenu(char cmd, int* sel, int* b);
void changeName(void);
void changeMap(void);
void changeIp(void);
void randMain(void);
void filePrint(char filedir[], int is_Map);
void lore(void);
void filePrintDelay(char filedir[]);
void tutorial(void);
void showMap (int Mapa);
void readMap(int map_choosed);
void randMap(int row, int column, int Nmonster);
player criaMonstro(void);
int infoFileCheck(char filedir[], int mode);
int winMenu();
int loseMenu();
FILE* fileOpen(char filedir[], char mode[]);
void textcolor (char * color);

clientInfo info;
char** mapa;
int Row;
int Column;
int enemies_total;

int main(void) {
    char msg[BUFFER_SIZE];
    int falha = 0;
    int espera = 1;
    int jogando = 1;
    int conectado = 0;
    upd_msg updt;
    mov_msg mov;
    int i;
    int j;
    int main_opt = 1;
    int map_choosed;
    int buffer_input;
    int in_fight = 0;
    int vida, atk, def;


    system("clear");
    lore();

    //Esta informação salva os dados do jogador.
    //Caso exista informações salvas, ele as utiliza normalmente
    //Caso contrário, ele cria do zero.


    if(infoFileCheck("./data/clientInfo.bin", 0)) {
        changeName();
        changeMap();
        playerChoice();
        infoFileCheck("./data/clientInfo.bin", 1);
    }
    else {
        printf("Deseja usar predefinições?\n");
        printf("0 - Sim\n1 - Nao\n");
        scanf("%d", &buffer_input);
        if(buffer_input == 1) {
            changeName();
            changeMap();
            playerChoice();
            infoFileCheck("./data/clientInfo.bin", 1);
        }
    }

    do {

        if(!menu(&espera, &falha, &jogando))
	        connectToServer(NULL);

        sendInfoToServer(info);


        while(espera && !falha) { // depois só mensagem de texto até o jogo começar
            mov.msg = getch();
            system("clear");
            printf("Aguardando o cliente 0 dar start.\n");

            if(mov.msg > -1) {
                sendMovToServer(mov);
            }

            if(readTxtFromServer(msg) > 0) {
                if(msg[0] == '+') {
                    espera = 0;
                    jogando = 1;
                    readMap( (int) msg[1] - '0');
                    printMap(mapa);
                }
                else if(msg[0] == '-') {
                    espera = 0;
                    falha = 1;
                }
            }
        }
        if(falha){
            exit(1);
        }

        //receber informações iniciais do jogo(mapa, status inicial, etc...)
        //depois desse ponto, todas as mensagens recebidas serão de update, e as enviadas são de movimento.
        while(jogando){

            while(readUpdFromServer(&updt) > 0){// recebe todas mensagens

                switch(updt.tipo){

                    case 0:
                        system("clear");
                        mapa[updt.a][updt.b] = ' ';
                        mapa[updt.x][updt.y] = updt.new;
                        if(in_fight == 0) printMap(mapa);

                        break;

                    case 1:

                    	in_fight = 1;

                        system("clear");

                        filePrint("./data/pvp.txt", 0);
                        textcolor(LIGHT_YELLOW);

                        printf("\n           ENEMY\n");
                        printf("           HP: %d\n", updt.vida);
                        printf("           ATK: %d\n", updt.x);
                        printf("           DEF: %d\n", updt.y);
                        printf("\n           VERSUS\n");

                    	printf("\n %s Player\n", info.nome);
                        printf("           HP: %d\n", vida);
                        printf("           ATK: %d\n", atk);
                        printf("           DEF: %d\n", def);
                        textcolor(DEFAULT);

                        break;

                    case 2:

                    	in_fight = 1;

                        system("clear");

                        filePrint("./data/pve.txt", 0);

                        textcolor(LIGHT_YELLOW);
                        printf("\n           ENEMY\n");
                        printf("           HP: %d\n", updt.vida);
                        printf("           ATK: %d\n", updt.x);
                        printf("           DEF: %d\n", updt.y);
                        printf("\n           VERSUS\n");

 						printf("\n %s Player\n", info.nome);
 					    printf("           HP: %d\n", vida);
 					    printf("           ATK: %d\n", atk);
 					    printf("           DEF: %d\n", def);

 					    textcolor(DEFAULT);

                        break;
                    case 3:

                        vida = updt.vida;
                        atk = updt.x;
                        def = updt.y;

                        break;
                    case 4:
    					system("clear");
    					in_fight = 0;
                       	//mapa[updt.x][updt.y] = updt.new;
                        //printMap(mapa);

                        break;

                    case 5:

    					in_fight = 0;

                        jogando = 0;
                        main_opt = 3;
                        main_opt = loseMenu();

                        break;
                    case 6:
                        break;
                    case 7:

    					in_fight = 0;

                        jogando = 0;
                        main_opt = 4;
                        main_opt = winMenu();

                        break;
                }

                if(in_fight == 0) {
                    textcolor(LIGHT_YELLOW);
	            	printf("\n %s Player\n", info.nome);
	                printf("           HP: %d\n", vida);
	                printf("           ATK: %d\n", atk);
	                printf("           DEF: %d\n", def);
                    textcolor(DEFAULT);
                }

            }

            mov.msg = getch();

            if(mov.msg != -1) {
                sendMovToServer(mov);

            } // retorna -1 se demorou muito e nada foi digitado.
        }

    } while(main_opt != -1);

    textcolor(LIGHT_YELLOW);
    system("clear");
    printf("Ate logo.\n");
    textcolor(DEFAULT);

    return 0;
}

int menu(int* lobby, int* error, int* starting) {
    char command;
    int buffer_input;
    char status[BUFFER_SIZE];
    int menu_status = 1;
    int pos = 0;
    int boolean = 0;

    textcolor(LIGHT_YELLOW);
    filePrint("./data/tela_inicial_0.txt", 0);
    textcolor(DEFAULT);

    while(menu_status != 0 && menu_status != -1) {

        buffer_input = getch();
        if(buffer_input != -1) command = (char) buffer_input;
        else command = '\0';

        if(command != -1) switch(menu_status) {
            case 1:
                menu_status = mainMenu(command, &pos);
                break;
            case 2:
                menu_status = optionMenu(command, &pos, &boolean);
                break;
            default:
                break;
        }

        //Valor utilizado caso o jogador de quit
        if(menu_status == -1) {
            *lobby = 0; //O jogador sai do menu, mas sem um valor de falha ou de começar o jogo
            *starting = 0;
            textcolor(LIGHT_YELLOW);
            printf("o/\n");
            textcolor(DEFAULT);
            exit(0);
        }
    }


    return boolean;
}

int mainMenu(char cmd, int* sel) {
    int mainMenu_status = 1;

    if(*sel == START_GAME) {
        textcolor(LIGHT_YELLOW);
        filePrint("./data/tela_inicial_0.txt", 0);
        textcolor(DEFAULT);
    }
    else if(*sel == OPTIONS) {
        textcolor(LIGHT_YELLOW);
        filePrint("./data/tela_inicial_1.txt", 0);
        textcolor(DEFAULT);
    }
    else if(*sel == TUTORIAL) {
        textcolor(LIGHT_YELLOW);
        filePrint("./data/tela_inicial_2.txt", 0);
        textcolor(DEFAULT);
    }
    else if(*sel == QUIT) {
        textcolor(LIGHT_YELLOW);
        filePrint("./data/tela_inicial_3.txt", 0);
        textcolor(DEFAULT);
    }

    if(cmd == 'w') {
        if(*sel == START_GAME) {
            *sel = QUIT;
        }
        else if(*sel == OPTIONS) {
            *sel = START_GAME;
        }
        else if(*sel == TUTORIAL) {
            *sel = OPTIONS;
        }
        else if(*sel == QUIT) {
            *sel = TUTORIAL;
        }
    }
    else if(cmd == 's') {
        if(*sel == START_GAME) {
            *sel = OPTIONS;
        }
        else if(*sel == OPTIONS) {
            *sel = TUTORIAL;
        }
        else if(*sel == TUTORIAL) {
            *sel = QUIT;
        }
        else if(*sel == QUIT) {
            *sel = START_GAME;
        }
    }
    else if(cmd == 'd') {
        if(*sel == START_GAME) {
            mainMenu_status = 0;
        }
        else if(*sel == OPTIONS) {
            mainMenu_status = 2;
            *sel = CHANGE_NAME;
        }
        else if(*sel == TUTORIAL) {
            tutorial();
        }
        else if(*sel == QUIT) {
            mainMenu_status = -1;
        }
    }

    return mainMenu_status;
}

int optionMenu(char cmd, int* sel, int* b) {
    int optionMenu_status = 2;

    if(*sel == CHANGE_NAME) {
        textcolor(LIGHT_YELLOW);
        filePrint("./data/options_0.txt", 0);
        textcolor(DEFAULT);
    }
    else if(*sel == CHANGE_MAP) {
        textcolor(LIGHT_YELLOW);
        filePrint("./data/options_1.txt", 0);
        textcolor(DEFAULT);
    }
    else if(*sel == CHANGE_IP) {
        textcolor(LIGHT_YELLOW);
        filePrint("./data/options_2.txt", 0);
        textcolor(DEFAULT);
    }
    else if(*sel == CREATE_MAP) {
        textcolor(LIGHT_YELLOW);
        filePrint("./data/options_3.txt", 0);
        textcolor(DEFAULT);
    }
    else if(*sel == CHANGE_STATUS) {
        textcolor(LIGHT_YELLOW);
        filePrint("./data/options_4.txt", 0);
        textcolor(DEFAULT);
    }
    else if(*sel == HOME) {
        textcolor(LIGHT_YELLOW);
        filePrint("./data/options_5.txt", 0);
        textcolor(DEFAULT);
    }

    if(cmd == 'w') {
        if(*sel == CHANGE_NAME) {
            *sel = HOME;
        }
        else if(*sel == CHANGE_MAP) {
            *sel = CHANGE_NAME;
        }
        else if(*sel == CHANGE_IP) {
            *sel = CHANGE_MAP;
        }
        else if(*sel == CREATE_MAP) {
            *sel = CHANGE_IP;
        }
        else if(*sel == CHANGE_STATUS) {
            *sel = CREATE_MAP;
        }
        else if(*sel == HOME) {
            *sel = CHANGE_STATUS;
        }
    }
    else if(cmd == 's') {
        if(*sel == CHANGE_NAME) {
            *sel = CHANGE_MAP;
        }
        else if(*sel == CHANGE_MAP) {
            *sel = CHANGE_IP;
        }
        else if(*sel == CHANGE_IP) {
            *sel = CREATE_MAP;
        }
        else if(*sel == CREATE_MAP) {
            *sel = CHANGE_STATUS;
        }
        else if(*sel == CHANGE_STATUS) {
            *sel = HOME;
        }
        else if(*sel == HOME) {
            *sel = CHANGE_NAME;
        }
    }
    else if(cmd == 'd') {
        if(*sel == CHANGE_NAME) {
            changeName();
        }
        else if(*sel == CHANGE_MAP) {
            changeMap();
        }
        else if(*sel == CHANGE_IP) {
            changeIp();
            *b = 1;
        }
        else if(*sel == CREATE_MAP) {
            randMain();
        }
        else if(*sel == CHANGE_STATUS) {
            playerChoice();
        }
        else if(*sel == HOME) {
            *sel = START_GAME;
            optionMenu_status = 1;
        }
    }

    return optionMenu_status;
}

int winMenu() {
    char cmd = -1;
    system("clear");
    textcolor(LIGHT_YELLOW);
    filePrint("./data/win_frame.txt", 0);
    textcolor(DEFAULT);
    while(cmd == -1)
        cmd = getch();
    if(cmd == 'q'){
        return -1;
    }else if(cmd == 'h'){
        return 1;
    }
}

int loseMenu() {
    char cmd = -1;
    system("clear");
    textcolor(LIGHT_YELLOW);
    filePrint("./data/lose_frame.txt", 0);
    textcolor(DEFAULT);
    while(cmd == -1)
        cmd = getch();
    if(cmd == 'q'){
        return -1;
    }else if(cmd == 'h'){
        return 1;
    }
}

void changeName(void) {
    system("clear");
    textcolor(LIGHT_YELLOW);
    printf("Digite o nome: ");
    textcolor(DEFAULT);
    scanf(" %[^\n]", info.nome);
    return;
}

void changeMap(void) {
    system("clear");
    int Mapa;
    int cond = 0;
    FILE* pF;



    do {

        textcolor(LIGHT_YELLOW);
        printf("Digite um numero de 1 a 9 pra escolher o mapa: \n");
        printf("9 - Randomico.\n");
        textcolor(DEFAULT);

        scanf("%d", &Mapa);

        if(Mapa == 9) {
            pF = fopen("./data/mapa9.txt", "rt");
            //Verifico se existe um mapa aleatorio criado.
            if(pF == NULL) {
                randMain();
            }
            else fclose(pF);

        }

        if(Mapa < 1 || Mapa > 9) {
            textcolor(LIGHT_YELLOW);
            printf("Digite um mapa valido.\n");
            textcolor(DEFAULT);
        }
        else {
            showMap(Mapa);
            textcolor(LIGHT_YELLOW);
            printf("Voce escolheu o mapa %d, digite 1 para confirmar\n", Mapa);
            textcolor(DEFAULT);
            scanf("%d", &cond);
        }

    } while(cond != 1);

    info.mapa = Mapa;

    return;
}

void changeIp(void) {
    system("clear");
    char stringIP[BUFFER_SIZE];
    textcolor(LIGHT_YELLOW);
    printf("Digite o novo IP: ");
    textcolor(DEFAULT);
    scanf(" %[^\n]", stringIP);
    connectToServer(stringIP);
    return;
}

void randMain(void) {
    system("clear");
    int i;
    player *enemies;
    FILE* pF;

    int NMonstros, NLinhas, NColunas;
    textcolor(LIGHT_YELLOW);
    printf("Digite o numero de linhas: ");
    scanf("%d", &NLinhas);
    printf("Digite o numero de colunas: ");
    scanf("%d", &NColunas);
    printf("Digite o numero de monstros: ");
    scanf("%d", &NMonstros);
    textcolor(DEFAULT);
    enemies = (player *) malloc(NMonstros * sizeof(player));

    if(enemies == NULL)
        exit(1);

    enemies_total = NMonstros;

    for(i = 0; i < NMonstros; i++){
      enemies[i] = criaMonstro();
    }

    pF = fopen("./data/monstros9.bin", "wb");
    if(pF == NULL) {
        printf("Error in opening of ./data/monstros9.bin\n");
        exit(1);
    }

    fwrite(enemies, NMonstros, sizeof(monsters), pF);

    free(enemies);
    fclose(pF);

    randMap(NLinhas, NColunas, NMonstros);

    return;
}

void playerChoice(void) {
    int cond;
    int aux;

    system("clear");

    filePrint("./data/classe.txt", 0);

    do {

        textcolor(LIGHT_YELLOW);
        printf("Digite 1 para Thief.\nDigite 2 para Paladin.\nDigite 3 para Barbarian.\n");
        textcolor(DEFAULT);
        scanf(" %d", &aux);
        info.classe = aux - 1;

        switch(info.classe) {
            case 0:
                filePrint("./data/thief.txt", 0);
                textcolor(LIGHT_YELLOW);
                printf("Voce vai jogar de Thief.\nDigite 1 para confirmar.\n");
                textcolor(DEFAULT);
                scanf("%d", &cond);
                system("clear");
                break;
            case 1:
                filePrint("./data/paladin.txt", 0);
                textcolor(LIGHT_YELLOW);
                printf("Voce vai jogar de Paladin.\nDigite 1 para confirmar.\n");
                textcolor(DEFAULT);
                scanf("%d", &cond);
                system("clear");
                break;
            case 2:
                filePrint("./data/barbaro.txt", 0);
                textcolor(LIGHT_YELLOW);
                printf("Voce vai jogar de Barbarian.\nDigite 1 para confirmar.\n");
                textcolor(DEFAULT);
                scanf("%d", &cond);
                system("clear");
                break;
            default:
                system("clear");
                textcolor(LIGHT_YELLOW);
                printf("Infelizmente, nao ha uma classe para %d.\n", info.classe);
                printf("Aguarde a continuacao. =)\n");
                textcolor(DEFAULT);
                cond = 0;
                break;
        }

    } while(cond != 1);

    system("clear");

    do {
        filePrint("./data/atributos.txt", 0);
        textcolor(LIGHT_YELLOW);
        printf("\nVoce possui 5 atributos para distribuir entre Strength, Dexterity, Constitution.\n");

        printf("Strengh: ");
        scanf("%d", &info.STR);
        printf("Dextrerity: ");
        scanf("%d", &info.DEX);
        printf("Constitution: ");
        scanf("%d", &info.CON);
        textcolor(DEFAULT);

        if(info.DEX + info.CON + info.STR > 5) {
            textcolor(LIGHT_YELLOW);
            printf("\nA escolha de atributos foi invalida.\n");
            printf("Sem cheats, noob!\n");
            textcolor(DEFAULT);
            cond = 0;
        }

        else if(info.DEX > 5 || info.CON > 5 || info.STR > 5) {
            textcolor(LIGHT_YELLOW);
            printf("\nA escolha de atributos foi invalida.\n");
            printf("Sem cheats, noob!\n");
            textcolor(DEFAULT);
            cond = 0;
        }

        else if(info.DEX + info.CON + info.STR < 5) {
            textcolor(LIGHT_YELLOW);
            printf("\nVoce nao esta utilizando todo potencial de um Cavaleiro de Ahceme.\n");
            printf("Digite 1 para confirmar: ");
            textcolor(DEFAULT);
            scanf("%d", &cond);
        }

        else {
            system("clear");
            textcolor(LIGHT_YELLOW);
            printf("\nVoce escolheu\n");
            printf("Constitution: %d\n", info.CON);
            printf("Dextrerity: %d\n", info.DEX);
            printf("Strengh: %d\n", info.STR);

            printf("Digite 1 para confirmar: ");
            textcolor(DEFAULT);
            scanf("%d", &cond);
        }


    } while(cond != 1);

    system("clear");

    return;
}

void lore(void) {
    char in = -1;
    char c;
    int i=0;
    FILE *arq;
    system("clear");

    arq = fopen("./data/lore.txt","rt");
textcolor(LIGHT_YELLOW);
    if(arq == NULL){
        printf("Error\n");
        exit(1);
    }

    while(fscanf(arq,"%c", &c) != EOF){

        printf("%c", c);
        fflush(stdout);
        usleep(4000);
    }
textcolor(DEFAULT);

    textcolor(LIGHT_YELLOW);
    printf("\n\nDigite qualquer tecla para continuar.\n");
    textcolor(DEFAULT);

    while(in == -1) in = getch();

    system("clear");

    return;
}

void filePrint(char filedir[], int is_Map) {
    int i = 0;
    char s;
    FILE* file;

    file = fopen(filedir, "rt");
    if(file == NULL) {
        printf("Error in  opening of: %s\n", filedir);
        exit(1);
    }

    system("clear");

    while( fscanf(file, "%c", &s) != EOF )
    {
        if(s == '*' && is_Map)
        {
            textcolor(LIGHT_GREEN);
            printf("%c", s);
            textcolor(DEFAULT);
        }
        else printf("%c", s);
    }

    fclose(file);

    return;
}

void tutorial(void){
    char comando = -1;

    system("clear");

    textcolor(LIGHT_YELLOW);
    printf("TUTORIAL : KNIGHTS OF AHCEME\n\n");
    printf("**********OBJETIVO**********\n");
    printf("-> Elimine todos os players monstros para assim poder lutar contra Jahva e alcancar a gloria eterna.\n");
    printf("-> Voce podera comer arvores para recuperar HP.\n");
    printf("Mas, cuidado, comer arvores em excesso te cansara e perder uma porcentagem de seu HP.\n\n");
    printf("********MOVIMENTACAO**********\n");
    printf("w - Andar para cima.\n");
    printf("a - Andar para esquerda.\n");
    printf("s - Andar para baixo.\n");
    printf("d - Andar para direita.\n\n");
    printf("**********BATALHA**********\n");
    printf("Para entrar em batalha basta colidir com outro player ou monstro.\n");
    printf("a - Atacar em batalha - Voce podera errar o ataque, causar dano critico ou roubar vida.\n");
    printf("w - Recuperar vida em batalha - Voce recuperara uma pequena porcentagem de sua vida, mas estara sujeito a ataques do adversario.\n");
    printf("r - Fugir da batalha - Voce ira fugir da batalha ao preco de perder uma porcentagem do seu HP.\n\n");
    printf("                                     HAVE FUN!!!\n");
    printf("\n\n\nPARA VOLTAR A TELA INICIAL DIGITE QUALQUER TECLA.\n");
    textcolor(DEFAULT);

    while(comando == -1) {
        comando = getch();
    }

  return;
}

void showMap (int Mapa){
    if(Mapa == 1){
        filePrint("./data/mapa1.txt", 1);
    }
    else if(Mapa == 2){
        filePrint("./data/mapa2.txt", 1);
    }
    else if (Mapa == 3){
        filePrint("./data/mapa3.txt", 1);
    }
    else if (Mapa == 4){
        filePrint("./data/mapa4.txt", 1);
    }
    else if(Mapa == 5){
        filePrint("./data/mapa5.txt", 1);
    }
    else if(Mapa == 6) {
        filePrint("./data/mapa6.txt", 1);
    }
    else if(Mapa == 7) {
        filePrint("./data/mapa7.txt", 1);
    }
    else if(Mapa == 8){
        filePrint("./data/mapa8.txt", 1);
    }
    else if(Mapa == 9){
        filePrint("./data/mapa9.txt", 1);
    }
    return;
}

void readMap(int map_choosed) {
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
            exit(1);
    }

    fscanf(file_map, "%d %d %d\n", &Row, &Column, &enemies_total);
    Column++;

    mapa = (char**) malloc(Row * sizeof(char*));
    if(mapa == NULL) exit(1);

    for(i = 0; i < Row; i++) {
        mapa[i] = (char*) malloc(Column * sizeof(char));
        if(mapa[i] == NULL) exit(1);
    }

    for(i = 0; i < Row; i++)
        for(j = 0; j < Column; j++)
            mapa[i][j] = fgetc(file_map);

    fclose(file_map);

    return;
}

player criaMonstro(void){
    int x;
    int y;
    int i;
    int k;
    player new_monster;

    new_monster.x = 0;
    new_monster.y = 0;
    new_monster.vida = 0;
    new_monster.ataque = 0;
    new_monster.defesa = 0;
    new_monster.turn = 0;
    new_monster.fight = 0;
    new_monster.whofight = -1;
    new_monster.sockid = 0;
    new_monster.nome[0] = 'm';
    new_monster.nome[1] = '\0';
    new_monster.classe = 1;


    srand((unsigned)time(NULL));

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

void randMap(int row, int column, int Nmonster) {
    char** matrix;
    time_t t;
    FILE *pArq;
    int i;
    int j;
    int prob;

    Row = row;
    column = column;

    //utilizo seed como tempo.
    srand((unsigned) time(&t));

    matrix = (char** ) malloc(row * sizeof(char*));
    if(matrix == NULL) exit(1);

    for(i = 0; i < row; i++) {
        matrix[i] = (char* ) malloc(column * sizeof(char));

        if(matrix[i] == NULL) exit(1);
    }

    for(i = 0; i < row; i++) {
        for(j = 0; j < column; j++) {

            //Faço o preenchemento das bordas.
            if(i == 0 || i == (row - 1))
                matrix[i][j] = '*';

            else if(j == 0 || j == (column - 1))
                matrix[i][j] = '*';

            //Faço o preenchemento do interno do mapa.
            else
                matrix[i][j] = ' ';
        }
    }

    for(i = 1; i < row-1; i++) {

        for(j = 1; j < column-1; j++) {

            prob = (rand() % 100) + 1;

            //Caso exista uma árvore próxima
            //É provável que exista outras aos arredores
            //Com isso, somamos 5 ao valor sorteado.
            if(matrix[i - 1][j])
                prob += 5;
            if(matrix[i + 1][j])
                prob += 5;
            if(matrix[i][j - 1])
                prob += 5;
            if(matrix[i][j + 1])
                prob += 5;

            //Por fim, utilizamos uma porcentagem de 10% para a existência de árvores.
            if(prob >= 90)
                matrix[i][j] = '*';
        }
    }

    pArq = fopen("./data/mapa9.txt", "wt");
    if(pArq == NULL) exit(1);

    //Printamos as informações iniciais.
    fprintf(pArq, "%d %d %d\n", row, column, Nmonster);

    //E printamos o mapa.
    for(i = 0; i < row; i++) {
        for(j = 0; j < column; j++)
            fprintf(pArq, "%c", matrix[i][j]);
        fprintf(pArq, "%c", '\n');
    }

    fclose(pArq);

    return;
}

int infoFileCheck(char filedir[], int mode) {
	FILE* pf;

	//Aqui ele escreve o clientInfo no clientInfo.bin
	if(mode) {
		pf = fopen(filedir, "wb");
		if(pf == NULL) {
			printf("Error in opening of %s\n", filedir);
			exit(1);
		}

		fwrite(&info, sizeof(clientInfo), 1, pf);
	}
	//Aqui ele vê se existe um clientInfo.bin
	else {
		pf = fopen(filedir, "rb");
		if(pf == NULL) { //Checa se existe um clientInfo.bin
            return 1;  //Caso contrário ele retorna 1 e o client necessitará ser escrito
        }
	    else {
            fread(&info, sizeof(clientInfo), 1, pf);
        }
    }

    fclose(pf);

	return 0;
}

void printMap (char **map)
{
    int i, j;
    system("clear");
    textcolor(LIGHT_GREEN);
    printf("*");
    textcolor(DEFAULT);
    for(i=0;i<Row;i++){
        for(j=0;j<Column;j++){
            if(i != 0 || j != 0)
            {
                if(map[i][j] == '*')
                {
                    textcolor(LIGHT_GREEN);
                    printf("%c", map[i][j]);
                    textcolor(DEFAULT);
                }
                else if(map[i][j] == 'm')
                {
                    textcolor(LIGHT_RED);
                    printf("%c", map[i][j]);
                    textcolor(DEFAULT);
                }
                else if(map[i][j] >= '0' && map[i][j] <= '9')
                {
                    textcolor(LIGHT_BLUE);
                    printf("%c", map[i][j]);
                    textcolor(DEFAULT);
                }
                else
                {
                    printf("%c", map[i][j]);
                }
            }
        }
    }
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

void textcolor (char * color) {
    printf("\E[%sm", color);
}
